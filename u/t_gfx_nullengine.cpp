/**
  *  \file u/t_gfx_nullengine.cpp
  *  \brief Test for gfx::NullEngine
  */

#include "gfx/nullengine.hpp"

#include "t_gfx.hpp"
#include "gfx/eventconsumer.hpp"
#include "afl/string/format.hpp"

namespace {
    /* A counter. Usable both as Runnable descendant and as signal target. */
    class Counter : public afl::base::Runnable {
     public:
        Counter(int& count)
            : m_count(count)
            { }
        void run()
            { ++m_count; }
     private:
        int& m_count;
    };

    /* Event consumer. Verifies that no actual user events happen. */
    class Consumer : public gfx::EventConsumer {
     public:
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            {
                TS_ASSERT(0);
                return false;
            }
        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
            {
                TS_ASSERT(0);
                return false;
            }
    };

    /* Event consumer that saves stuff. */
    class SavingConsumer : public gfx::EventConsumer {
     public:
        SavingConsumer()
            : m_acc()
            { }
        virtual bool handleKey(util::Key_t key, int /*prefix*/)
            {
                m_acc += afl::string::Format("key:%s\n", util::formatKey(key));
                return true;
            }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t /*pressedButtons*/)
            {
                m_acc += afl::string::Format("mouse:%d,%d\n", pt.getX(), pt.getY());
                return true;
            }
        String_t get() const
            { return m_acc; }
     private:
        String_t m_acc;
    };
}

/** Test timer stuff.
    Verifies that timers work as advertised. */
void
TestGfxNullEngine::testTimers()
{
    // Event counter
    int numRun1 = 0, numRun2 = 0;
    int numTimer1 = 0, numTimer2 = 0;

    // Create stuff
    gfx::NullEngine t;
    afl::base::Ref<gfx::Timer> time1 = t.createTimer();
    afl::base::Ref<gfx::Timer> time2 = t.createTimer();
    TS_ASSERT(&time1.get() != 0);
    TS_ASSERT(&time2.get() != 0);

    // Set up everything
    Counter ctrTimer1(numTimer1);
    Counter ctrTimer2(numTimer2);
    time1->sig_fire.add(&ctrTimer1, &Counter::run);
    time2->sig_fire.add(&ctrTimer2, &Counter::run);
    time1->setInterval(20);
    time2->setInterval(50);
    t.dispatcher().postNewRunnable(new Counter(numRun1));
    t.dispatcher().postNewRunnable(new Counter(numRun2));

    // Process events
    Consumer c;
    int n = 0;
    while (numRun1 == 0 || numRun2 == 0 || numTimer1 == 0 || numTimer2 == 0) {
        // Check sequencing: runnable 1 must run first, then runnable 2, then timer 1, then timer 2.
        TS_ASSERT_LESS_THAN_EQUALS(numRun2, numRun1);
        TS_ASSERT_LESS_THAN_EQUALS(numTimer1, numRun2);
        TS_ASSERT_LESS_THAN_EQUALS(numTimer2, numTimer1);
        t.handleEvent(c, false);

        // We have four events, so we need at most four loops through.
        ++n;
        TS_ASSERT_LESS_THAN_EQUALS(n, 4);
    }
}

/** Test event stuff.
    Verifies that event injection works as advertised. */
void
TestGfxNullEngine::testEvents()
{
    // Create stuff
    gfx::NullEngine t;
    SavingConsumer c;
    TS_ASSERT_EQUALS(c.get(), "");

    // Fire and process events. Events must not be reordered.
    t.postKey(util::Key_Escape, 0);
    t.postMouse(gfx::Point(100, 200), gfx::EventConsumer::MouseButtons_t());
    t.postMouse(gfx::Point(100, 201), gfx::EventConsumer::MouseButtons_t());
    t.postKey(util::Key_Return, 0);

    // Verify stringifications to fail early.
    TS_ASSERT_EQUALS(util::formatKey(util::Key_Return), "RET");
    TS_ASSERT_EQUALS(util::formatKey(util::Key_Escape), "ESC");
    TS_ASSERT_EQUALS(util::formatKey('a'), "A");

    // Do it.
    // We allow a few more loops than required because handleEvent is allowed to return without having processed an event we know about.
    const char EXPECT[] =
        "key:ESC\n"
        "mouse:100,200\n"
        "mouse:100,201\n"
        "key:RET\n"
        "key:A\n";

    bool did = false;
    for (int i = 0; i < 20; ++i) {
        if (c.get() == EXPECT) {
            break;
        }
        if (!did && !c.get().empty()) {
            // Inject another event in the middle. Must not overtake the others.
            t.postKey('a', 0);
            did = true;
        }
        t.handleEvent(c, false);
    }
    TS_ASSERT_EQUALS(c.get(), EXPECT);
}
