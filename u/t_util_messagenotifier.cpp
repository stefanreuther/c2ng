/**
  *  \file u/t_util_messagenotifier.cpp
  *  \brief Test for util::MessageNotifier
  */

#include "util/messagenotifier.hpp"

#include "t_util.hpp"
#include "util/requestdispatcher.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/base/closure.hpp"

/** Simple test. */
void
TestUtilMessageNotifier::testIt()
{
    // Simple RequestDispatcher implementation for single-threaded execution.
    // FIXME: make this a re-usable component?
    class MyDispatcher : public util::RequestDispatcher {
     public:
        virtual void postNewRunnable(afl::base::Runnable* p)
            {
                TS_ASSERT(p != 0);
                m_stuff.pushBackNew(p);
            }
        void execute()
            {
                while (!m_stuff.empty()) {
                    afl::container::PtrVector<afl::base::Runnable> stuff;
                    stuff.swap(m_stuff);
                    for (size_t i = 0, n = stuff.size(); i < n; ++i) {
                        stuff[i]->run();
                    }
                }
            }
     private:
        afl::container::PtrVector<afl::base::Runnable> m_stuff;
    };
    MyDispatcher dispatcher;

    // Signal logger
    class MyLogger : public afl::base::Closure<void()> {
     public:
        MyLogger(int& count)
            : m_count(count)
            { }
        virtual void call()
            { ++m_count; }
        virtual MyLogger* clone() const
            { return new MyLogger(m_count); }
     private:
        int& m_count;
    };

    // Testee
    util::MessageNotifier testee(dispatcher);
    int count = 0;
    testee.sig_change.addNewClosure(new MyLogger(count));
    TS_ASSERT_EQUALS(count, 0);

    // Write a message. Callback does not immediately appear because it must be dispatched to the thread.
    testee.write(afl::sys::LogListener::Warn, "hi", "ho");
    TS_ASSERT_EQUALS(count, 0);

    // Trigger dispatcher, this will produce one callback.
    dispatcher.execute();
    TS_ASSERT_EQUALS(count, 1);
    count = 0;

    // Write more messages. This will eventually produce two callbacks (normal + retriggered).
    for (int i = 0; i < 10; ++i) {
        testee.write(afl::sys::LogListener::Warn, "hi", "ho");
    }
    TS_ASSERT_EQUALS(count, 0);
    dispatcher.execute();
    dispatcher.execute();
    dispatcher.execute();
    TS_ASSERT_EQUALS(count, 2);
}
