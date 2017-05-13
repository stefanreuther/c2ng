/**
  *  \file u/t_interpreter_vmio_structures.cpp
  *  \brief Test for interpreter::vmio::Structures
  */

#include "interpreter/vmio/structures.hpp"

#include "t_interpreter_vmio.hpp"

/** Test ProcessKind serialisation. */
void
TestInterpreterVmioStructures::testProcessKind()
{
    using interpreter::vmio::structures::ProcessKind_t;
    using interpreter::Process;

    ProcessKind_t a;
    Process::ProcessKind k;

    // 0 = default
    {
        a = Process::pkDefault;
        TS_ASSERT_EQUALS(a.m_bytes[0], 0);
        k = a;
        TS_ASSERT_EQUALS(k, Process::pkDefault);
    }

    // 1 = ship task
    {
        a = Process::pkShipTask;
        TS_ASSERT_EQUALS(a.m_bytes[0], 1);
        k = a;
        TS_ASSERT_EQUALS(k, Process::pkShipTask);
    }

    // 2 = planet task
    {
        a = Process::pkPlanetTask;
        TS_ASSERT_EQUALS(a.m_bytes[0], 2);
        k = a;
        TS_ASSERT_EQUALS(k, Process::pkPlanetTask);
    }

    // 3 = base task
    {
        a = Process::pkBaseTask;
        TS_ASSERT_EQUALS(a.m_bytes[0], 3);
        k = a;
        TS_ASSERT_EQUALS(k, Process::pkBaseTask);
    }

    // anything else decodes to default
    {
        a.m_bytes[0] = 99;
        k = a;
        TS_ASSERT_EQUALS(k, Process::pkDefault);
    }
}
