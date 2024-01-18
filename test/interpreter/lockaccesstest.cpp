/**
  *  \file test/interpreter/lockaccesstest.cpp
  *  \brief Test for interpreter::LockAccess
  */

#include "interpreter/lockaccess.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST("interpreter.LockAccess", a)
{
    class Tester : public interpreter::LockAccess {
     public:
        virtual bool hasLock(const String_t& /*name*/) const
            { return false; }
    };
    Tester t;
}
