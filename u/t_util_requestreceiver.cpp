/**
  *  \file u/t_util_requestreceiver.cpp
  *  \brief Test for util::RequestReceiver
  */

#include "util/requestreceiver.hpp"

#include "t_util.hpp"

namespace {
    struct Value {
        int i;
        Value(int i)
            : i(i)
            { }
    };

    // Simple dispatcher: direct execution (single-thread)
    class SimpleDispatcher : public util::RequestDispatcher {
     public:
        virtual void postNewRunnable(afl::base::Runnable* p)
            {
                p->run();
                delete p;
            }
    };

    // Simple request for testing: add to an integer
    class SimpleRequest : public util::Request<Value> {
     public:
        SimpleRequest(int n)
            : m_n(n)
            { }
        virtual void handle(Value& n)
            { n.i += m_n; }
     private:
        int m_n;
    };
}

/** Simple test. */
void
TestUtilRequestReceiver::testIt()
{
    // Define a simple dispatcher
    SimpleDispatcher disp;

    // Define a receiver with an object variable
    Value var(0);
    util::RequestReceiver<Value> rx(disp, var);

    // Post some requests
    rx.getSender().postNewRequest(new SimpleRequest(10));
    TS_ASSERT_EQUALS(var.i, 10);
    rx.getSender().postNewRequest(new SimpleRequest(20));
    TS_ASSERT_EQUALS(var.i, 30);
}

/** Test send after receiver dies. */
void
TestUtilRequestReceiver::testDie()
{
    // Define a simple dispatcher (must out-live everything)
    SimpleDispatcher disp;

    util::RequestSender<Value> sp;

    // Define a receiver with an object variable
    {
        std::auto_ptr<Value> pVar(new Value(42));
        util::RequestReceiver<Value> rx(disp, *pVar);
        sp = rx.getSender();

        sp.postNewRequest(new SimpleRequest(3));
        TS_ASSERT_EQUALS(pVar->i, 45);
        sp.postNewRequest(new SimpleRequest(2));
        TS_ASSERT_EQUALS(pVar->i, 47);
    }

    // Sending will still work, but no execution
    sp.postNewRequest(new SimpleRequest(10));
}

