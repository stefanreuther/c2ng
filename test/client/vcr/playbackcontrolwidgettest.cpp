/**
  *  \file test/client/vcr/playbackcontrolwidgettest.cpp
  *  \brief Test for client::vcr::PlaybackControlWidget
  */

#include "client/vcr/playbackcontrolwidget.hpp"

#include "afl/base/countof.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/nullcanvas.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"

using afl::string::Format;

namespace {
    /* An environment placing the PlaybackControlWidget on a null graphics engine,
       and capturing all callbacks as CallReceiver. */
    struct Environment : public afl::test::CallReceiver {
        gfx::NullEngine engine;
        gfx::NullResourceProvider provider;
        ui::Root root;
        client::vcr::PlaybackControlWidget testee;

        /* Constructor.
           @param a Asserter
           @param acceptShiftMove Parameter to PlaybackControlWidget constructor */
        Environment(afl::test::Assert a, bool acceptShiftMove)
            : CallReceiver(a),
              engine(),
              provider(),
              root(engine, provider, gfx::WindowParameters()),
              testee(root, acceptShiftMove)
            {
                testee.sig_togglePlay.add(this, &Environment::onTogglePlay);
                testee.sig_moveBy.add(this, &Environment::onMoveBy);
                testee.sig_moveToBeginning.add(this, &Environment::onMoveToBeginning);
                testee.sig_moveToEnd.add(this, &Environment::onMoveToEnd);
                testee.sig_changeSpeed.add(this, &Environment::onChangeSpeed);

                // Widget must be on root to legally consume events.
                // Also this verifies that our (empty) UI management callbacks work sufficiently well.
                root.add(testee);
                testee.setExtent(gfx::Rectangle(10, 10, 80, 30));
            }

        void onTogglePlay()
            { checkCall("onTogglePlay()"); }
        void onMoveBy(int delta)
            { checkCall(Format("onMoveBy(%d)", delta)); }
        void onMoveToBeginning()
            { checkCall("onMoveToBeginning()"); }
        void onMoveToEnd()
            { checkCall("onMoveToEnd()"); }
        void onChangeSpeed(bool faster)
            { checkCall(Format("onChangeSpeed(%d)", (int) faster)); }
    };
}

/* Baseline test: key that is not defined does not generate a callback */
AFL_TEST("client.vcr.PlaybackControlWidget:base", a)
{
    Environment env(a, false);
    a.check("handleKey", !env.testee.handleKey('x', 0));
    env.checkFinish();
}

/*
 *  Keys
 */

namespace {
    struct Map {
        const char* name;
        uint32_t key;
        const char* expect;
    };

    const Map MAP[] = {
        { "a-left",  util::KeyMod_Alt + util::Key_Left,    "onMoveToBeginning()" },
        { "a-right", util::KeyMod_Alt + util::Key_Right,   "onMoveToEnd()" },
        { "c-left",  util::KeyMod_Ctrl + util::Key_Left,   "onMoveBy(-20)" },
        { "c-right", util::KeyMod_Ctrl + util::Key_Right,  "onMoveBy(20)" },
        { "s-left",  util::KeyMod_Shift + util::Key_Left,  "onMoveBy(-1)" },
        { "s-right", util::KeyMod_Shift + util::Key_Right, "onMoveBy(1)" },
        { "minus",   '-',                                  "onChangeSpeed(0)" },
        { "plus",    '+',                                  "onChangeSpeed(1)" },
        { "return",  util::Key_Return,                     "onTogglePlay()" },
        { "right",   util::Key_Right,                      "onTogglePlay()" },
        { "s-b",     'B',                                  "onMoveBy(-1)" },
        { "s-f",     'F',                                  "onMoveBy(1)" },
        { "space",   ' ',                                  "onTogglePlay()" },
    };
}

AFL_TEST("client.vcr.PlaybackControlWidget:keys", a)
{
    for (size_t i = 0; i < countof(MAP); ++i) {
        afl::test::Assert sub(a(MAP[i].name));
        Environment env(sub, true);
        env.expectCall(MAP[i].expect);
        sub.check("handleKey", env.testee.handleKey(MAP[i].key, 0));
        env.checkFinish();
    }
}

AFL_TEST("client.vcr.PlaybackControlWidget:shift", a)
{
    Environment env(a, false);
    a.check("handleKey", !env.testee.handleKey(util::KeyMod_Shift + util::Key_Left, 0));
    a.check("handleKey", !env.testee.handleKey(util::KeyMod_Shift + util::Key_Right, 0));
    env.checkFinish();
}

/*
 *  UI integration
 */

AFL_TEST("client.vcr.PlaybackControlWidget:ui", a)
{
    // We cannot test much more than that those functions do not fail.
    Environment env(a, false);

    // - setPlayState
    AFL_CHECK_SUCCEEDS(a("setPlayState"), env.testee.setPlayState(true));

    // - draw (normally scheduled by root)
    gfx::NullCanvas can;
    AFL_CHECK_SUCCEEDS(a("draw"), env.testee.draw(can));

    // - getLayoutInfo
    ui::layout::Info info = env.testee.getLayoutInfo();
    a.checkGreaterThan("prefSize.X", info.getPreferredSize().getX(), 0);
    a.checkGreaterThan("prefSize.Y", info.getPreferredSize().getY(), 0);

    // - handleMouse
    a.check("handleMouse", !env.testee.handleMouse(gfx::Point(0, 0), gfx::EventConsumer::MouseButtons_t()));
}
