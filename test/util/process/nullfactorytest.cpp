/**
  *  \file test/util/process/nullfactorytest.cpp
  *  \brief Test for util::process::NullFactory
  */

#include "util/process/nullfactory.hpp"

#include "afl/test/testrunner.hpp"
#include "util/process/subprocess.hpp"
#include <memory>

/** Simple test. Just calls all functions and validates their results. */
AFL_TEST("util.process.NullFactory", a)
{
    // Creation succeeds
    util::process::NullFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    a.check("01. get", p.get());

    // Start fails
    a.check("11. isActive", !p->isActive());
    a.check("12. start", !p->start("echo", afl::base::Nothing));
    a.check("13. getStatus", !p->getStatus().empty());

    // I/O fails
    String_t line;
    a.check("21. writeLine", !p->writeLine(line));
    a.check("22. readLine", !p->readLine(line));

    // Stop succeeds; process is stopped afterwards
    a.check("31. stop", p->stop());

    // We don't have a process Id
    a.checkEqual("41. getProcessId", p->getProcessId(), 0U);
}
