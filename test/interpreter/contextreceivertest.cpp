/**
  *  \file test/interpreter/contextreceivertest.cpp
  *  \brief Test for interpreter::ContextReceiver
  */

#include "interpreter/contextreceiver.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("interpreter.ContextReceiver")
{
    class Tester : public interpreter::ContextReceiver {
     public:
        virtual void pushNewContext(interpreter::Context* /*pContext*/)
            { }
    };
    Tester t;
}
