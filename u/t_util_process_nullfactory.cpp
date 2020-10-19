/**
  *  \file u/t_util_process_nullfactory.cpp
  *  \brief Test for util::Process::NullFactory
  */

#include "util/process/nullfactory.hpp"

#include <memory>
#include "t_util_process.hpp"
#include "util/process/subprocess.hpp"

/** Simple test. Just calls all functions and validates their results. */
void
TestUtilProcessNullFactory::testIt()
{
    // Creation succeeds
    util::process::NullFactory testee;
    std::auto_ptr<util::process::Subprocess> p(testee.createNewProcess());
    TS_ASSERT(p.get());

    // Start fails
    TS_ASSERT(!p->isActive());
    TS_ASSERT(!p->start("echo", afl::base::Nothing));
    TS_ASSERT(!p->getStatus().empty());

    // I/O fails
    String_t line;
    TS_ASSERT(!p->writeLine(line));
    TS_ASSERT(!p->readLine(line));

    // Stop succeeds; process is stopped afterwards
    TS_ASSERT(p->stop());

    // We don't have a process Id
    TS_ASSERT_EQUALS(p->getProcessId(), 0U);
}

