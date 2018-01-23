/**
  *  \file util/processrunner.cpp
  *  \brief Class util::ProcessRunner
  */

#include "util/processrunner.hpp"

#ifdef TARGET_OS_POSIX
/*
 *  POSIX Implementation
 */
# include <vector>
# include <errno.h>
# include <stdio.h>
# include <sys/fcntl.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include "afl/data/access.hpp"
# include "afl/data/defaultvaluefactory.hpp"
# include "afl/data/segment.hpp"
# include "afl/data/vector.hpp"
# include "afl/except/systemexception.hpp"
# include "afl/io/resp/parser.hpp"
# include "afl/io/resp/writer.hpp"
# include "afl/sys/mutexguard.hpp"

class util::ProcessRunner::Impl : private afl::io::DataSink {
 public:
    Impl();
    void use(int readFD, int writeFD);
    void start();
    void stop();
    int run(const Command& cmd, String_t& output);

 private:
    bool serveRequest();

    // DataSink:
    virtual bool handleData(afl::base::ConstBytes_t& data);

    // Object reading:
    afl::data::Value* readObject();

    afl::sys::Mutex m_mutex;

    int m_readFD;
    int m_writeFD;
    pid_t m_workerPid;

    uint8_t m_readBuffer[4096];
    afl::base::ConstBytes_t m_readDescriptor;

    afl::io::resp::Writer m_writer;
};

// Constructor.
inline
util::ProcessRunner::Impl::Impl()
    : DataSink(),
      m_readFD(-1),
      m_writeFD(-1),
      m_workerPid(-1),
      m_readDescriptor(),
      m_writer(*this)
{ }

// Main/helper: set file descriptors to use for reading/writing.
inline void
util::ProcessRunner::Impl::use(int readFD, int writeFD)
{
    m_readFD = readFD;
    m_writeFD = writeFD;
}

// Main program: start helper process.
inline void
util::ProcessRunner::Impl::start()
{
    // ex Runner::start

    // Create pipes
    int command_pipe[2];
    int result_pipe[2];
    if (::pipe(command_pipe) < 0 || ::pipe(result_pipe) < 0) {
        throw afl::except::SystemException(afl::sys::Error(errno), "<ProcessRunner.start: pipe>");
    }

    // Avoid file handle inheritance
    ::fcntl(command_pipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(command_pipe[1], F_SETFD, FD_CLOEXEC);
    ::fcntl(result_pipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(result_pipe[1], F_SETFD, FD_CLOEXEC);

    // Create worker
    pid_t pid = ::fork();
    if (pid < 0) {
        throw afl::except::SystemException(afl::sys::Error(errno), "<ProcessRunner.start: fork>");
    }
    if (pid == 0) {
        // I am the child. Read commands, write results, forever.
        ::close(command_pipe[1]);
        ::close(result_pipe[0]);
        use(command_pipe[0], result_pipe[1]);
        try {
            while (serveRequest()) {
                // nix
            }
        }
        catch (std::exception& e) {
            m_writer.visitError("<ProcessRunner.start>", e.what());
        }
        _exit(127);
    }

    // I am the parent. Write commands, read results.
    ::close(command_pipe[0]);
    ::close(result_pipe[1]);
    use(result_pipe[0], command_pipe[1]);
    m_workerPid = pid;
}

// Main program: stop helper process.
inline void
util::ProcessRunner::Impl::stop()
{
    // Send stop notification
    m_writer.visitInteger(0);
    ::close(m_writeFD);
    ::close(m_readFD);

    // Wait for child to exit
    // We're shutting down and want to get rid of our worker, even if that worker has a bug or otherwise hangs.
    // Since there is no timed-waitpid syscall, we're using a wait loop.
    // This loop runs at most 1.5 seconds. If the child didn't exit after 0.5 seconds, give them a SIGTERM;
    // if they ignore that, give them a SIGKILL.
    // This initial usleep() will be enough most of the time, making tests run faster.
    ::usleep(5000);
    for (int i = 0; i < 30; ++i) {
        int status;
        int n = ::waitpid(m_workerPid, &status, WNOHANG);
        if (n > 0) {
            break;
        }
        if (i == 10) {
            ::kill(m_workerPid, SIGTERM);
        }
        if (i == 20) {
            ::kill(m_workerPid, SIGKILL);
        }
        ::usleep(50000);
    }
}

// Main program: Run child process.
inline int
util::ProcessRunner::Impl::run(const Command& cmd, String_t& output)
{
    // ex Runner::run
    afl::sys::MutexGuard guard(m_mutex);

    // Send command
    m_writer.visitInteger(1);
    m_writer.visitVector(*afl::data::Vector::create(afl::data::Segment().pushBackElements(cmd.command)));
    if (const String_t* p = cmd.workDirectory.get()) {
        m_writer.visitString(*p);
    } else {
        m_writer.visitNull();
    }

    // Read result
    while (1) {
        std::auto_ptr<afl::data::Value> val(readObject());
        String_t valstr(afl::data::Access(val).toString());
        if (valstr.empty()) {
            break;
        }
        output += valstr;
    }

    // Read exit code
    std::auto_ptr<afl::data::Value> exitCodeVal(readObject());
    return afl::data::Access(exitCodeVal).toInteger();
}

// Helper process: serve a single request.
inline bool
util::ProcessRunner::Impl::serveRequest()
{
    // ex Runner::serveRequest
    // Read a boolean. This tells us whether to proceed
    std::auto_ptr<afl::data::Value> p(readObject());
    if (afl::data::Access(p).toInteger() == 0) {
        return false;
    }

    // Read an object. This is the command line.
    p.reset(readObject());

    // Read optional workdir
    std::auto_ptr<afl::data::Value> workdir(readObject());

    // Validate
    afl::data::Access a(p);
    if (a.getArraySize() == 0) {
        throw std::runtime_error("<ProcessRunner.serveRequest: protocol error>");
    }

    // It's valid, so we can execute it. Build argv.
    std::vector<String_t> argv_str;
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        argv_str.push_back(a[i].toString());
    }
    std::vector<const char*> argv_c;
    for (size_t i = 0, n = argv_str.size(); i < n; ++i) {
        argv_c.push_back(argv_str[i].c_str());
    }
    argv_c.push_back(0);

    // Pipe to communicate with child
    int fd[2];
    if (::pipe(fd) < 0) {
        throw std::runtime_error("<ProcessRunner.serveRequest: pipe failed>");
    }

    // Create child
    pid_t pid = ::fork();
    if (pid < 0) {
        ::close(fd[0]);
        ::close(fd[1]);
        throw std::runtime_error("<ProcessRunner.serveRequest: fork failed>");
    }

    if (pid == 0) {
        // I am the child
        ::close(fd[0]);
        ::close(m_readFD);
        ::close(m_writeFD);

        // Set up child's stdin/out/err
        int null = ::open("/dev/null", O_RDONLY);
        if (null >= 0) {
            ::dup2(null, STDIN_FILENO);
            ::close(null);
        }
        ::dup2(fd[1], STDOUT_FILENO);
        ::dup2(fd[1], STDERR_FILENO);
        ::close(fd[1]);

        // Change directory
        if (workdir.get() != 0) {
            String_t dir = afl::data::Access(workdir).toString();
            if (::chdir(dir.c_str()) != 0) {
                ::perror(dir.c_str());
                ::_exit(126);
            }
        }

        // Run child
        ::execvp(argv_c[0], const_cast<char**>(&argv_c[0]));
        ::perror(argv_c[0]);
        ::_exit(127);
    }

    // I am the parent. Read child's stdout, and send to caller.
    ::close(fd[1]);
    char buffer[1024];
    ssize_t n;
    while ((n = ::read(fd[0], buffer, sizeof(buffer))) > 0) {
        m_writer.visitString(String_t(buffer, n));
    }
    ::close(fd[0]);
    m_writer.visitNull();

    // Child has closed its stdout. Wait for it to exit.
    int status;
    pid_t result = ::waitpid(pid, &status, 0);
    if (result < 0) {
        throw std::runtime_error("<ProcessRunner.serveRequest: waitpid failed>");
    } else if (WIFEXITED(status)) {
        m_writer.visitInteger(WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        m_writer.visitInteger(1000 + WTERMSIG(status));
    } else {
        m_writer.visitInteger(1999);
    }
    return true;
}

// Main/helper: send data to other side.
bool
util::ProcessRunner::Impl::handleData(afl::base::ConstBytes_t& data)
{
    while (!data.empty()) {
        ssize_t n = ::write(m_writeFD, data.unsafeData(), data.size());
        if (n <= 0) {
            return false;
        }
        data.split(n);
    }
    return true;
}

// Main/helper: read object from other side.
afl::data::Value*
util::ProcessRunner::Impl::readObject()
{
    afl::data::DefaultValueFactory factory;
    afl::io::resp::Parser parser(factory);
    while (1) {
        if (m_readDescriptor.empty()) {
            ssize_t n = ::read(m_readFD, m_readBuffer, sizeof(m_readBuffer));
            if (n <= 0) {
                throw std::runtime_error("<ProcessRunner.readObject: IPC error>");
            }
            m_readDescriptor = afl::base::ConstBytes_t(m_readBuffer).trim(n);
        }
        if (parser.handleData(m_readDescriptor)) {
            return parser.extract();
        }
    }
}
#else
/*
 *  Non-POSIX Implementation
 *  (as of 20170729, just a dummy)
 */
# include "afl/except/unsupportedexception.hpp"

class util::ProcessRunner::Impl {
 public:
    void start()
        { }
    void stop()
        { }
    int run(const Command& /*cmd*/, String_t& /*output*/)
        { throw afl::except::UnsupportedException("<ProcessRunner.run>"); }
};
#endif

/***************************** ProcessRunner *****************************/

util::ProcessRunner::ProcessRunner()
    : m_pImpl(new Impl())
{
    m_pImpl->start();
}

util::ProcessRunner::~ProcessRunner()
{
    m_pImpl->stop();
}

int
util::ProcessRunner::run(const Command& cmd, String_t& output)
{
    return m_pImpl->run(cmd, output);
}
