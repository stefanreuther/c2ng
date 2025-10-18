/**
  *  \file client/application.cpp
  *  \brief Class client::Application
  */
#include <memory>
#include <ctime>
#include <stdlib.h>

#include "client/application.hpp"

#include "afl/base/closure.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/http/client.hpp"
#include "afl/net/http/clientconnection.hpp"
#include "afl/net/http/clientconnectionprovider.hpp"
#include "afl/net/http/manager.hpp"
#include "afl/net/securenetworkstack.hpp"
#include "afl/net/tunnel/tunnelablenetworkstack.hpp"
#include "afl/string/format.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/mutexguard.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/translator.hpp"
#include "client/applicationparameters.hpp"
#include "client/dialogs/simulator.hpp"
#include "client/map/screen.hpp"
#include "client/screens/browserscreen.hpp"
#include "client/screens/controlscreen.hpp"
#include "client/screens/playerscreen.hpp"
#include "client/si/commands.hpp"
#include "client/si/control.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/nullcontrol.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "client/tiles/historyadaptor.hpp"
#include "client/usercallback.hpp"
#include "game/authcache.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/directoryhandler.hpp"
#include "game/interface/labelextra.hpp"
#include "game/interface/plugins.hpp"
#include "game/interface/privatefunctions.hpp"
#include "game/interface/taskwaypoints.hpp"
#include "game/interface/vmfile.hpp"
#include "game/map/cursors.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/pcc/browserhandler.hpp"
#include "game/proxy/browserproxy.hpp"
#include "game/session.hpp"
#include "gfx/application.hpp"
#include "gfx/complex.hpp"
#include "gfx/gen/orbitconfig.hpp"
#include "gfx/gen/spaceviewconfig.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"
#include "ui/defaultresourceprovider.hpp"
#include "ui/draw.hpp"
#include "ui/pixmapcolorscheme.hpp"
#include "ui/res/ccimageloader.hpp"
#include "ui/res/directoryprovider.hpp"
#include "ui/res/engineimageloader.hpp"
#include "ui/res/generatedengineprovider.hpp"
#include "ui/res/generatedplanetprovider.hpp"
#include "ui/res/manager.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/root.hpp"
#include "ui/screenshotlistener.hpp"
#include "util/consolelogger.hpp"
#include "util/messagecollector.hpp"
#include "util/profiledirectory.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestthread.hpp"
#include "util/rich/parser.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"
#include "version.hpp"

using game::interface::PrivateFunctions;
using interpreter::BytecodeObject;
using interpreter::Process;
using util::RequestSender;

namespace {
    const char LOG_NAME[] = "main";

    const char PROGRAM_TITLE[] = "PCC2 v" PCC2_VERSION;

    enum ConfirmAction {
        ActionCanceled,
        ActionPlay,
        ActionSimulator
    };

    class ScriptInitializer : public client::si::ScriptTask {
     public:
        ScriptInitializer(afl::base::Ref<afl::io::Directory> resourceDirectory, util::ProfileDirectory& profile)
            : m_resourceDirectory(resourceDirectory),
              m_profile(profile)
            { }
        virtual void execute(uint32_t pgid, game::Session& t)
            {
                // Configure load directory
                t.world().setSystemLoadDirectory(m_resourceDirectory.asPtr());

                // Get process list
                interpreter::ProcessList& processList = t.processList();

                // Create process to load core.q
                Process& coreProcess = processList.create(t.world(), "<Core>");
                coreProcess.pushFrame(game::interface::createFileLoader("core.q", "core.q", false), false);
                processList.resumeProcess(coreProcess, pgid);

                // Create process to load plugins
                Process& pluginProcess = processList.create(t.world(), "<PluginLoader>");
                pluginProcess.pushFrame(game::interface::createLoaderForUnloadedPlugins(t.plugins()), false);
                processList.resumeProcess(pluginProcess, pgid);

                // Create process to load pcc2init.q
                try {
                    Process& initProcess = processList.create(t.world(), "<Init>");
                    initProcess.pushFrame(game::interface::createFileLoader(t.world().fileSystem().makePathName(m_profile.open()->getDirectoryName(), "pcc2init.q"),
                                                                            "pcc2init.q", true), false);
                    processList.resumeProcess(initProcess, pgid);
                }
                catch (std::exception& e) {
                    t.log().write(afl::sys::LogListener::Warn, LOG_NAME, t.translator()("Unable to open profile directory"), e);
                }
            }
     private:
        afl::base::Ref<afl::io::Directory> m_resourceDirectory;
        util::ProfileDirectory& m_profile;
    };

    class PluginInitializer : public util::Request<game::Session> {
     public:
        PluginInitializer(afl::base::Ref<afl::io::Directory> resDir, util::ProfileDirectory& dir, const std::vector<String_t>& commandLineResources)
            : m_resourceDirectory(resDir),
              m_profile(dir),
              m_commandLineResources(commandLineResources)
            { }
        virtual void handle(game::Session& session)
            {
                // Note that plugin names must be specified in upper-case here.
                // The plugins are loaded through the script interface, which upper-cases the names before looking them up.
                try {
                    // Global cc-res.cfg
                    afl::base::Ref<afl::io::Stream> configFile = m_resourceDirectory->openFile("cc-res.cfg", afl::io::FileSystem::OpenRead);
                    std::auto_ptr<util::plugin::Plugin> plug(new util::plugin::Plugin("(GLOBAL CC-RES.CFG)"));
                    plug->initFromConfigFile(m_profile.open()->getDirectoryName(), session.translator()("Global cc-res.cfg"), *configFile, session.translator());
                    session.plugins().addNewPlugin(plug.release());
                }
                catch (...) { }

                try {
                    // User cc-res.cfg
                    afl::base::Ptr<afl::io::Stream> configFile = m_profile.openFileNT("cc-res.cfg");
                    if (configFile.get() != 0) {
                        std::auto_ptr<util::plugin::Plugin> plug(new util::plugin::Plugin("(USER CC-RES.CFG)"));
                        plug->initFromConfigFile(m_profile.open()->getDirectoryName(), session.translator()("User cc-res.cfg"), *configFile, session.translator());
                        session.plugins().addNewPlugin(plug.release());
                    }
                }
                catch (...) { }

                try {
                    // Plugins
                    afl::base::Ref<afl::io::DirectoryEntry> e = m_profile.open()->getDirectoryEntryByName("plugins");
                    session.setPluginDirectoryName(e->getPathName());
                    session.plugins().findPlugins(*e->openDirectory());
                }
                catch (...) { }

                if (!m_commandLineResources.empty()) {
                    // Command line
                    std::auto_ptr<util::plugin::Plugin> plug(new util::plugin::Plugin("(COMMAND LINE)"));
                    for (size_t i = 0, n = m_commandLineResources.size(); i < n; ++i) {
                        plug->addItem(util::plugin::Plugin::ResourceFile, m_commandLineResources[i]);
                    }
                    session.plugins().addNewPlugin(plug.release());
                }
            }
     private:
        afl::base::Ref<afl::io::Directory> m_resourceDirectory;
        util::ProfileDirectory& m_profile;
        const std::vector<String_t>& m_commandLineResources;
    };

    class BrowserInitializer : public afl::base::Closure<game::browser::Session*(game::Session&)> {
     public:
        BrowserInitializer(afl::base::Ref<afl::io::Directory> defaultSpecDirectory,
                           util::ProfileDirectory& profile,
                           afl::net::http::Manager& httpManager)
            : m_defaultSpecDirectory(defaultSpecDirectory),
              m_profile(profile),
              m_httpManager(httpManager)
            { }

        virtual game::browser::Session* call(game::Session& session)
            {
                std::auto_ptr<game::browser::Session> t(new game::browser::Session(session.world().fileSystem(), session.translator(), session.log(), m_profile));

                game::browser::Browser& b = t->browser();
                t->accountManager().load();
                b.addNewHandler(new game::browser::DirectoryHandler(b, m_defaultSpecDirectory, m_profile));
                b.addNewHandler(new game::pcc::BrowserHandler(b, m_httpManager, m_defaultSpecDirectory, m_profile));
                b.addNewHandler(new game::nu::BrowserHandler(b, m_httpManager, m_defaultSpecDirectory));

                return t.release();
            }

     private:
        afl::base::Ref<afl::io::Directory> m_defaultSpecDirectory;
        util::ProfileDirectory& m_profile;
        afl::net::http::Manager& m_httpManager;
    };

    /*
     *  The BrowserListener is invoked by BrowserScreen when the user wants to open a game or simulator.
     */
    class BrowserListener : public client::screens::BrowserScreen::Callback {
     public:
        BrowserListener(client::si::UserSide& us,
                        RequestSender<game::browser::Session> browserSender,
                        RequestSender<game::Session> gameSender)
            : m_pScreen(0),
              m_userSide(us),
              m_browserSender(browserSender),
              m_gameSender(gameSender)
            { }

        void setScreen(client::screens::BrowserScreen* pScreen)
            {
                m_pScreen = pScreen;
            }

        void onOpenGame(int player)
            {
                if (m_pScreen != 0) {
                    client::si::NullControl(m_userSide).executeHookWait("BeforeLoad");
                    m_pScreen->setBlockState(true);
                    m_gameSender.postNewRequest(new LoadRequest(player, m_pScreen->getSender(), m_gameSender, m_browserSender));
                }
            }

        void onSimulate()
            {
                if (m_pScreen != 0) {
                    m_pScreen->setBlockState(true);
                    m_gameSender.postNewRequest(new SimRequest(m_pScreen->getSender(), m_gameSender, m_browserSender));
                }
            }

     private:
        class Finalizer : public Process::Finalizer {
         public:
            Finalizer(RequestSender<client::screens::BrowserScreen> uiSender, ConfirmAction successAction)
                : m_uiSender(uiSender),
                  m_successAction(successAction)
                { }
            ~Finalizer()
                { /* TODO: unexpected stop */ }
            virtual void finalizeProcess(Process& p)
                {
                    if (p.getState() == Process::Ended) {
                        m_uiSender.postNewRequest(new ConfirmRequest(m_successAction));
                    } else {
                        m_uiSender.postNewRequest(new ConfirmRequest(ActionCanceled));
                    }
                }
         private:
            RequestSender<client::screens::BrowserScreen> m_uiSender;
            ConfirmAction m_successAction;
        };

        // Request to load a game: prepare everything
        class LoadRequest : public util::Request<game::Session> {
         public:
            LoadRequest(int player,
                        RequestSender<client::screens::BrowserScreen> uiSender,
                        RequestSender<game::Session> gameSender,
                        RequestSender<game::browser::Session> browserSender)
                : m_player(player), m_uiSender(uiSender), m_gameSender(gameSender), m_browserSender(browserSender)
                { }
            void handle(game::Session& session)
                {
                    Process& proc = session.processList().create(session.world(), "<Loader>");
                    interpreter::BCORef_t bco = BytecodeObject::create(true);
                    PrivateFunctions::addTakeRoot(session, *bco, m_gameSender, m_browserSender);
                    PrivateFunctions::addMakeGame(session, *bco);
                    PrivateFunctions::addMakeShipList(session, *bco);
                    PrivateFunctions::addLoadShipList(session, *bco);
                    PrivateFunctions::addLoadCurrentTurn(session, *bco, m_player);
                    PrivateFunctions::addPostprocessCurrentTurn(session, *bco, m_player);
                    proc.pushFrame(bco, false);
                    proc.setNewFinalizer(new Finalizer(m_uiSender, ActionPlay));

                    uint32_t pgid = session.processList().allocateProcessGroup();
                    session.processList().resumeProcess(proc, pgid);
                    session.processList().startProcessGroup(pgid);
                    session.runScripts();
                }

         private:
            const int m_player;
            RequestSender<client::screens::BrowserScreen> m_uiSender;
            RequestSender<game::Session> m_gameSender;
            RequestSender<game::browser::Session> m_browserSender;
        };

        // Request for simulation: prepare ship list
        class SimRequest : public util::Request<game::Session> {
         public:
            SimRequest(RequestSender<client::screens::BrowserScreen> uiSender,
                       RequestSender<game::Session> gameSender,
                       RequestSender<game::browser::Session> browserSender)
                : m_uiSender(uiSender), m_gameSender(gameSender), m_browserSender(browserSender)
                { }
            void handle(game::Session& session)
                {
                    Process& proc = session.processList().create(session.world(), "<Loader>");
                    interpreter::BCORef_t bco = BytecodeObject::create(true);
                    PrivateFunctions::addTakeRoot(session, *bco, m_gameSender, m_browserSender);
                    PrivateFunctions::addMakeShipList(session, *bco);
                    PrivateFunctions::addLoadShipList(session, *bco);
                    proc.pushFrame(bco, false);
                    proc.setNewFinalizer(new Finalizer(m_uiSender, ActionSimulator));

                    uint32_t pgid = session.processList().allocateProcessGroup();
                    session.processList().resumeProcess(proc, pgid);
                    session.processList().startProcessGroup(pgid);
                    session.runScripts();
                }

         private:
            RequestSender<client::screens::BrowserScreen> m_uiSender;
            RequestSender<game::Session> m_gameSender;
            RequestSender<game::browser::Session> m_browserSender;
        };

        class ConfirmRequest : public util::Request<client::screens::BrowserScreen> {
         public:
            ConfirmRequest(ConfirmAction action)
                : m_action(action)
                { }
            void handle(client::screens::BrowserScreen& screen)
                {
                    screen.setBlockState(false);
                    if (m_action != ActionCanceled) {
                        screen.stop(m_action);
                    }
                }
         private:
            const ConfirmAction m_action;
        };

        client::screens::BrowserScreen* m_pScreen;
        client::si::UserSide& m_userSide;
        RequestSender<game::browser::Session> m_browserSender;
        RequestSender<game::Session> m_gameSender;
    };

    class ConnectionProvider : public afl::net::http::ClientConnectionProvider,
                               private afl::base::Stoppable,
                               private afl::base::Uncopyable
    {
     public:
        ConnectionProvider(afl::net::http::Client& client, afl::net::NetworkStack& stack)
            : m_client(client),
              m_networkStack(stack),
              m_secureNetworkStack(),
              m_wake(0),
              m_mutex(),
              m_stop(false),
              m_thread("ConnectionProvider", *this)
            { m_thread.start(); }
        ~ConnectionProvider()
            { }
        void requestNewConnection()
            { m_wake.post(); }
     private:
        // Thread:
        virtual void run()
            {
                try {
                    m_secureNetworkStack.reset(new afl::net::SecureNetworkStack(m_networkStack));
                }
                catch (std::exception& e) {
                    // FIXME: log it
                }
                while (1) {
                    // Wait for something to happen
                    m_wake.wait();

                    // Stop requested?
                    {
                        afl::sys::MutexGuard g(m_mutex);
                        if (m_stop) {
                            break;
                        }
                    }

                    // Create requested connections
                    afl::net::Name name;
                    String_t scheme;
                    while (m_client.getUnsatisfiedTarget(name, scheme)) {
                        if (scheme == "http") {
                            tryConnect(m_networkStack, name, scheme);
                        } else if (scheme == "https" && m_secureNetworkStack.get() != 0) {
                            tryConnect(*m_secureNetworkStack, name, scheme);
                        } else {
                            // Mismatching scheme, request cannot be fulfilled
                            m_client.cancelRequestsByTarget(name, scheme,
                                                            afl::net::http::ClientRequest::UnsupportedProtocol,
                                                            afl::string::Messages::invalidUrl());
                        }
                    }
                }
            }
        virtual void stop()
            {
                {
                    afl::sys::MutexGuard g(m_mutex);
                    m_stop = true;
                }
                m_wake.post();
            }

        void tryConnect(afl::net::NetworkStack& stack, const afl::net::Name& name, const String_t& scheme)
            {
                const uint32_t CONNECTION_TIMEOUT = 30000;
                try {
                    // Try connecting...
                    afl::base::Ref<afl::net::Socket> socket = stack.connect(name, CONNECTION_TIMEOUT);
                    m_client.addNewConnection(new afl::net::http::ClientConnection(name, scheme, socket));
                }
                catch (std::exception& e) {
                    // Regular failure case
                    m_client.cancelRequestsByTarget(name, scheme,
                                                    afl::net::http::ClientRequest::ConnectionFailed,
                                                    e.what());
                }
                catch (...) {
                    // Irregular failure case; avoid that exceptions kill the thread
                    m_client.cancelRequestsByTarget(name, scheme,
                                                    afl::net::http::ClientRequest::ConnectionFailed,
                                                    afl::string::Messages::unknownError());
                }
            }

        // Integration:
        afl::net::http::Client& m_client;
        afl::net::NetworkStack& m_networkStack;
        std::auto_ptr<afl::net::SecureNetworkStack> m_secureNetworkStack;

        // Work:
        afl::sys::Semaphore m_wake;
        afl::sys::Mutex m_mutex;
        bool m_stop;

        // Thread: must be last
        afl::sys::Thread m_thread;
    };


    /*
     *  Browser screen initalisation actions
     */

    typedef afl::base::Closure<void(client::screens::BrowserScreen&)> BrowserAction_t;

    class AutoLoadAction : public BrowserAction_t {
     public:
        AutoLoadAction(int playerNumber)
            : m_playerNumber(playerNumber)
            { }
        virtual void call(client::screens::BrowserScreen& screen)
            { screen.setAutoLoad(m_playerNumber); }
     private:
        int m_playerNumber;
    };

    class AutoFocusAction : public BrowserAction_t {
     public:
        AutoFocusAction(int playerNumber)
            : m_playerNumber(playerNumber)
            { }
        virtual void call(client::screens::BrowserScreen& screen)
            { screen.setAutoFocus(m_playerNumber); }
     private:
        int m_playerNumber;
    };


    afl::base::Ref<gfx::Canvas> generateGameBackground(afl::sys::LogListener& log, gfx::Point size)
    {
        uint32_t ticks = afl::sys::Time::getTickCounter();
        util::RandomNumberGenerator rng(ticks);
        gfx::gen::OrbitConfig config;
        config.setSize(size);
        afl::base::Ref<gfx::Canvas> result = config.render(rng)->makeCanvas();
        log.write(log.Trace, LOG_NAME, afl::string::Format("Rendered game background in %d ms", afl::sys::Time::getTickCounter() - ticks));
        return result;
    }

    afl::base::Ref<gfx::Canvas> generateBrowserBackground(afl::sys::LogListener& log, gfx::Point size)
    {
        uint32_t ticks = afl::sys::Time::getTickCounter();
        util::RandomNumberGenerator rng(ticks);
        gfx::gen::SpaceViewConfig cfg;
        cfg.setSize(size);
        cfg.setNumSuns(0);
        afl::base::Ref<gfx::Canvas> result = cfg.render(rng)->makeCanvas();
        log.write(log.Trace, LOG_NAME, afl::string::Format("Rendered browser background in %d ms", afl::sys::Time::getTickCounter() - ticks));
        return result;
    }

    void play(client::si::UserSide& us)
    {
        using client::si::OutputState;
        using client::si::InputState;
        ui::PixmapColorScheme colorScheme(us.root(), generateGameBackground(us.mainLog(), us.root().getExtent().getSize()));
        OutputState::Target state = OutputState::PlayerScreen;
        InputState in;
        bool running = true;
        bool first = true;
        while (running) {
            OutputState out;
            switch (state) {
             case OutputState::NoChange:
             case OutputState::ExitProgram:
             case OutputState::ExitGame:
                // FIXME: at this point, we may have a process in InputState. That one must be terminated.
                running = false;
                break;

             case OutputState::PlayerScreen:
                client::screens::doPlayerScreen(us, in, out, colorScheme, first);
                first = false;
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::ShipScreen:
                client::screens::ControlScreen(us, game::map::Cursors::ShipScreen, client::screens::ControlScreen::ShipScreen).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::PlanetScreen:
                client::screens::ControlScreen(us, game::map::Cursors::PlanetScreen, client::screens::ControlScreen::PlanetScreen).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::BaseScreen:
                client::screens::ControlScreen(us, game::map::Cursors::BaseScreen, client::screens::ControlScreen::BaseScreen).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::HistoryScreen:
                client::screens::ControlScreen(us, game::map::Cursors::HistoryScreen, client::screens::ControlScreen::HistoryScreen)
                    .withHistoryAdaptor()
                    .run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::FleetScreen:
                client::screens::ControlScreen(us, game::map::Cursors::FleetScreen, client::screens::ControlScreen::FleetScreen)
                    .withFleetProxy()
                    .run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::ShipTaskScreen:
                client::screens::ControlScreen(us, game::map::Cursors::ShipScreen, client::screens::ControlScreen::ShipTaskScreen)
                    .withTaskEditor(Process::pkShipTask)
                    .run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::PlanetTaskScreen:
                client::screens::ControlScreen(us, game::map::Cursors::PlanetScreen, client::screens::ControlScreen::PlanetTaskScreen)
                    .withTaskEditor(Process::pkPlanetTask)
                    .run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::BaseTaskScreen:
                client::screens::ControlScreen(us, game::map::Cursors::BaseScreen, client::screens::ControlScreen::BaseTaskScreen)
                    .withTaskEditor(Process::pkBaseTask)
                    .run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::Starchart:
                client::map::Screen(us,
                                    us.root(),
                                    us.translator(),
                                    us.gameSender()).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;
            }
        }
    }
}

client::Application::Application(afl::sys::Dialog& dialog,
                                 afl::string::Translator& tx,
                                 afl::sys::Environment& env,
                                 afl::io::FileSystem& fs,
                                 afl::net::NetworkStack& net)
    : gfx::Application(dialog, tx, PROGRAM_TITLE),
      m_environment(env),
      m_fileSystem(fs),
      m_networkStack(net)
{ }

void
client::Application::appMain(gfx::Engine& engine)
{
    // Capture environment
    afl::io::FileSystem& fs = m_fileSystem;

    // Infrastructure
    util::ConsoleLogger console;
    console.attachWriter(true, m_environment.attachTextWriterNT(m_environment.Error));
    console.attachWriter(false, m_environment.attachTextWriterNT(m_environment.Output));
    log().addListener(console);
    util::ProfileDirectory profile(m_environment, m_fileSystem);

    // At this point we are safely operable.
    // Start collecting messages.
    // Starting from here, log messages will be retrievable
    util::MessageCollector collector;
    log().addListener(collector);
    console.setConfiguration("*@Trace=hide", translator());
    collector.setConfiguration("*@Trace=hide", translator());

    // Parse command line.
    ApplicationParameters params(*this, PROGRAM_TITLE);
    params.parse(m_environment.getCommandLine());
    if (!params.getTraceConfiguration().empty()) {
        console.setConfiguration(params.getTraceConfiguration(), translator());
        collector.setConfiguration(params.getTraceConfiguration(), translator());
    }
    log().write(log().Info, LOG_NAME, afl::string::Format("[%s]", PROGRAM_TITLE));

    // Derived environment
    afl::base::Ref<afl::io::Directory> resourceDirectory    = fs.openDirectory(fs.makePathName(fs.makePathName(m_environment.getInstallationDirectoryName(), "share"), "resource"));
    afl::base::Ref<afl::io::Directory> defaultSpecDirectory = fs.openDirectory(fs.makePathName(fs.makePathName(m_environment.getInstallationDirectoryName(), "share"), "specs"));

    // Set up GUI
    // - objects
    log().write(log().Debug, LOG_NAME, translator()("Starting GUI..."));
    ui::res::Manager mgr;
    mgr.addNewImageLoader(new ui::res::EngineImageLoader(engine));
    mgr.addNewImageLoader(new ui::res::CCImageLoader());
    mgr.addNewProvider(new ui::res::DirectoryProvider(resourceDirectory, fs, log(), translator()), "(MAIN)");
    mgr.addNewProvider(new ui::res::GeneratedPlanetProvider(), "(MAIN-PLANETS)");

    // - window parameters
    gfx::WindowParameters windowParams = params.getWindowParameters();
    windowParams.icon = mgr.loadImage("playvcr"); // loads playvcr.bmp

    // - window
    ui::DefaultResourceProvider provider(mgr, resourceDirectory, engine.dispatcher(), translator(), log());
    ui::Root root(engine, provider, windowParams);
    mgr.setScreenSize(root.getExtent().getSize());
    mgr.addNewProvider(new ui::res::GeneratedEngineProvider(provider.getFont("-"), translator()), "(MAIN-ENGINES)");
    root.sig_screenshot.addNewClosure(new ui::ScreenshotListener(fs, log(), translator()));

    // Setup network
    afl::net::tunnel::TunnelableNetworkStack net(m_networkStack);
    if (const String_t* p = params.getProxyAddress().get()) {
        net.add(*p);
    }

    // Set up HTTP
    // FIXME: do this here? We would have to do this elsewhere if it takes time; like, for loading config files.
    log().write(afl::sys::Log::Debug, LOG_NAME, translator()("Starting network..."));
    afl::net::http::Client client;
    afl::sys::Thread clientThread("http", client);
    client.setNewConnectionProvider(new ConnectionProvider(client, net));
    clientThread.start();
    afl::net::http::Manager httpManager(client);

    // At this point, the GUI is up and running.
    // This thread may now do nothing else than GUI.
    // All I/O accesses must from now on go through a background thread.
    // Set up session objects. None of these constructors block (I hope).
    log().write(afl::sys::Log::Debug, LOG_NAME, translator()("Starting background thread..."));
    game::Session gameSession(translator(), fs);
    gameSession.log().addListener(log());
    gameSession.setSystemInformation(util::getSystemInformation());
    game::interface::LabelExtra::create(gameSession);
    game::interface::TaskWaypoints::create(gameSession);

    // Password
    if (const String_t* p = params.getPassword().get()) {
        std::auto_ptr<game::AuthCache::Item> item(new game::AuthCache::Item());
        item->password = *p;
        gameSession.authCache().addNew(item.release());
    }

    // Set some variables
    gameSession.world().setNewGlobalValue("C2$RESOURCEDIRECTORY", interpreter::makeStringValue(resourceDirectory->getDirectoryName()));

    // Set up background thread and request receivers.
    // These must be after the session objects so that they die before them, allowing final requests to finish.
    util::RequestThread backgroundThread("game.background", log(), translator(), params.getRequestThreadDelay());
    util::RequestReceiver<game::Session> gameReceiver(backgroundThread, gameSession);
    RequestSender<game::browser::Session> browserSender = gameReceiver.getSender().makeTemporary(new BrowserInitializer(defaultSpecDirectory, profile, httpManager));

    // Set up foreground thread.
    client::si::UserSide userSide(root, gameReceiver.getSender(), translator(), root.engine().dispatcher(), collector, log());
    registerCommands(userSide);

    // Browser proxy
    client::UserCallback browserCallback(userSide);
    game::proxy::BrowserProxy browserProxy(browserSender, root.engine().dispatcher(), browserCallback);

    // Initialize by posting requests to the background thread.
    // (This will not take time.)
    gameReceiver.getSender().postNewRequest(new PluginInitializer(resourceDirectory, profile, params.getCommandLineResources()));

    // Command-line processing
    // Keep an action to execute after loading the BrowserScreen
    std::auto_ptr<BrowserAction_t> browserAction;
    if (const String_t* initialGameDirectory = params.getGameDirectory().get()) {
        switch (params.getDirectoryMode()) {
         case ApplicationParameters::OpenGame:
            browserProxy.openFolder(*initialGameDirectory);
            browserProxy.openParent(1);
            browserAction.reset(new AutoLoadAction(params.getPlayerNumber()));
            break;

         case ApplicationParameters::OpenBrowser:
            browserProxy.openFolder(*initialGameDirectory);
            break;
        }
    }

    // Script initialisation, wait for completion
    // (The NullControl will make us essentially responsive to UI from scripts.)
    {
        client::si::NullControl ctl(userSide);
        std::auto_ptr<client::si::ScriptTask> t(new ScriptInitializer(resourceDirectory, profile));
        ctl.executeTaskWait(t);
    }

    log().write(afl::sys::Log::Debug, LOG_NAME, translator()("Initialisation complete"));

    // Start game browser
    // FIXME: wrap this loop in a try/catch
    // FIXME: create the background image in the background thread
    ui::PixmapColorScheme docColors(root, generateBrowserBackground(log(), root.getExtent().getSize()));
    while (1) {
        // Helpful information
        ui::rich::DocumentView docView(root.getExtent().getSize(), 0, root.provider());
        docView.setExtent(gfx::Rectangle(gfx::Point(0, 0), docView.getLayoutInfo().getPreferredSize()));
        docView.getDocument().add(util::rich::Parser::parseXml("<big>PCC2ng</big>"));
        docView.getDocument().addNewline();
        docView.getDocument().addNewline();
        docView.getDocument().add(util::rich::Parser::parseXml("<font color=\"dim\">&#xA9; 2017-2025 Stefan Reuther &lt;streu@gmx.de&gt;</font>"));
        docView.getDocument().addNewline();
        docView.getDocument().finish();
        docView.handleDocumentUpdate();
        docView.adjustToDocumentSize();
        docView.setExtent(root.getExtent());
        docView.setColorScheme(docColors);
        root.add(docView);

        BrowserListener cb(userSide, browserSender, gameReceiver.getSender());
        client::screens::BrowserScreen browserScreen(cb, userSide, browserProxy, browserSender);
        cb.setScreen(&browserScreen);
        if (browserAction.get() != 0) {
            browserAction->call(browserScreen);
            browserAction.reset();
        }
        int result = browserScreen.run(docColors);
        if (result != 0) {
            // OK, play
            client::si::NullControl ctl(userSide);
            client::si::OutputState out;
            switch (result) {
             case ActionPlay:
                play(userSide);
                ctl.executeHookWait("AfterExit");
                break;

             case ActionSimulator:
                client::dialogs::doBattleSimulator(userSide, ctl, out);
                break;
            }
            userSide.reset();
            browserAction.reset(new AutoFocusAction(browserScreen.getCurrentPlayerNumber()));
        } else {
            // Close
            client::si::NullControl(userSide).executeHookWait("Quit");
            break;
        }
    }

    // Stop
    client.stop();
    clientThread.join();
}
