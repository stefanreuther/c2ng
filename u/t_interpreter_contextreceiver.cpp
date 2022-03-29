/**
  *  \file u/t_interpreter_contextreceiver.cpp
  *  \brief Test for interpreter::ContextReceiver
  */

#include "interpreter/contextreceiver.hpp"

#include "t_interpreter.hpp"

/** Interface test. */
void
TestInterpreterContextReceiver::testInterface()
{
    class Tester : public interpreter::ContextReceiver {
     public:
        virtual void pushNewContext(interpreter::Context* /*pContext*/)
            { }
    };
    Tester t;
}

