/**
  *  \file test/util/process/posixfactorytest.cpp
  *  \brief Test for util::process::PosixFactory
  */

#include "util/process/posixfactory.hpp"

#include "afl/test/testrunner.hpp"
#include "util/process/subprocess.hpp"
#include <memory>

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
AFL_TEST("util.process.PosixFactory:basics", a)
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    a.check("01. get", p.get());

    // Start succeeds
    const String_t args[] = {
        "-c",
        "while read a; do echo +$a+; done"
    };

    a.check("11. isActive", !p->isActive());
    a.check("12. start", p->start("/bin/sh", args));
    a.check("13. isActive", p->isActive());
    a.checkDifferent("14. getProcessId", p->getProcessId(), 0U);

    // Communication succeeds
    String_t result;
    a.check("21. writeLine", p->writeLine("hi\n"));
    a.check("22. readLine", p->readLine(result));
    a.checkEqual("23. result", result, "+hi+\n");

    a.check("31. writeLine", p->writeLine("ho\n"));
    a.check("32. readLine", p->readLine(result));
    a.checkEqual("33. result", result, "+ho+\n");

    // Stop it
    a.check("41. stop", p->stop());
    a.check("42. getStatus", !p->getStatus().empty());
#endif
}

/** Test pipe stress. */
AFL_TEST("util.process.PosixFactory:error:pipe-stress-1", a)
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    a.check("01. get", p.get());

    // Start fails (cannot create first pipe)
    Stresser s;
    const String_t args[] = {
        "-c",
        "echo hi"
    };

    a.check("11. isActive", !p->isActive());
    a.check("12. start", !p->start("/bin/sh", args));
    a.check("13. isActive", !p->isActive());
    a.check("14. status", !p->getStatus().empty());
#endif
}

AFL_TEST("util.process.PosixFactory:error:pipe-stress-2", a)
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    a.check("01. get", p.get());

    // Start fails (cannot create second pipe)
    Stresser s;
    s.close();
    s.close();

    const String_t args[] = {
        "-c",
        "echo hi"
    };

    a.check("11. isActive", !p->isActive());
    a.check("12. start", !p->start("/bin/sh", args));
    a.check("13. isActive", !p->isActive());
    a.check("14. getStatus", !p->getStatus().empty());
#endif
}

AFL_TEST("util.process.PosixFactory:error:exec-fail", a)
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    a.check("01. get", p.get());

    // Start succeeds, sort-of
    a.check("11. isActive", !p->isActive());
    a.check("12. start", p->start("/this/program/does/not/exist", afl::base::Nothing));
    a.check("13. isActive", p->isActive());

    // Reading returns the error message
    String_t result;
    a.check("21. readLine", p->readLine(result));
    a.check("22. result", result.find("/this/program/does/not/exist") != String_t::npos);

    // Stop it
    a.check("31. stop", p->stop());
    a.check("32", !p->getStatus().empty());
#endif
}

AFL_TEST("util.process.PosixFactory:signal", a)
{
#if TARGET_OS_POSIX
    // Creation succeeds
    util::process::PosixFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    a.check("01. get", p.get());

    // Start succeeds
    const String_t args[] = {
        "-c",
        "kill -15 $$"
    };

    a.check("11. isActive", !p->isActive());
    a.check("12. start", p->start("/bin/sh", args));
    a.check("13. isActive", p->isActive());

    // Cannot read
    String_t result;
    a.check("21. readLine", !p->readLine(result));

    // Stop it
    a.check("31. stop", !p->stop());
    a.check("32. status", p->getStatus().find("signal") != String_t::npos);
#endif
}
