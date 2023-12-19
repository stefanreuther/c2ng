/**
  *  \file u/t_interpreter_lockaccess.cpp
  *  \brief Test for interpreter::LockAccess
  */

#include "interpreter/lockaccess.hpp"

#include "t_interpreter.hpp"

/** Interface test. */
void
TestInterpreterLockAccess::testInterface()
{
    class Tester : public interpreter::LockAccess {
     public:
        virtual bool hasLock(const String_t& /*name*/) const
            { return false; }
    };
    Tester t;
}

