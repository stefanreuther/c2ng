/**
  *  \file main/c2ng.cpp
  */

#include <memory>
#include "config.h"

#include "afl/base/closure.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/http/client.hpp"
#include "afl/net/http/defaultconnectionprovider.hpp"
#include "afl/net/http/manager.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/proxytranslator.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/longcommandlineparser.hpp"
#include "afl/sys/thread.hpp"
#include "client/screens/browserscreen.hpp"
#include "client/screens/controlscreen.hpp"
#include "client/screens/playerscreen.hpp"
#include "client/session.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/outputstate.hpp"
#include "client/widgets/busyindicator.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/directoryhandler.hpp"
#include "game/game.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/session.hpp"
#include "game/specificationloader.hpp"
#include "game/turn.hpp"
#include "ui/defaultresourceprovider.hpp"
#include "ui/draw.hpp"
#include "ui/eventloop.hpp"
#include "ui/res/ccimageloader.hpp"
#include "ui/res/directoryprovider.hpp"
#include "ui/res/engineimageloader.hpp"
#include "ui/res/manager.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/root.hpp"
#include "ui/skincolorscheme.hpp"
#include "util/consolelogger.hpp"
#include "util/profiledirectory.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestthread.hpp"
#include "util/rich/parser.hpp"
#include "version.hpp"
#include "game/historyturn.hpp"
#include "game/turnloader.hpp"
#include "game/pcc/browserhandler.hpp"
#ifdef HAVE_SDL
# include "gfx/sdl/engine.hpp"
typedef gfx::sdl::Engine Engine_t;
#else
# error "foo"
#endif

namespace {
    const char LOG_NAME[] = "main";


    void waitForInitialisation(ui::Root& root, util::RequestDispatcher& disp)
    {
        // FIXME: preload images?
        client::widgets::BusyIndicator ind(root, "!Loading...");
        ind.setExtent(gfx::Rectangle(gfx::Point(), ind.getLayoutInfo().getPreferredSize()));
        root.moveWidgetToEdge(ind, 1, 2, 10);
        root.addChild(ind, 0);

        // Event loop
        ui::EventLoop loop(root);
        class Stopper : public util::Request<ui::EventLoop> {
         public:
            virtual void handle(ui::EventLoop& loop)
                { loop.stop(0); }
        };
        util::RequestReceiver<ui::EventLoop> loopReceiver(root.engine().dispatcher(), loop);

        class StopPoster : public afl::base::Runnable {
         public:
            StopPoster(util::RequestSender<ui::EventLoop> reply)
                : m_reply(reply)
                { }
            virtual void run()
                { m_reply.postNewRequest(new Stopper()); }
         private:
            util::RequestSender<ui::EventLoop> m_reply;
        };
        disp.postNewRunnable(new StopPoster(loopReceiver.getSender()));

        // Background thread
        loop.run();
    }

    class ScriptInitializer : public util::Request<game::Session> {
     public:
        ScriptInitializer(afl::base::Ptr<afl::io::Directory> resourceDirectory)
            : m_resourceDirectory(resourceDirectory)
            { }
        virtual void handle(game::Session& t)
            {
                // Load core.q
                afl::base::Ptr<afl::io::Stream> file = m_resourceDirectory->openFile("core.q", afl::io::FileSystem::OpenRead);
                t.executeFile(*file);
            }
     private:
        afl::base::Ptr<afl::io::Directory> m_resourceDirectory;
    };

    class BrowserInitializer : public util::Request<game::browser::Session> {
     public:
        BrowserInitializer(afl::io::FileSystem& fileSystem,
                           afl::base::Ptr<afl::io::Directory> defaultSpecDirectory,
                           util::ProfileDirectory& profile,
                           afl::net::http::Manager& httpManager)
            : m_fileSystem(fileSystem),
              m_defaultSpecDirectory(defaultSpecDirectory),
              m_profile(profile),
              m_httpManager(httpManager)
            { }

        virtual void handle(game::browser::Session& t)
            {
                std::auto_ptr<game::browser::Browser>& b = t.browser();
                t.accountManager().reset(new game::browser::AccountManager(m_profile, t.translator(), t.log()));
                t.accountManager()->load();
                b.reset(new game::browser::Browser(m_fileSystem, t.translator(), t.log(), *t.accountManager(), t.userCallbackProxy()));
                b->handlers().addNewHandler(new game::browser::DirectoryHandler(*b, m_defaultSpecDirectory, m_profile, m_fileSystem));
                b->handlers().addNewHandler(new game::pcc::BrowserHandler(*b, m_httpManager, m_defaultSpecDirectory, m_profile));
                b->handlers().addNewHandler(new game::nu::BrowserHandler(*b, m_httpManager, m_defaultSpecDirectory));
            }

     private:
        afl::io::FileSystem& m_fileSystem;
        afl::base::Ptr<afl::io::Directory> m_defaultSpecDirectory;
        util::ProfileDirectory& m_profile;
        afl::net::http::Manager& m_httpManager;
    };

    class BrowserPositioner : public util::Request<game::browser::Session> {
     public:
        BrowserPositioner(String_t path)
            : m_path(path)
            { }
        void handle(game::browser::Session& session)
            {
                if (game::browser::Browser* b = session.browser().get()) {
                    b->openFolder(m_path);
                    b->loadContent();
                    b->openParent();
                }
            }
     private:
        String_t m_path;
    };

    class BrowserListener : public afl::base::Closure<void(int)> {
     public:
        BrowserListener(client::screens::BrowserScreen& screen,
                        util::RequestSender<game::browser::Session> browserSender,
                        util::RequestSender<game::Session> gameSender)
            : m_screen(screen),
              m_uiSender(screen.getSender()),
              m_browserSender(browserSender),
              m_gameSender(gameSender)
            { }

        void call(int player)
            {
                m_screen.setBlockState(true);
                m_browserSender.postNewRequest(new LoadRequest(player, m_uiSender, m_gameSender));
            }

        BrowserListener* clone() const
            { return new BrowserListener(*this); }

     private:
        class LoadRequest : public util::Request<game::browser::Session> {
         public:
            LoadRequest(int player,
                        util::RequestSender<client::screens::BrowserScreen> uiSender,
                        util::RequestSender<game::Session> gameSender)
                : m_player(player),
                  m_uiSender(uiSender),
                  m_gameSender(gameSender)
                { }

            void handle(game::browser::Session& session)
                {
                    if (game::browser::Browser* p = session.browser().get()) {
                        p->loadChildRoot();
                        m_gameSender.postNewRequest(new LoadRequest2(m_player, p->getSelectedRoot(), m_uiSender));
                    }
                }

         private:
            int m_player;
            util::RequestSender<client::screens::BrowserScreen> m_uiSender;
            util::RequestSender<game::Session> m_gameSender;
        };

        class LoadRequest2 : public util::Request<game::Session> {
         public:
            LoadRequest2(int player, afl::base::Ptr<game::Root> root, util::RequestSender<client::screens::BrowserScreen> uiSender)
                : m_player(player),
                  m_root(root),
                  m_uiSender(uiSender)
                { }
            void handle(game::Session& session)
                {
                    bool ok;
                    if (m_root.get() != 0) {
                        // Get turn loader
                        afl::base::Ptr<game::TurnLoader> loader = m_root->getTurnLoader();
                        if (loader.get() != 0) {
                            // Everything fine: make a new session
                            try {
                                session.setGame(new game::Game());
                                session.setRoot(m_root);
                                session.setShipList(new game::spec::ShipList());
                                m_root->specificationLoader().loadShipList(*session.getShipList(), *m_root);

                                m_root->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), m_player, *m_root);
                                session.getGame()->setViewpointPlayer(m_player);
                                session.getGame()->currentTurn().universe().postprocess(game::PlayerSet_t(m_player), game::PlayerSet_t(m_player), game::map::Object::Playable,
                                                                                        m_root->hostVersion(), m_root->hostConfiguration(),
                                                                                        session.translator(), session.log());
                                ok = true;
                            }
                            catch (std::exception& e) {
                                session.log().write(afl::sys::LogListener::Error, LOG_NAME, session.translator().translateString("Unable to load turn"), e);
                                session.setGame(0);
                                session.setRoot(0);
                                session.setShipList(0);
                                ok = false;
                            }
                        } else {
                            // Don't have a turn loader
                            ok = false;
                        }
                    } else {
                        ok = false;
                    }
                    m_uiSender.postNewRequest(new ConfirmRequest(ok));
                }

         private:
            const int m_player;
            const afl::base::Ptr<game::Root> m_root;
            util::RequestSender<client::screens::BrowserScreen> m_uiSender;
        };
        class ConfirmRequest : public util::Request<client::screens::BrowserScreen> {
         public:
            ConfirmRequest(bool ok)
                : m_ok(ok)
                { }
            void handle(client::screens::BrowserScreen& screen)
                {
                    screen.setBlockState(false);
                    if (m_ok) {
                        screen.stop(1);
                    }
                }
         private:
            const bool m_ok;
        };

        client::screens::BrowserScreen& m_screen;
        util::RequestSender<client::screens::BrowserScreen> m_uiSender;
        util::RequestSender<game::browser::Session> m_browserSender;
        util::RequestSender<game::Session> m_gameSender;
    };


    class CommandLineParameters {
     public:
        CommandLineParameters()
            : m_haveGameDirectory(false),
              m_gameDirectory()
            { }

        void parse(afl::base::Ptr<afl::sys::Environment::CommandLine_t> cmdl)
            {
                afl::sys::LongCommandLineParser parser(cmdl);
                bool option;
                String_t text;
                while (parser.getNext(option, text)) {
                    if (option) {
                        // FIXME
                    } else {
                        if (!m_haveGameDirectory) {
                            m_haveGameDirectory = true;
                            m_gameDirectory = text;
                        } else {
                            // FIXME
                        }
                    }
                }
            }

        bool getGameDirectory(String_t& dir)
            {
                if (m_haveGameDirectory) {
                    dir = m_gameDirectory;
                    return true;
                } else {
                    return false;
                }
            }

     private:
        bool m_haveGameDirectory;
        String_t m_gameDirectory;
    };


    void play(client::Session& session)
    {
        using client::si::OutputState;
        using client::si::InputState;
        OutputState::Target state = OutputState::PlayerScreen;
        InputState in;
        bool running = true;
        while (running) {
            OutputState out;
            switch (state) {
             case OutputState::NoChange:
             case OutputState::ExitProgram:
             case OutputState::ExitGame:
                // FIXME: at this point, we may have a process in InputState. That one must be terminated.
                // FIXME: save the game of course...
                running = false;
                break;

             case OutputState::PlayerScreen:
                client::screens::doPlayerScreen(session, in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::ShipScreen:
                client::screens::ControlScreen(session, game::map::Cursors::ShipScreen, OutputState::ShipScreen).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::PlanetScreen:
                client::screens::ControlScreen(session, game::map::Cursors::PlanetScreen, OutputState::PlanetScreen).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::BaseScreen:
                client::screens::ControlScreen(session, game::map::Cursors::BaseScreen, OutputState::BaseScreen).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;
            }
        }
    }
}

int main(int, char** argv)
{
    // Capture environment
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();

    // Infrastructure (FIXME).
    afl::string::NullTranslator tx;
    util::ConsoleLogger log;
    log.attachWriter(true, env.attachTextWriterNT(env.Error));
    log.attachWriter(false, env.attachTextWriterNT(env.Output));
    util::ProfileDirectory profile(env, fs, tx, log);
    afl::string::Translator::setSystemInstance(std::auto_ptr<afl::string::Translator>(new afl::string::ProxyTranslator(tx)));

    // At this point we are safely operable.
    try {
        // Parse command line.
        CommandLineParameters params;
        params.parse(env.getCommandLine());
        log.write(log.Info, LOG_NAME, "[PCC2 v" PCC2_VERSION "]");

        // Derived environment
        afl::base::Ptr<afl::io::Directory> resourceDirectory    = fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "resource"));
        afl::base::Ptr<afl::io::Directory> defaultSpecDirectory = fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs"));

        // Set up GUI
        log.write(log.Debug, LOG_NAME, tx.translateString("Starting GUI..."));
        Engine_t engine(log);
        ui::res::Manager mgr;
        mgr.addNewImageLoader(new ui::res::EngineImageLoader(engine));
        mgr.addNewImageLoader(new ui::res::CCImageLoader());
        mgr.addNewProvider(new ui::res::DirectoryProvider(resourceDirectory));

        ui::DefaultResourceProvider provider(mgr, resourceDirectory, engine.dispatcher(), tx, log);
        ui::Root root(engine, provider, 800, 600, 32, Engine_t::WindowFlags_t());
        mgr.setScreenSize(root.getExtent().getSize());

        // Set up HTTP
        // FIXME: do this here? We would have to do this elsewhere if it takes time; like, for loading config files.
        log.write(log.Debug, LOG_NAME, tx.translateString("Starting network..."));
        afl::net::http::Client client;
        afl::sys::Thread clientThread("http", client);
        client.setNewConnectionProvider(new afl::net::http::DefaultConnectionProvider(client, afl::net::NetworkStack::getInstance()));
        clientThread.start();
        afl::net::http::Manager httpManager(client);

        // At this point, the GUI is up and running.
        // This thread may now do nothing else than GUI.
        // All I/O accesses must from now on go through a background thread.
        // Set up session objects. None of these constructors block (I hope).
        log.write(log.Debug, LOG_NAME, tx.translateString("Starting background thread..."));
        game::browser::Session browserSession(tx, log);
        game::Session gameSession(tx, fs);
        gameSession.log().addListener(log);

        // Set up background thread and request receivers.
        // These must be after the session objects so that they die before them, allowing final requests to finish.
        util::RequestThread backgroundThread("game.background", log);
        util::RequestReceiver<game::browser::Session> browserReceiver(backgroundThread, browserSession);
        util::RequestReceiver<game::Session> gameReceiver(backgroundThread, gameSession);

        // Set up foreground thread.
        client::Session clientSession(root, gameReceiver.getSender(), tx);

        // Initialize by posting requests to the background thread.
        // (This will not take time.)
        gameReceiver.getSender().postNewRequest(new ScriptInitializer(resourceDirectory));
        browserReceiver.getSender().postNewRequest(new BrowserInitializer(fs, defaultSpecDirectory, profile, httpManager));
        {
            String_t initialGameDirectory;
            if (params.getGameDirectory(initialGameDirectory)) {
                browserReceiver.getSender().postNewRequest(new BrowserPositioner(initialGameDirectory));
            }
        }

        // Wait for completion of initialisation
        waitForInitialisation(root, backgroundThread);
        log.write(log.Debug, LOG_NAME, tx.translateString("Initialisation complete"));

        // Start game browser
        // FIXME: wrap this in a try/catch
        while (1) {
            // Helpful information
            ui::rich::DocumentView docView(root.getExtent().getSize(), 0, root.provider());
            ui::SkinColorScheme docColors(ui::BLACK_COLOR_SET, root.colorScheme());
            docView.setExtent(gfx::Rectangle(gfx::Point(0, 0), docView.getLayoutInfo().getPreferredSize()));
            docView.getDocument().add(util::rich::Parser::parseXml("<big>PCC2ng Milestone Zero</big>"));
            docView.getDocument().addNewline();
            docView.getDocument().addNewline();
            docView.getDocument().add(util::rich::Parser::parseXml("<font color=\"dim\">&#xA9; 2016 Stefan Reuther &lt;streu@gmx.de&gt;</font>"));
            docView.getDocument().addNewline();
            docView.getDocument().finish();
            docView.handleDocumentUpdate();
            docView.adjustToDocumentSize();
            docView.setExtent(gfx::Rectangle(gfx::Point(0, 0), docView.getLayoutInfo().getPreferredSize()));
            docView.setColorScheme(docColors);
            root.add(docView);

            // Browser
            client::screens::BrowserScreen browserScreen(root, browserReceiver.getSender());
            browserScreen.sig_gameSelection.addNewClosure(new BrowserListener(browserScreen, browserReceiver.getSender(), gameReceiver.getSender()));
            int result = browserScreen.run();
            if (result != 0) {
                // OK, play
                play(clientSession);
            } else {
                // Close
                break;
            }
        }

        // Stop
        client.stop();
        clientThread.join();
    }
    catch (std::exception& e) {
        log.write(log.Error, LOG_NAME, tx.translateString("Uncaught exception"), e);
        log.write(log.Error, LOG_NAME, tx.translateString("Program exits abnormally (crash)"));
        return 1;
    }
    catch (...) {
        log.write(log.Error, LOG_NAME, tx.translateString("Uncaught exception"));
        log.write(log.Error, LOG_NAME, tx.translateString("Program exits abnormally (crash)"));
        return 1;
    }
}
