/**
  *  \file u/t_ui_eventloop.cpp
  *  \brief Test for ui::EventLoop
  */

#include "ui/eventloop.hpp"

#include "t_ui.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"

/** Test normal behaviour. */
void
TestUiEventLoop::testStop()
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Create loop
    ui::EventLoop loop(root);
    TS_ASSERT_EQUALS(loop.isStopped(), false);

    // stop() causes run() to exit immediately
    loop.stop(33);
    TS_ASSERT_EQUALS(loop.isStopped(), true);
    TS_ASSERT_EQUALS(loop.run(), 33);

    TS_ASSERT_EQUALS(loop.isStopped(), false);
}

/** Test behaviour with tasks. */
void
TestUiEventLoop::testTask()
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Create loop
    ui::EventLoop loop(root);
    TS_ASSERT_EQUALS(loop.isStopped(), false);

    // Post a task that will cause it to stop
    typedef afl::base::Closure<void(int)> Closure_t;
    class Task : public afl::base::Runnable {
     public:
        Task(Closure_t* p)
            : m_p(p)
            { }
        void run()
            { m_p->call(0); }
     private:
        std::auto_ptr<Closure_t> m_p;
    };
    engine.dispatcher().postNewRunnable(new Task(loop.makeStop(77)));

    // Run will stop once Engine exexcutes the Runnable
    TS_ASSERT_EQUALS(loop.run(), 77);
    TS_ASSERT_EQUALS(loop.isStopped(), false);
}

