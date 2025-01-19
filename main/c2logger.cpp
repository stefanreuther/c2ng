/**
  *  \file main/c2logger.cpp
  *
  *  This is a very straight port/copy of the original, PCC2-based PlanetsCentral version.
  *  It does not use any of our abstractions (mostly caused by the lack there-of).
  *
  *  In particular, this means it uses no character-set translation whatsoever.
  *
  *  It runs on POSIX only.
  */

#if TARGET_OS_POSIX
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/socket.h>         // everything sockets
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>              // getaddrinfo
#include <pwd.h>
#include <signal.h>
#include <unistd.h>
#include "afl/string/string.hpp"

static String_t arg_logfile;              // "-log="
static String_t arg_pidfile;              // "-pid="
static String_t arg_cd;                   // "-cd="
static bool opt_kill = false;             // "-kill", "-restart"
static bool opt_start = true;             // "-restart"
static bool opt_fg = false;               // "-fg"
static long arg_loglimit = 10*1024*1024;  // "-limit="
static uid_t arg_uid = 0;                 // "-uid="
static String_t arg_listen_host;          // "-listen="
static String_t arg_listen_port;          // "-listen="

/** Rotate logfile. Renames the existing logfile, and creates a new one,
    giving it the same file descriptor.
    \param fd Logfile fd
    \param timestamp Timestamp */
static void
rotateLog(int fd, const char* timestamp)
{
    String_t newName = arg_logfile + "-" + timestamp;
    int i = 0;
    while (access(newName.c_str(), 0) == 0) {
        char tmp[20];
        ++i;
        sprintf(tmp, "%d", i);
        newName = arg_logfile + "-" + timestamp + "-" + tmp;
    }
    close(fd);
    rename(arg_logfile.c_str(), newName.c_str());

    int newfd = open(arg_logfile.c_str(), O_WRONLY | O_CREAT, 0666);
    if (newfd >= 0) {
        // ok, replace old log file with new one
        if (fd != newfd) {
            dup2(newfd, fd);
            close(newfd);
        }
    } else {
        // Problem creating log file -- disable rotation
        // Now what?
        dup2(STDOUT_FILENO, fd);
    }
}

/** Write to logfile.
    \param fd Logfile fd
    \param atbol Status tracking: are we at the beginning of a line?
    \param text Text to write
    \param length Number of bytes */
static void
writeLog(int fd, bool& atbol, const char* text, size_t length)
{
    char timestamp[50];
    while (length > 0) {
        if (atbol) {
            time_t now = time(0);

            // try log rotation
            if (arg_loglimit > 0 && lseek(fd, 0, SEEK_CUR) >= arg_loglimit) {
                strftime(timestamp, sizeof(timestamp), "%Y%m%d", gmtime(&now));
                rotateLog(fd, timestamp);
            }

            // write timestamp
            strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", gmtime(&now));
            write(fd, timestamp, std::strlen(timestamp));
            atbol = false;
        }

        const char* p = static_cast<const char*>(std::memchr(text, '\n', length));
        if (p != 0) {
            size_t len = p+1 - text;
            write(fd, text, len);
            text += len;
            length -= len;
            atbol = true;
        } else {
            write(fd, text, length);
            atbol = false;
            break;
        }
    }
}

static int
bindSocket(const char* host, const char* port)
{
    struct addrinfo hints;
    struct addrinfo* result;
    std::memset(&hints, 0, sizeof(hints));

    hints.ai_socktype = SOCK_STREAM;
    int ai = getaddrinfo(host, port, &hints, &result);
    if (ai != 0) {
        std::fprintf(stderr, "Unable to bind to %s:%s: %s\n", host, port, gai_strerror(ai));
    }

    // Make a socket
    int sock = -1;
    int errorCode = 0;
    for (struct addrinfo* p = result; p != 0; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock >= 0) {
            // Check whether this socket is good for us.
            // errorCode = process(sock, p->ai_addr, p->ai_addrlen);
            int one = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
            if (bind(sock, p->ai_addr, p->ai_addrlen) == 0) {
                break;
            }

            // Nope.
            errorCode = errno;
            close(sock);
            sock = -1;
        }
    }

    // Clean up
    if (result != 0) {
        freeaddrinfo(result);
    }

    // Anything found?
    if (sock < 0) {
        if (errorCode == 0) {
            std::fprintf(stderr, "Unable to bind to %s:%s: unknown error\n", host, port);
        } else {
            std::fprintf(stderr, "Unable to bind to %s:%s: %s\n", host, port, std::strerror(errorCode));
        }
    }
    return sock;
}

/** Run child process under c2logger control.
    \param argv parameters for new process (including process binary, argv[0])
    \retval true process has terminated; restart it
    \retval false process has terminated; don't restart it */
static bool
run(char** argv)
{
    // Open logfile
    int log = open(arg_logfile.c_str(), O_WRONLY | O_CREAT, 0666);
    if (log < 0) {
        std::perror(arg_logfile.c_str());
        return false;
    }
    lseek(log, 0, SEEK_END);

    // Create pipe for child's stdout
    enum { Read, Write };
    int fds[2];
    if (pipe(fds) != 0) {
        std::perror("pipe");
        close(log);
        return false;
    }

    // Create child
    pid_t pid = fork();
    if (pid < 0) {
        std::perror("fork");
        close(fds[0]);
        close(fds[1]);
        close(log);
        return false;
    }
    if (pid == 0) {
        // I am the child
        int null = open("/dev/null", O_RDONLY);
        dup2(null, STDIN_FILENO);
        dup2(fds[Write], STDOUT_FILENO);
        dup2(fds[Write], STDERR_FILENO);
        close(fds[Read]);
        close(fds[Write]);
        close(null);
        close(log);
        if (arg_cd.size()) {
            if (chdir(arg_cd.c_str()) < 0) {
                std::perror(arg_cd.c_str());
                exit(127);
            }
        }
        if (arg_listen_host.size()) {
            // FIXME: we are manually implementing the socket stuff here.
            // As of 20190120, afl does not have a way to perform a bind and give us the socket number,
            // so we cannot use it. In addition, using afl would enlarge the binary by ~50k.
            int sock = bindSocket(arg_listen_host.c_str(), arg_listen_port.c_str());
            if (sock < 0) {
                std::fprintf(stderr, "Unable to provide socket; exiting.");
                exit(127);
            }
            char tmp[30];
            sprintf(tmp, "%d", sock);
            setenv("C2SOCKET", tmp, 1);
        }
        if (arg_uid != 0) {
            if (setuid(arg_uid) < 0) {
                std::perror("setuid");
                exit(127);
            }
        }
        execv(argv[0], argv);
        std::perror(argv[0]);
        exit(127);
    }

    // I am the parent
    close(fds[Write]);
    bool atbol = false;
    char buffer[4096];
    std::sprintf(buffer,
                 "\n"
                 "-------------------------\n"
                 "Process '%s' started with pid %ld\n"
                 "-------------------------\n",
                 argv[0], (long) pid);
    writeLog(log, atbol, buffer, strlen(buffer));

    // Create pidfile
    if (arg_pidfile.size()) {
        int fd = open(arg_pidfile.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if (fd < 0) {
            std::sprintf(buffer, "Unable to create pidfile: %s\n", strerror(errno));
            writeLog(log, atbol, buffer, strlen(buffer));
        } else {
            std::sprintf(buffer, "%ld", (long) pid);
            write(fd, buffer, strlen(buffer));
            close(fd);
        }
    }

    ssize_t n;
    while ((n = read(fds[Read], buffer, sizeof(buffer))) > 0) {
        writeLog(log, atbol, buffer, n);
    }
    int err = errno;

    // Write a blank line
    if (!atbol) {
        writeLog(log, atbol, "\n", 1);
    }

    // Log read error
    if (n < 0) {
        std::sprintf(buffer, "** Read error, %s\n", strerror(err));
        writeLog(log, atbol, buffer, strlen(buffer));
    }

    // Wait for child death
    bool restart;
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        std::sprintf(buffer, "** Process terminated with exit code %d\n", int(WEXITSTATUS(status)));
        restart = false;
    } else if (WIFSIGNALED(status)) {
        restart = (WTERMSIG(status) == SIGUSR1);
        std::sprintf(buffer, "** Process terminated with signal %d%s\n",
                     int(WTERMSIG(status)),
                     restart ? ", will be restarted" : "");
    } else {
        std::sprintf(buffer, "** Process exited with status 0x%08X\n", unsigned(status));
        restart = false;
    }
    writeLog(log, atbol, buffer, strlen(buffer));
    close(log);

    // Remove pidfile
    if (arg_pidfile.size()) {
        unlink(arg_pidfile.c_str());
    }

    return restart;
}

/** Check for stale file. This is used to detect pidfiles created before
    system boot (i.e. previous instance).
    \param t Timestamp of file
    \retval true File is definitely stale
    \retval false File may be current */
static bool
isStaleFile(time_t t)
{
    // Open uptime file
    int fd = open("/proc/uptime", O_RDONLY);
    if (fd < 0) {
        return false;           // Unknown uptime, assume file is ok
    }

    // Read uptime file
    char buffer[100];
    ssize_t n = read(fd, buffer, sizeof(buffer)-1);
    close(fd);
    if (n <= 0) {
        return false;           // Unable to read uptime, assume file is ok
    }

    // Parse
    buffer[n] = 0;
    time_t now = time(0);
    time_t uptime = strtol(buffer, 0, 10);
    if (uptime <= 0 || uptime > now) {
        return false;           // Parse error
    }

    // File is stale if boot time is after our mtime
    return now - uptime > t;
}

/** Check for existing process, kill it if found. Returns when the previous
    process (if found) has been killed, or when it believes there is no
    previous process. */
static void
checkExistingProcess()
{
    // Try to open pidfile
    int fd = open(arg_pidfile.c_str(), O_RDONLY);
    if (fd < 0) {
        return;                 // pidfile does not exist, ok
    }

    // Check pidfile age
    struct stat stpid;
    if (fstat(fd, &stpid) == 0) {
        if (isStaleFile(stpid.st_mtime)) {
            std::printf("%s: stale pidfile ignored.\n", arg_pidfile.c_str());
            close(fd);
            return;
        }
    }

    // pidfile exists. Read pid.
    char buffer[100];
    ssize_t n = read(fd, buffer, sizeof(buffer)-1);
    close(fd);
    if (n <= 0) {
        return;                 // pidfile is empty or not readable, assume ok
    }
    buffer[n] = 0;

    // parse pid
    pid_t pid = static_cast<pid_t>(std::strtol(buffer, 0, 10));
    if (pid <= 0) {
        return;                 // pidfile is invalid, assume ok
    }

    if (kill(pid, SIGTERM) == 0) {
        std::printf("%s: terminating previous instance... ", arg_pidfile.c_str());
        std::fflush(stdout);
        do {
            usleep(100000);
        } while (kill(pid, 0) == 0);
        std::printf("done\n");
    }
}

int main(int /*argc*/, char** argv)
{
    // Parse command line
    const char* progname = argv[0];
    while (const char* p = *++argv) {
        if (p[0] == '-' && p[1] == '-') {
            ++p;
        }
        if (std::strncmp(p, "-log=", 5) == 0) {
            arg_logfile = p+5;
        } else if (std::strncmp(p, "-pid=", 5) == 0) {
            arg_pidfile = p+5;
        } else if (std::strncmp(p, "-cd=", 4) == 0) {
            arg_cd = p+4;
        } else if (std::strncmp(p, "-limit=", 7) == 0) {
            arg_loglimit = std::strtol(p+7, 0, 0);
        } else if (std::strcmp(p, "-restart") == 0) {
            opt_kill = true;
        } else if (std::strcmp(p, "-kill") == 0) {
            opt_kill = true;
            opt_start = false;
        } else if (std::strcmp(p, "-fg") == 0) {
            opt_fg = true;
        } else if (std::strncmp(p, "-uid=", 5) == 0) {
            struct passwd* user = getpwnam(p+5);
            if (user == 0) {
                std::fprintf(stderr, "%s: user '%s' not known\n", progname, p+5);
                return 1;
            }
            arg_uid = user->pw_uid;
        } else if (std::strncmp(p, "-listen=", 8) == 0) {
            const char* host_port = p+8;
            const char* delim = std::strchr(host_port, ':');
            if (!delim || delim == host_port || delim[0] == '\0') {
                std::fprintf(stderr, "%s: '-listen' requires a host name and port number\n", progname);
                return 1;
            }
            arg_listen_host.assign(host_port, delim - host_port);
            arg_listen_port.assign(delim+1);
        } else if (std::strcmp(p, "-help") == 0) {
            std::printf("%s: PCC2 Server Control and Logging Utility (c2ng)\n\n"
                        "c2logger [-opts] COMMAND [ARGS...]\n\n"
                        " -log=LOGFILE     Name of logfile (default: COMMAND.log)\n"
                        " -pid=PIDFILE     Name of pidfile (default: none)\n"
                        " -cd=DIR          Working directory for COMMAND\n"
                        " -uid=USERNAME    Run COMMAND as USERNAME\n"
                        " -listen=H:P      Create listen socket on host/port\n"
                        " -limit=BYTES     Rotate logfile after BYTES (default: 10 meg)\n"
                        " -restart, -kill  Restart/kill program (default: start)\n"
                        " -fg              Remain in foreground (default: background)\n",
                        progname);
            return 0;
        } else if (p[0] == '-') {
            std::fprintf(stderr, "%s: invalid option '%s'\n", progname, p);
            return 1;
        } else {
            break;
        }
    }

    // Anything remaining?
    if (opt_start && *argv == 0) {
        std::fprintf(stderr, "%s: must specify program to execute\n", progname);
        return 1;
    }

    // Check pidfile
    if (arg_pidfile.size() != 0) {
        if (opt_kill) {
            checkExistingProcess();
        }
        unlink(arg_pidfile.c_str());
    } else {
        if (opt_kill) {
            std::fprintf(stderr, "%s: must specify '-pid=PIDFILE' to use '-kill'/'-restart'\n", progname);
            return 1;
        }
    }

    // Background myself
    if (!opt_fg) {
        if (fork() > 0) {
            _exit(0);
        }
    }

    // Operate
    if (opt_start) {
        // Figure out logfile
        if (arg_logfile.size() == 0) {
            const char* p = std::strrchr(*argv, '/');
            if (p) {
                arg_logfile = p+1;
            } else {
                arg_logfile = *argv;
            }
            arg_logfile += ".log";
        }

        while (run(argv)) {
            /* nix */
        }
    }
}

#else
# include <cstdio>
int main()
{
    std::fprintf(stderr, "c2logger not implemented for this platform.\n");
    return 1;
}
#endif
