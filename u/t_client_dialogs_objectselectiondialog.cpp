/**
  *  \file u/t_client_dialogs_objectselectiondialog.cpp
  *  \brief Test for client::dialogs::ObjectSelectionDialog
  */

#include "client/dialogs/objectselectiondialog.hpp"

#include "t_client_dialogs.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "client/session.hpp"
#include "client/si/control.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "gfx/font.hpp"
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
        virtual void doTest(client::Session& session, gfx::NullEngine& engine, client::si::Control& parentControl) = 0;

        void run()
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
                game::config::HostConfiguration config;
                session.getGame()->currentTurn().universe().postprocess(/*playing:*/game::PlayerSet_t(1),
                                                                        /*available:*/game::PlayerSet_t(1),
                                                                        /*plability:*/game::map::Object::Playable,
                                                                        game::HostVersion(),
                                                                        config,
                                                                        1,
                                                                        tx, log);

                // Create pseudo graphics infrastructure (must live longest!)
                gfx::NullEngine engine;
                gfx::NullResourceProvider provider;
                ui::Root root(engine, provider, 400, 300, 32, gfx::Engine::WindowFlags_t());

                // Session does not work without scripts, so preload it.
                {
                    // Create process
                    interpreter::ProcessList& processList = session.world().processList();
                    interpreter::Process& proc = processList.create(session.world(), "Initializer");

                    // Create BCO
                    interpreter::BCORef_t bco = *new interpreter::BytecodeObject();

                    // Create script
                    interpreter::MemoryCommandSource mcs;
                    mcs.addLine("Sub C2$Eval(code, UI.Prefix)");
                    mcs.addLine(" Eval AtomStr(code)");
                    mcs.addLine("EndSub");
                    mcs.addLine("CreateKeymap Global, Ship, Planet, Base, Fleet");
                    mcs.addLine("CreateKeymap SelectionDialog(Global)");
                    mcs.addLine("CreateKeymap PlanetSelectionDialog(SelectionDialog)");
                    mcs.addLine("Bind SelectionDialog \"esc\"    := \"UI.EndDialog 0\"");
                    mcs.addLine("Bind SelectionDialog \"enter\"  := \"UI.EndDialog 1\"");
        
                    // Create compilation context
                    interpreter::DefaultStatementCompilationContext scc(session.world());
                    scc.withContextProvider(&proc);
                    scc.withFlag(scc.LinearExecution);

                    // Compile
                    interpreter::StatementCompiler sc(mcs);
                    interpreter::StatementCompiler::StatementResult result = sc.compileList(*bco, scc);
                    sc.finishBCO(*bco, scc);
                    TS_ASSERT_EQUALS(result, interpreter::StatementCompiler::EndOfInput);

                    // Execute
                    uint32_t pgid = processList.allocateProcessGroup();
                    proc.pushFrame(bco, false);
                    processList.resumeProcess(proc, pgid);
                    processList.startProcessGroup(pgid);
                    processList.run();
                    processList.removeTerminatedProcesses();
                }

                // Create session thread
                util::RequestThread sessionThread("TestClientDialogsObjectSelectionDialog::testIt", log);
                util::RequestReceiver<game::Session> sessionReceiver(sessionThread, session);
                session.log().addListener(log);

                // Create a client session. This is required to make UI commands work.
                client::Session clientSession(root, sessionReceiver.getSender(), tx, collector, log);

                // Create a parent Control
                class TheControl : public client::si::Control {
                 public:
                    TheControl(client::si::UserSide& ui, ui::Root& root, afl::string::Translator& tx)
                        : Control(ui, root, tx)
                        { }
                    virtual void handleStateChange(client::si::UserSide& ui, client::si::RequestLink2 link, client::si::OutputState::Target target)
                        {
                            // We do not expect a state change directed at this control
                            TS_ASSERT(target == client::si::OutputState::NoChange);
                            ui.continueProcess(link);
                        }
                    virtual void handleEndDialog(client::si::UserSide& /*ui*/, client::si::RequestLink2 /*link*/, int /*code*/)
                        {
                            // We do not expect an EndDialog directed at this control
                            TS_ASSERT(0);
                        }
                    virtual void handlePopupConsole(client::si::UserSide& /*ui*/, client::si::RequestLink2 /*link*/)
                        {
                            // We do not expect a PopupConsole directed at this control
                            TS_ASSERT(0);
                        }
                    virtual client::si::ContextProvider* createContextProvider()
                        {
                            // We do not provide context
                            return 0;
                        }
                };
                TheControl parentControl(clientSession.interface(), root, tx);

                // Do the test
                doTest(clientSession, engine, parentControl);
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
void
TestClientDialogsObjectSelectionDialog::testOK()
{
    class OKTester : public DialogTester {
     public:
        virtual void doTest(client::Session& session, gfx::NullEngine& engine, client::si::Control& parentControl)
            {
                // After opening the dialog, there will be some inter-thread communication to set things up
                // (negotiate keymap, receive data).
                // During this time, the UI will not be responsive (FIXME for later: keys should be queued).
                // We therefore fire a key from a timer.
                client::si::OutputState output;
                afl::base::Ref<gfx::Timer> t = engine.createTimer();
                t->sig_fire.addNewClosure(new KeyCallback(engine, util::Key_Return));
                t->setInterval(100);
                int result = client::dialogs::doObjectSelectionDialog(client::dialogs::PLANET_SELECTION_DIALOG, session.interface(), parentControl, output);

                // Verify result: must be ID of our planet.
                TS_ASSERT_EQUALS(result, PLANET_ID);
            }
    };
    OKTester().run();
}

/** Test "Cancel" button. */
void
TestClientDialogsObjectSelectionDialog::testCancel()
{
    class CancelTester : public DialogTester {
     public:
        virtual void doTest(client::Session& session, gfx::NullEngine& engine, client::si::Control& parentControl)
            {
                // Open the dialog
                client::si::OutputState output;
                afl::base::Ref<gfx::Timer> t = engine.createTimer();
                t->sig_fire.addNewClosure(new KeyCallback(engine, util::Key_Escape));
                t->setInterval(100);
                int result = client::dialogs::doObjectSelectionDialog(client::dialogs::PLANET_SELECTION_DIALOG, session.interface(), parentControl, output);

                // Verify result: must be 0
                TS_ASSERT_EQUALS(result, 0);
            }
    };
    CancelTester().run();
}
