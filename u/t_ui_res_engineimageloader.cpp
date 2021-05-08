/**
  *  \file u/t_ui_res_engineimageloader.cpp
  *  \brief Test for ui::res::EngineImageLoader
  */

#include <stdexcept>
#include "ui/res/engineimageloader.hpp"

#include "t_ui_res.hpp"
#include "gfx/engine.hpp"
#include "gfx/nullcanvas.hpp"
#include "afl/io/internalstream.hpp"

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
void
TestUiResEngineImageLoader::testOK()
{
    // Engine
    class TestEngine : public BaseTestEngine {
     public:
        virtual afl::base::Ref<gfx::Canvas> loadImage(afl::io::Stream& file)
            {
                TS_ASSERT_EQUALS(file.getName(), "testOK");
                TS_ASSERT_EQUALS(file.getPos(), 0U);
                return *new gfx::NullCanvas();
            }
    };
    TestEngine engine;

    // Stream
    afl::io::InternalStream stream;
    stream.setName("testOK");
    stream.fullWrite(afl::string::toBytes("hi"));
    TS_ASSERT_EQUALS(stream.getPos(), 2U);

    // Test it
    ui::res::EngineImageLoader testee(engine);
    afl::base::Ptr<gfx::Canvas> result(testee.loadImage(stream));

    TS_ASSERT(result.get() != 0);
    TS_ASSERT(dynamic_cast<gfx::NullCanvas*>(result.get()) != 0);
}

/** Test error case. */
void
TestUiResEngineImageLoader::testFail()
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

    TS_ASSERT(result.get() == 0);
}
