/**
  *  \file u/t_util_process_posixfactory.cpp
  *  \brief Test for util::Process::PosixFactory
  */

#include "util/process/posixfactory.hpp"

#include <memory>
#include "t_util_process.hpp"
#include "util/process/subprocess.hpp"

#ifdef TARGET_OS_POSIX
#  include <vector>
#  include <unistd.h>
#  include <sys/resource.h>
namespace {
    /*
     *  A File Descriptor Stresser
     *
     *  Allocates as many file descriptors as it can, to trigger error cases.
     */
    class Stresser {
     public:
        Stresser()
            : m_limitOK(false), m_limit(), m_fds()
            {
                // Get and lower resource limit
                // By default, the limit on open files is 65535 on my Linux system.
                // Despite the system being able to handle that easily,
                // reducing the limit makes this test behave a little nicer.
                // 'getrlimit(RLIMIT_NOFILE)' is part of SUSv2 (1997), so we shouldn't need a feature test.
                if (::getrlimit(RLIMIT_NOFILE, &m_limit) == 0) {
                    rlimit newLimit = m_limit;
                    newLimit.rlim_cur = 128;
                    ::setrlimit(RLIMIT_NOFILE, &newLimit);
                    m_limitOK = true;
                }

                // Block all fds
                int fd;
                while ((fd = dup(0)) >= 0) {
                    m_fds.push_back(fd);
                }
            }

        ~Stresser()
            {
                // Release all fds
                while (close()) { }

                // Restore limit
                if (m_limitOK) {
                    ::setrlimit(RLIMIT_NOFILE, &m_limit);
                }
            }

        bool close()
            {
                if (m_fds.empty()) {
                    return false;
                } else {
                    ::close(m_fds.back());
                    m_fds.pop_back();
                    return true;
                }
            }

     private:
        bool m_limitOK;
        rlimit m_limit;
        std::vector<int> m_fds;
    };
}
#endif

/** Simple test. Creates a simple process and talks to it. */
void
TestUtilProcessPosixFactory::testIt()
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    TS_ASSERT(p.get());

    // Start succeeds
    const String_t args[] = {
        "-c",
        "while read a; do echo +$a+; done"
    };

    TS_ASSERT(!p->isActive());
    TS_ASSERT(p->start("/bin/sh", args));
    TS_ASSERT(p->isActive());
    TS_ASSERT_DIFFERS(p->getProcessId(), 0U);

    // Communication succeeds
    String_t result;
    TS_ASSERT(p->writeLine("hi\n"));
    TS_ASSERT(p->readLine(result));
    TS_ASSERT_EQUALS(result, "+hi+\n");

    TS_ASSERT(p->writeLine("ho\n"));
    TS_ASSERT(p->readLine(result));
    TS_ASSERT_EQUALS(result, "+ho+\n");

    // Stop it
    TS_ASSERT(p->stop());
    TS_ASSERT(!p->getStatus().empty());
#endif
}

/** Test pipe stress. */
void
TestUtilProcessPosixFactory::testPipeStress1()
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    TS_ASSERT(p.get());

    // Start fails (cannot create first pipe)
    Stresser s;
    const String_t args[] = {
        "-c",
        "echo hi"
    };

    TS_ASSERT(!p->isActive());
    TS_ASSERT(!p->start("/bin/sh", args));
    TS_ASSERT(!p->isActive());
    TS_ASSERT(!p->getStatus().empty());
#endif
}

void
TestUtilProcessPosixFactory::testPipeStress2()
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    TS_ASSERT(p.get());

    // Start fails (cannot create second pipe)
    Stresser s;
    s.close();
    s.close();

    const String_t args[] = {
        "-c",
        "echo hi"
    };

    TS_ASSERT(!p->isActive());
    TS_ASSERT(!p->start("/bin/sh", args));
    TS_ASSERT(!p->isActive());
    TS_ASSERT(!p->getStatus().empty());
#endif
}

void
TestUtilProcessPosixFactory::testExecFail()
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    TS_ASSERT(p.get());

    // Start succeeds, sort-of
    TS_ASSERT(!p->isActive());
    TS_ASSERT(p->start("/this/program/does/not/exist", afl::base::Nothing));
    TS_ASSERT(p->isActive());

    // Reading returns the error message
    String_t result;
    TS_ASSERT(p->readLine(result));
    TS_ASSERT(result.find("/this/program/does/not/exist") != String_t::npos);

    // Stop it
    TS_ASSERT(p->stop());
    TS_ASSERT(!p->getStatus().empty());
#endif
}

void
TestUtilProcessPosixFactory::testSignal()
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    TS_ASSERT(p.get());

    // Start succeeds
    const String_t args[] = {
        "-c",
        "kill -15 $$"
    };

    TS_ASSERT(!p->isActive());
    TS_ASSERT(p->start("/bin/sh", args));
    TS_ASSERT(p->isActive());

    // Cannot read
    String_t result;
    TS_ASSERT(!p->readLine(result));

    // Stop it
    TS_ASSERT(!p->stop());
    TS_ASSERT(p->getStatus().find("signal") != String_t::npos);
#endif
}

