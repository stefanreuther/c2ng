/**
  *  \file test/ui/eventlooptest.cpp
  *  \brief Test for ui::EventLoop
  */

#include "ui/eventloop.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"

/** Test normal behaviour. */
AFL_TEST("ui.EventLoop:stop", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Create loop
    ui::EventLoop loop(root);
    a.checkEqual("01. isStopped", loop.isStopped(), false);

    // stop() causes run() to exit immediately
    loop.stop(33);
    a.checkEqual("11. isStopped", loop.isStopped(), true);
    a.checkEqual("12. run", loop.run(), 33);

    a.checkEqual("21. isStopped", loop.isStopped(), false);
}

/** Test behaviour with tasks. */
AFL_TEST("ui.EventLoop:postNewRunnable", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Create loop
    ui::EventLoop loop(root);
    a.checkEqual("01. isStopped", loop.isStopped(), false);

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
    a.checkEqual("11. run", loop.run(), 77);
    a.checkEqual("12. isStopped", loop.isStopped(), false);
}
