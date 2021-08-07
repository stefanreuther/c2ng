/**
  *  \file u/t_client_si_control.cpp
  *  \brief Test for client::si::Control
  */

#include "client/si/control.hpp"

#include "t_client_si.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "client/si/userside.hpp"
#include "game/session.hpp"
#include "gfx/defaultfont.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestthread.hpp"

namespace {
    class Tester : public client::si::Control {
     public:
        Tester(client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx)
            : Control(iface, root, tx)
            { }
        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target /*target*/)
            { interface().continueProcessWithFailure(link, "doesn't work"); }
        virtual void handleEndDialog(client::si::RequestLink2 link, int /*code*/)
            { interface().continueProcess(link); }
        virtual void handlePopupConsole(client::si::RequestLink2 link)
            { interface().continueProcess(link); }
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleSetViewRequest(client::si::RequestLink2 link, String_t /*name*/, bool /*withKeymap*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleUseKeymapRequest(client::si::RequestLink2 link, String_t /*name*/, int /*prefix*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleOverlayMessageRequest(client::si::RequestLink2 link, String_t /*text*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual client::si::ContextProvider* createContextProvider()
            { return 0; }
    };
}

/** Multithreaded test. */
void
TestClientSiControl::testMulti()
{
    // Build the environment
    // - UI side
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // - Script side
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    util::MessageCollector collector;
    game::Session session(tx, fs);
    util::RequestThread thread("TestClientSiControl::testIt", log, tx);
    util::RequestReceiver<game::Session> sessionReceiver(thread, session);
    client::si::UserSide iface(sessionReceiver.getSender(), engine.dispatcher(), collector, log);

    // Build a tester and execute a command.
    Tester t(iface, root, tx);
    t.executeCommandWait("Print 'hi'", false, "testMulti");
}

/** Singlethreaded test. */
void
TestClientSiControl::testSingle()
{
    // Build the environment
    // - UI side
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // - Script side
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    util::MessageCollector collector;
    game::Session session(tx, fs);
    util::RequestReceiver<game::Session> sessionReceiver(engine.dispatcher(), session);
    client::si::UserSide iface(sessionReceiver.getSender(), engine.dispatcher(), collector, log);

    // Build a tester and execute a command.
    Tester t(iface, root, tx);
    t.executeCommandWait("Print 'hi'", false, "testSingle");
}
