/**
  *  \file test/util/requestreceivertest.cpp
  *  \brief Test for util::RequestReceiver
  */

#include "util/requestreceiver.hpp"
#include "afl/test/testrunner.hpp"

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
AFL_TEST("util.RequestReceiver:basics", a)
{
    // Define a simple dispatcher
    SimpleDispatcher disp;

    // Define a receiver with an object variable
    Value var(0);
    util::RequestReceiver<Value> rx(disp, var);

    // Post some requests
    rx.getSender().postNewRequest(new SimpleRequest(10));
    a.checkEqual("01", var.i, 10);
    rx.getSender().postNewRequest(new SimpleRequest(20));
    a.checkEqual("02", var.i, 30);
}

/** Test send after receiver dies. */
AFL_TEST("util.RequestReceiver:send-after-receiver-death", a)
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
        a.checkEqual("01", pVar->i, 45);
        sp.postNewRequest(new SimpleRequest(2));
        a.checkEqual("02", pVar->i, 47);
    }

    // Sending will still work, but no execution
    sp.postNewRequest(new SimpleRequest(10));
}
