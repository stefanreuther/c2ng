/**
  *  \file test/ui/res/engineimageloadertest.cpp
  *  \brief Test for ui::res::EngineImageLoader
  */

#include "ui/res/engineimageloader.hpp"

#include "afl/io/internalstream.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/engine.hpp"
#include "gfx/nullcanvas.hpp"
#include <stdexcept>

namespace {
    class BaseTestEngine : public gfx::Engine {
     public:
        virtual afl::base::Ref<gfx::Canvas> createWindow(const gfx::WindowParameters& /*param*/)
            { throw std::runtime_error("not implemented"); }
        virtual void handleEvent(gfx::EventConsumer& /*consumer*/, bool /*relativeMouseMovement*/)
            { throw std::runtime_error("not implemented"); }
        virtual util::Key_t getKeyboardModifierState()
            { return 0; }
        virtual util::RequestDispatcher& dispatcher()
            { throw std::runtime_error("not implemented"); }
        virtual afl::base::Ref<gfx::Timer> createTimer()
            { throw std::runtime_error("not implemented"); }

        // Method under test in this module:
        virtual afl::base::Ref<gfx::Canvas> loadImage(afl::io::Stream& file) = 0;
    };
}

/** Test success case. */
AFL_TEST("ui.res.EngineImageLoader:success", a)
{
    // Engine
    class TestEngine : public BaseTestEngine {
     public:
        TestEngine(afl::test::Assert a)
            : m_assert(a)
            { }
        virtual afl::base::Ref<gfx::Canvas> loadImage(afl::io::Stream& file)
            {
                m_assert.checkEqual("01. getName", file.getName(), "testOK");
                m_assert.checkEqual("02. getPos", file.getPos(), 0U);
                return *new gfx::NullCanvas();
            }
     private:
        afl::test::Assert m_assert;
    };
    TestEngine engine(a);

    // Stream
    afl::io::InternalStream stream;
    stream.setName("testOK");
    stream.fullWrite(afl::string::toBytes("hi"));
    a.checkEqual("11. getPos", stream.getPos(), 2U);

    // Test it
    ui::res::EngineImageLoader testee(engine);
    afl::base::Ptr<gfx::Canvas> result(testee.loadImage(stream));

    a.checkNonNull("21. result", result.get());
    a.checkNonNull("22. result", dynamic_cast<gfx::NullCanvas*>(result.get()));
}

/** Test error case. */
AFL_TEST("ui.res.EngineImageLoader:failure", a)
{
    // Engine
    class TestEngine : public BaseTestEngine {
     public:
        virtual afl::base::Ref<gfx::Canvas> loadImage(afl::io::Stream& /*file*/)
            { throw std::runtime_error("invalid image"); }
    };
    TestEngine engine;

    // Stream
    afl::io::InternalStream stream;

    // Test it
    ui::res::EngineImageLoader testee(engine);
    afl::base::Ptr<gfx::Canvas> result(testee.loadImage(stream));

    a.checkNull("01. result", result.get());
}
