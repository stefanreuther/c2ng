/**
  *  \file util/process/posixfactory.cpp
  *  \brief Class util::process::PosixFactory
  */

#ifdef TARGET_OS_POSIX
#include "util/process/posixfactory.hpp"
#include "util/process/subprocess.hpp"

#include <fcntl.h>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <sys/wait.h>
#include "afl/string/format.hpp"

namespace {
    String_t formatError(const char* why)
    {
        const char* err = std::strerror(errno);
        String_t result = why;
        result += ": ";
        result += err;
        return why;
    }

    class PosixSubprocess : public util::process::Subprocess {
     public:
        PosixSubprocess()
            : m_pid(0), m_readFD(-1), m_writeFD(-1)
            { }

        bool isActive() const
            { return m_pid != 0; }

        uint32_t getProcessId() const
            { return m_pid; }

        bool start(const String_t& path, afl::base::Memory<const String_t> args)
            {
                // ex RouterSession::start
                // Don't do anything if session is already started
                if (m_pid != 0) {
                    return true;
                }

                // Make pipes
                enum { Read, Write };
                int toChild[2], fromChild[2];
                if (pipe(toChild) != 0) {
                    m_status = formatError("pipe");
                    return false;
                }
                if (pipe(fromChild) != 0) {
                    m_status = formatError("pipe");
                    close(toChild[0]);
                    close(toChild[1]);
                    return false;
                }

                // Make child
                pid_t child = fork();
                if (child < 0) {
                    m_status = formatError("fork");
                    close(toChild[0]);
                    close(toChild[1]);
                    close(fromChild[0]);
                    close(fromChild[1]);
                    return false;
                }
                if (child == 0) {
                    // I am the child
                    dup2(toChild[Read], 0);
                    dup2(fromChild[Write], 1);
                    dup2(fromChild[Write], 2);
                    close(toChild[0]);
                    close(toChild[1]);
                    close(fromChild[0]);
                    close(fromChild[1]);

                    // Build argv
                    std::vector<char*> argv;
                    argv.push_back(const_cast<char*>(path.c_str()));
                    while (const String_t* p = args.eat()) {
                        argv.push_back(const_cast<char*>(p->c_str()));
                    }
                    argv.push_back(0);

                    execv(argv[0], &argv[0]);
                    perror(argv[0]);
                    _exit(1);
                }

                // I am the parent
                m_readFD = fromChild[Read];
                m_writeFD = toChild[Write];
                m_pid = child;
                close(fromChild[Write]);
                close(toChild[Read]);
                fcntl(m_readFD, F_SETFD, FD_CLOEXEC);
                fcntl(m_writeFD, F_SETFD, FD_CLOEXEC);
                return true;
            }

        bool stop()
            {
                // no need to do anything if session is already stopped
                if (m_pid == 0) {
                    return true;
                }

                // terminate it by closing its stdin
                close(m_writeFD);
                m_writeFD = -1;

                // satisfy possibly pending output
                char buffer[1024];
                while (read(m_readFD, buffer, sizeof(buffer)) > 0) {
                    // ok
                }
                close(m_readFD);
                m_readFD = -1;

                // Wait for termination
                int status;
                pid_t result = waitpid(m_pid, &status, 0);
                m_pid = 0;
                if (result < 0) {
                    m_status = afl::string::Format("wait fails: %s", std::strerror(errno));
                    return false;
                } else if (WIFEXITED(status)) {
                    m_status = afl::string::Format("exited with code %d", int(WEXITSTATUS(status)));
                    return true;
                } else if (WIFSIGNALED(status)) {
                    m_status = afl::string::Format("terminated by signal %d", int(WTERMSIG(status)));
                    return false;
                } else {
                    m_status = afl::string::Format("exited with unknown termination code %04X", status);
                    return false;
                }
            }

        bool writeLine(const String_t& line)
            {
                return (write(m_writeFD, line.data(), line.size()) == ssize_t(line.size()));
            }

        bool readLine(String_t& result)
            {
                // FIXME: inefficient!!!
                result.clear();

                char ch;
                while (read(m_readFD, &ch, 1) == 1) {
                    if (ch != '\r') {
                        result += ch;
                    }
                    if (ch == '\n') {
                        return true;
                    }
                }
                return false;
            }

        String_t getStatus() const
            { return m_status; }

     private:
        pid_t m_pid;
        int m_readFD;
        int m_writeFD;
        String_t m_status;
    };
}

util::process::Subprocess*
util::process::PosixFactory::createNewProcess()
{
    return new PosixSubprocess();
}

#else
int g_variableToMakeProcessPosixFactoryObjectFileNotEmpty;
#endif


