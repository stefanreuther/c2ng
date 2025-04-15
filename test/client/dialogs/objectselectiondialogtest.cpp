/**
  *  \file test/client/dialogs/objectselectiondialogtest.cpp
  *  \brief Test for client::dialogs::ObjectSelectionDialog
  */

#include "client/dialogs/objectselectiondialog.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "client/si/commands.hpp"
#include "client/si/control.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/statementcompiler.hpp"
#include "util/requestthread.hpp"

namespace {
    /*
     *  Test harness
     *
     *  20160826: this is the first "UI" test and shows step-by-step what we have to do to set up a minimal UI/backend test.
     *  The ObjectSelectionDialog accesses game data, so we have to supply game infrastructure.
     *  Fortunately, game infrastructure already sets up most of the scripting stuff.
     *  We also have to supply graphics (and thus, eventing and work queue) infrastructure.
     */

    const int PLANET_ID = 42;

    class DialogTester : public afl::base::Deletable {
     public:
        /* Implementation of the test goes here: */
        virtual void doTest(afl::test::Assert a, client::si::UserSide& us, gfx::NullEngine& engine, client::si::Control& parentControl) = 0;

        void run(afl::test::Assert a)
            {
                // Translator
                afl::string::NullTranslator tx;
                afl::sys::Log log;
                afl::io::NullFileSystem fs;
                util::MessageCollector collector;

                // Create a game session containing some data
                game::Session session(tx, fs);
                session.setGame(new game::Game());
                game::map::Planet* pl = session.getGame()->currentTurn().universe().planets().create(PLANET_ID);
                pl->setOwner(1);
                pl->addPlanetSource(game::PlayerSet_t(1));
                pl->setPosition(game::map::Point(2222,3333));
                pl->setName("Planet Express");
                session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
                session.setShipList(new game::spec::ShipList());
                session.postprocessTurn(session.getGame()->currentTurn(), game::PlayerSet_t(1), game::PlayerSet_t(1), game::map::Object::Playable);

                // Create pseudo graphics infrastructure (must live longest!)
                gfx::NullEngine engine;
                gfx::NullResourceProvider provider;
                ui::Root root(engine, provider, gfx::WindowParameters());

                // Session does not work without scripts, so preload it.
                {
                    // Create process
                    interpreter::ProcessList& processList = session.processList();
                    interpreter::Process& proc = processList.create(session.world(), "Initializer");

                    // Create BCO
                    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);

                    // Create script
                    interpreter::MemoryCommandSource mcs;
                    mcs.addLine("Sub C2$Eval(code, UI.Prefix, UI.Key)");
                    mcs.addLine(" Eval AtomStr(code)");
                    mcs.addLine("EndSub");
                    mcs.addLine("CreateKeymap Global, Ship, Planet, Base, Fleet");
                    mcs.addLine("CreateKeymap SelectionDialog(Global)");
                    mcs.addLine("CreateKeymap PlanetSelectionDialog(SelectionDialog)");
                    mcs.addLine("Bind SelectionDialog \"esc\"    := \"UI.EndDialog 0\"");
                    mcs.addLine("Bind SelectionDialog \"enter\"  := \"UI.EndDialog 1\"");

                    // Create compilation context
                    interpreter::DefaultStatementCompilationContext scc(session.world());
                    scc.withStaticContext(&proc);
                    scc.withFlag(scc.LinearExecution);

                    // Compile
                    interpreter::StatementCompiler sc(mcs);
                    interpreter::StatementCompiler::Result result = sc.compileList(*bco, scc);
                    sc.finishBCO(*bco, scc);
                    a.checkEqual("01. compileList succeeded", result, interpreter::StatementCompiler::EndOfInput);

                    // Execute
                    uint32_t pgid = processList.allocateProcessGroup();
                    proc.pushFrame(bco, false);
                    processList.resumeProcess(proc, pgid);
                    processList.startProcessGroup(pgid);
                    processList.run(0);
                    processList.removeTerminatedProcesses();
                }

                // Create session thread
                util::RequestThread sessionThread("TestClientDialogsObjectSelectionDialog::testIt", log, tx);
                util::RequestReceiver<game::Session> sessionReceiver(sessionThread, session);
                session.log().addListener(log);

                // Create a client session. This is required to make UI commands work.
                client::si::UserSide us(root, sessionReceiver.getSender(), tx, root.engine().dispatcher(), collector, log);
                registerCommands(us);

                // Create a parent Control
                class TheControl : public client::si::Control {
                 public:
                    TheControl(afl::test::Assert a, client::si::UserSide& ui)
                        : Control(ui), m_assert(a)
                        { }
                    virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
                        {
                            // We do not expect a state change directed at this control
                            m_assert.checkEqual("handleStateChange", target, client::si::OutputState::NoChange);
                            interface().continueProcess(link);
                        }
                    virtual void handleEndDialog(client::si::RequestLink2 /*link*/, int /*code*/)
                        {
                            // We do not expect an EndDialog directed at this control
                            m_assert.fail("handleEndDialog unexpected");
                        }
                    virtual void handlePopupConsole(client::si::RequestLink2 /*link*/)
                        {
                            // We do not expect a PopupConsole directed at this control
                            m_assert.fail("handlePopupConsole unexpected");
                        }
                    virtual void handleScanKeyboardMode(client::si::RequestLink2 /*link*/)
                        {
                            // We do not expect a ScanKeyboardMode directed at this control
                            m_assert.fail("handleScanKeyboardMode unexpected");
                        }
                    virtual void handleSetView(client::si::RequestLink2 /*link*/, String_t /*name*/, bool /*withKeymap*/)
                        {
                            // We do not expect a Chart.SetView directed at this control
                            m_assert.fail("handleSetView unexpected");
                        }
                    virtual void handleUseKeymap(client::si::RequestLink2 /*link*/, String_t /*name*/, int /*prefix*/)
                        {
                            // We do not expect a UseKeymap directed at this control
                            m_assert.fail("handleUseKeymap unexpected");
                        }
                    virtual void handleOverlayMessage(client::si::RequestLink2 /*link*/, String_t /*text*/)
                        {
                            // We do not expect a UI.OverlayMessage directed at this control
                            m_assert.fail("handleOverlayMessage unexpected");
                        }
                    virtual afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type /*type*/) const
                        {
                            return 0;
                        }
                    virtual game::interface::ContextProvider* createContextProvider()
                        {
                            // We do not provide context
                            return 0;
                        }
                 private:
                    afl::test::Assert m_assert;
                };
                TheControl parentControl(a, us);

                // Do the test
                doTest(a, us, engine, parentControl);
            }
    };

    // A callback that posts a key
    class KeyCallback : public afl::base::Closure<void()> {
     public:
        KeyCallback(gfx::NullEngine& e, util::Key_t key)
            : m_engine(e), m_key(key)
            { }
        virtual KeyCallback* clone() const
            { return new KeyCallback(m_engine, m_key); }
        virtual void call()
            { m_engine.postKey(m_key, 0); }
     private:
        gfx::NullEngine& m_engine;
        util::Key_t m_key;
    };

}

/** Test "OK" button. */
AFL_TEST("client.dialogs.ObjectSelectionDialog:ok", a)
{
    class OKTester : public DialogTester {
     public:
        virtual void doTest(afl::test::Assert a, client::si::UserSide& us, gfx::NullEngine& engine, client::si::Control& parentControl)
            {
                // After opening the dialog, there will be some inter-thread communication to set things up
                // (negotiate keymap, receive data).
                // During this time, the UI will not be responsive (FIXME for later: keys should be queued).
                // We therefore fire a key from a timer.
                client::si::OutputState output;
                afl::base::Ref<gfx::Timer> t = engine.createTimer();
                t->sig_fire.addNewClosure(new KeyCallback(engine, util::Key_Return));
                t->setInterval(100);
                int result = client::dialogs::doObjectSelectionDialog(client::dialogs::PLANET_SELECTION_DIALOG, us, parentControl, output);

                // Verify result: must be ID of our planet.
                a.checkEqual("01", result, PLANET_ID);
            }
    };
    OKTester().run(a);
}

/** Test "Cancel" button. */
AFL_TEST("client.dialogs.ObjectSelectionDialog:cancel", a)
{
    class CancelTester : public DialogTester {
     public:
        virtual void doTest(afl::test::Assert a, client::si::UserSide& us, gfx::NullEngine& engine, client::si::Control& parentControl)
            {
                // Open the dialog
                client::si::OutputState output;
                afl::base::Ref<gfx::Timer> t = engine.createTimer();
                t->sig_fire.addNewClosure(new KeyCallback(engine, util::Key_Escape));
                t->setInterval(100);
                int result = client::dialogs::doObjectSelectionDialog(client::dialogs::PLANET_SELECTION_DIALOG, us, parentControl, output);

                // Verify result: must be 0
                a.checkEqual("01", result, 0);
            }
    };
    CancelTester().run(a);
}
