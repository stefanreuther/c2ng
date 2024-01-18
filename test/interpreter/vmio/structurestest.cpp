/**
  *  \file test/interpreter/vmio/structurestest.cpp
  *  \brief Test for interpreter::vmio::Structures
  */

#include "interpreter/vmio/structures.hpp"
#include "afl/test/testrunner.hpp"

/** Test ProcessKind serialisation. */
AFL_TEST("interpreter.vmio.Structures:ProcessKind", a)
{
    using interpreter::vmio::structures::ProcessKind_t;
    using interpreter::Process;

    ProcessKind_t str;
    Process::ProcessKind k;

    // 0 = default
    {
        str = Process::pkDefault;
        a.checkEqual("01. pkDefault in", str.m_bytes[0], 0);
        k = str;
        a.checkEqual("02. pkDefault out)", k, Process::pkDefault);
    }

    // 1 = ship task
    {
        str = Process::pkShipTask;
        a.checkEqual("11. pkShipTask in", str.m_bytes[0], 1);
        k = str;
        a.checkEqual("12. pkShipTask out", k, Process::pkShipTask);
    }

    // 2 = planet task
    {
        str = Process::pkPlanetTask;
        a.checkEqual("21. pkPlanetTask in", str.m_bytes[0], 2);
        k = str;
        a.checkEqual("22. pkPlanetTask out", k, Process::pkPlanetTask);
    }

    // 3 = base task
    {
        str = Process::pkBaseTask;
        a.checkEqual("31. pkBaseTask in", str.m_bytes[0], 3);
        k = str;
        a.checkEqual("32. pkBaseTask out", k, Process::pkBaseTask);
    }

    // anything else decodes to default
    {
        str.m_bytes[0] = 99;
        k = str;
        a.checkEqual("41. other", k, Process::pkDefault);
    }
}
