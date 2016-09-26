/**
  *  \file u/t_util_slaverequestsender.cpp
  *  \brief Test for util::SlaveRequestSender
  */

#include "util/slaverequestsender.hpp"

#include "t_util.hpp"
#include "afl/string/string.hpp"
#include "util/slaverequest.hpp"
#include "util/slaveobject.hpp"
#include "afl/sys/semaphore.hpp"
#include "util/requestthread.hpp"
#include "util/requestreceiver.hpp"
#include "afl/sys/log.hpp"

namespace {
    class TestSlave : public util::SlaveObject<String_t> {
     public:
        void init(String_t& s)
            { s += "init."; }
        void done(String_t& s)
            { s += "done."; }

        String_t m_helper;
    };

    /** Synchronize a RequestDispatcher.
        After this function returns, all requests that have been posted before have finished executing. */
    void synchronize(util::RequestDispatcher& t)
    {
        afl::sys::Semaphore sem(0);
        class Cleaner : public afl::base::Runnable {
         public:
            Cleaner(afl::sys::Semaphore& sem)
                : m_sem(sem)
                { }
            void run()
                { m_sem.post(); }
         private:
            afl::sys::Semaphore& m_sem;
        };
        t.postNewRunnable(new Cleaner(sem));
        sem.wait();
    }
}

/** Simple test.
    Tests just construction and destruction. */
void
TestUtilSlaveRequestSender::testIt()
{
    // Start with a master object.
    String_t master;
    afl::sys::Log log;
    util::RequestThread masterThread("TestUtilSlaveRequestSender", log);
    util::RequestReceiver<String_t> masterReceiver(masterThread, master);
    util::RequestSender<String_t> masterSender(masterReceiver.getSender());

    // Add a slave object.
    {
        util::SlaveRequestSender<String_t,TestSlave> testee(masterSender, new TestSlave());
    }

    // Clean the pipe.
    synchronize(masterThread);

    // Validate the string.
    TS_ASSERT_EQUALS(master, "init.done.");
}

/** Test calling functions. */
void
TestUtilSlaveRequestSender::testCall()
{
    // Start with a master object.
    String_t master;
    afl::sys::Log log;
    util::RequestThread masterThread("TestUtilSlaveRequestSender", log);
    util::RequestReceiver<String_t> masterReceiver(masterThread, master);
    util::RequestSender<String_t> masterSender(masterReceiver.getSender());

    // Add a slave object and give it some commands.
    {
        class SetRequest : public util::SlaveRequest<String_t,TestSlave> {
         public:
            SetRequest(String_t value)
                : m_value(value)
                { }
            void handle(String_t&, TestSlave& obj)
                {
                    obj.m_helper = m_value;
                }
         private:
            String_t m_value;
        };
        class AddRequest : public util::SlaveRequest<String_t,TestSlave> {
         public:
            void handle(String_t& master, TestSlave& obj)
                {
                    master += obj.m_helper;
                }
        };
        
        util::SlaveRequestSender<String_t,TestSlave> testee(masterSender, new TestSlave());
        testee.postNewRequest(new SetRequest("hi."));
        testee.postNewRequest(new AddRequest());
        testee.postNewRequest(new SetRequest("ho."));
        testee.postNewRequest(new AddRequest());
        testee.postNewRequest(new AddRequest());
    }

    // Clean the pipe.
    synchronize(masterThread);

    // Validate the string.
    TS_ASSERT_EQUALS(master, "init.hi.ho.ho.done.");
}
