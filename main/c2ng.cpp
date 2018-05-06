/**
  *  \file main/c2ng.cpp
  */

#include <memory>
#include <ctime>
#include <stdlib.h>

#include "afl/base/closure.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/ref.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/http/client.hpp"
#include "afl/net/http/defaultconnectionprovider.hpp"
#include "afl/net/http/manager.hpp"
#include "afl/net/tunnel/tunnelablenetworkstack.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/proxytranslator.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/longcommandlineparser.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/translator.hpp"
#include "client/plugins.hpp"
#include "client/screens/browserscreen.hpp"
#include "client/screens/controlscreen.hpp"
#include "client/screens/playerscreen.hpp"
#include "client/session.hpp"
#include "client/si/control.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/scriptside.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/directoryhandler.hpp"
#include "game/game.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/pcc/browserhandler.hpp"
#include "game/session.hpp"
#include "game/specificationloader.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "gfx/application.hpp"
#include "gfx/complex.hpp"
#include "gfx/gen/orbitconfig.hpp"
#include "gfx/gen/spaceviewconfig.hpp"
#include "interpreter/values.hpp"
#include "ui/defaultresourceprovider.hpp"
#include "ui/draw.hpp"
#include "ui/pixmapcolorscheme.hpp"
#include "ui/res/ccimageloader.hpp"
#include "ui/res/directoryprovider.hpp"
#include "ui/res/engineimageloader.hpp"
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

namespace {
    const char LOG_NAME[] = "main";

    const char PROGRAM_TITLE[] = "PCC2 v" PCC2_VERSION;


    void waitForInitialisation(client::Session& clientSession, uint32_t waitId)
    {
        // Create a Control.
        // This is required to allow background scripts (=plugin initialisation) to call into UI.
        // We don't allow major interactions yet, though.
        class NullControl : public client::si::Control {
         public:
            NullControl(client::Session& session)
                : Control(session.interface(), session.root(), session.translator())
                { }
            virtual void handleStateChange(client::si::UserSide& ui, client::si::RequestLink2 link, client::si::OutputState::Target /*target*/)
                { ui.continueProcessWithFailure(link, "Context error"); }
            virtual void handleEndDialog(client::si::UserSide& ui, client::si::RequestLink2 link, int /*code*/)
                { ui.continueProcessWithFailure(link, "Context error"); }
            virtual void handlePopupConsole(client::si::UserSide& ui, client::si::RequestLink2 link)
                { ui.continueProcessWithFailure(link, "Context error"); }
            virtual client::si::ContextProvider* createContextProvider()
                { return 0; }
        };
        NullControl ctl(clientSession);
        ctl.attachPreparedWait(waitId);
    }

    class ScriptInitializer : public util::SlaveRequest<game::Session, client::si::ScriptSide> {
     public:
        ScriptInitializer(afl::base::Ref<afl::io::Directory> resourceDirectory, uint32_t waitId)
            : m_resourceDirectory(resourceDirectory),
              m_waitId(waitId)
            { }
        virtual void handle(game::Session& t, client::si::ScriptSide& ss)
            {
                // Configure load directory
                t.world().setSystemLoadDirectory(m_resourceDirectory.asPtr());

                // Get process list
                interpreter::ProcessList& processList = t.world().processList();

                // Create process to load core.q
                interpreter::Process& coreProcess = processList.create(t.world(), "<Core>");
                coreProcess.pushFrame(client::createFileLoader("core.q"), false);

                // Create process to load plugins
                interpreter::Process& pluginProcess = processList.create(t.world(), "<PluginLoader>");
                pluginProcess.pushFrame(client::createLoaderForUnloadedPlugins(t.plugins()), false);

                // Execute both processes in one group
                uint32_t pgid = processList.allocateProcessGroup();
                processList.resumeProcess(coreProcess, pgid);
                processList.resumeProcess(pluginProcess, pgid);
                ss.executeProcessGroupWait(m_waitId, pgid, t);
            }
     private:
        afl::base::Ref<afl::io::Directory> m_resourceDirectory;
        const uint32_t m_waitId;
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
                    afl::base::Ptr<afl::io::Stream> configFile = m_resourceDirectory->openFileNT("cc-res.cfg", afl::io::FileSystem::OpenRead);
                    if (configFile.get() != 0) {
                        std::auto_ptr<util::plugin::Plugin> plug(new util::plugin::Plugin("(GLOBAL CC-RES.CFG)"));
                        plug->initFromConfigFile(m_profile.open()->getDirectoryName(), session.translator().translateString("Global cc-res.cfg"), *configFile);
                        session.plugins().addNewPlugin(plug.release());
                    }
                }
                catch (...) { }

                try {
                    // User cc-res.cfg
                    afl::base::Ptr<afl::io::Stream> configFile = m_profile.openFileNT("cc-res.cfg");
                    if (configFile.get() != 0) {
                        std::auto_ptr<util::plugin::Plugin> plug(new util::plugin::Plugin("(USER CC-RES.CFG)"));
                        plug->initFromConfigFile(m_profile.open()->getDirectoryName(), session.translator().translateString("User cc-res.cfg"), *configFile);
                        session.plugins().addNewPlugin(plug.release());
                    }
                }
                catch (...) { }

                try {
                    // Plugins
                    session.plugins().findPlugins(*m_profile.open()->openDirectory("plugins"));
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

    class BrowserInitializer : public util::Request<game::browser::Session> {
     public:
        BrowserInitializer(afl::io::FileSystem& fileSystem,
                           afl::base::Ref<afl::io::Directory> defaultSpecDirectory,
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
                b.reset(new game::browser::Browser(m_fileSystem, t.translator(), t.log(), *t.accountManager(), m_profile, t.userCallbackProxy()));
                b->handlers().addNewHandler(new game::browser::DirectoryHandler(*b, m_defaultSpecDirectory, m_profile, m_fileSystem));
                b->handlers().addNewHandler(new game::pcc::BrowserHandler(*b, m_httpManager, m_defaultSpecDirectory, m_profile));
                b->handlers().addNewHandler(new game::nu::BrowserHandler(*b, m_httpManager, m_defaultSpecDirectory));
            }

     private:
        afl::io::FileSystem& m_fileSystem;
        afl::base::Ref<afl::io::Directory> m_defaultSpecDirectory;
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

                                m_root->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), m_player, *m_root, session);
                                session.getGame()->setViewpointPlayer(m_player);
                                session.getGame()->currentTurn().universe().postprocess(game::PlayerSet_t(m_player), game::PlayerSet_t(m_player), game::map::Object::Playable,
                                                                                        m_root->hostVersion(), m_root->hostConfiguration(),
                                                                                        session.getGame()->currentTurn().getTurnNumber(),
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
    

    class RootOptions {
     public:
        static const int MIN_WIDTH = 640;   // ex GFX_MIN_WIDTH
        static const int MIN_HEIGHT = 480;  // ex GFX_MIN_HEIGHT
        static const int MAX_DIM = 10000;
        RootOptions(afl::string::Translator& tx)
            : m_params(),
              m_translator(tx)
            {
                m_params.size = gfx::Point(800, 600);
                m_params.bitsPerPixel = 32;
                m_params.title = m_translator.translateString("Planets Command Center II (c2ng)");
            }
        String_t getHelp()
            {
                return m_translator.translateString("-fullscreen"     "\tRun fullscreen\n"
                                                    "-windowed"       "\tRun in a window\n"
                                                    "-bpp=N"          "\tUse color depth of N bits per pixel\n"
                                                    "-size=W[xH]"     "\tUse resolution of WxH pixels\n");
            }
        bool handleOption(const String_t& option, afl::sys::CommandLineParser& parser)
            {
                // ex gfx/init.cc:options
                if (option == "fullscreen") {
                    m_params.fullScreen = true;
                    return true;
                } else if (option == "windowed") {
                    m_params.fullScreen = false;
                    return true;
                } else if (option == "bpp") {
                    // ex gfx/init.cc:optSetBpp
                    util::StringParser sp(parser.getRequiredParameter(option));
                    int bpp = 0;
                    if (!sp.parseInt(bpp) || !sp.parseEnd()) {
                        throw afl::except::CommandLineException(m_translator.translateString("Invalid parameter to \"-bpp\""));
                    }
                    if (bpp != 8 && bpp != 16 && bpp != 32) {
                        throw afl::except::CommandLineException(m_translator.translateString("Parameter to \"-bpp\" must be 8, 16 or 32"));
                    }
                    m_params.bitsPerPixel = bpp;
                    return true;
                } else if (option == "hw") {
                    // FIXME: do we still need this option "-hw"? Should it be in engine options?
                    return false;
                } else if (option == "size") {
                    // ex gfx/init.cc:optSetSize
                    util::StringParser sp(parser.getRequiredParameter(option));
                    int w = 0, h = 0;
                    if (!sp.parseInt(w)) {
                        throw afl::except::CommandLineException(m_translator.translateString("Invalid parameter to \"-size\""));
                    }
                    if (sp.parseCharacter('X') || sp.parseCharacter('x') || sp.parseCharacter('*')) {
                        if (!sp.parseInt(h)) {
                            throw afl::except::CommandLineException(m_translator.translateString("Invalid parameter to \"-size\""));
                        }
                    } else {
                        // FIXME: PCC2 had a special case to recognize 1200 as 1200x1024, which is the only non-4:3 resolution.
                        h = 3*w/4;
                    }
                    if (!sp.parseEnd()) {
                        throw afl::except::CommandLineException(m_translator.translateString("Invalid parameter to \"-size\""));
                    }
                    if (w < MIN_WIDTH || h < MIN_HEIGHT || w > MAX_DIM || h > MAX_DIM) {
                        throw afl::except::CommandLineException(m_translator.translateString("Parameter to \"-size\" is out of range"));
                    }
                    m_params.size = gfx::Point(w, h);
                    return true;
                } else {
                    return false;
                }
            }

        const gfx::WindowParameters& getWindowParameters() const
            { return m_params; }

     private:
        gfx::WindowParameters m_params;
        afl::string::Translator& m_translator;
    };


    class CommandLineParameters {
     public:
        CommandLineParameters(afl::string::Translator& tx)
            : m_rootOptions(tx),
              m_haveGameDirectory(false),
              m_gameDirectory(),
              m_proxyAddress(),
              m_commandLineResources(),
              m_translator(tx)
            { }

        void parse(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl, afl::sys::Dialog& dialog)
            {
                afl::sys::LongCommandLineParser parser(cmdl);
                bool option;
                String_t text;
                while (parser.getNext(option, text)) {
                    if (option) {
                        if (m_rootOptions.handleOption(text, parser)) {
                            // ok
                        } else if (text == "resource") {
                            m_commandLineResources.push_back(parser.getRequiredParameter(text));
                        } else if (text == "proxy") {
                            m_proxyAddress = parser.getRequiredParameter(text);
                        } else if (text == "help") {
                            doHelp(dialog);
                        } else {
                            throw afl::except::CommandLineException(afl::string::Format(m_translator.translateString("Unknown command line parameter \"-%s\"").c_str(), text));
                        }
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

        void doHelp(afl::sys::Dialog& dialog)
            {
                String_t help = PROGRAM_TITLE;
                help += "\n\n";
                help += m_translator.translateString("Usage: c2ng [-options] gamedir");
                help += "\n\n";
                help += m_translator.translateString("Options:");
                help += "\n";
                help += util::formatOptions(m_translator.translateString("-resource=NAME\tAdd resource provider\n"
                                                                         "-proxy=URL\tSet network proxy\n")
                                            + m_rootOptions.getHelp());
                help += "\n";
                help += m_translator.translateString("(c) copyright 2017-2018 Stefan Reuther <streu@gmx.de>");
                help += "\n";
                dialog.showInfo(help, PROGRAM_TITLE);
                std::exit(0);
            }

        const std::vector<String_t>& getCommandLineResources() const
            { return m_commandLineResources; }

        RootOptions& rootOptions()
            { return m_rootOptions; }

        const afl::base::Optional<String_t>& getProxyAddress() const
            { return m_proxyAddress; }

     private:
        RootOptions m_rootOptions;
        bool m_haveGameDirectory;
        String_t m_gameDirectory;
        afl::base::Optional<String_t> m_proxyAddress;
        std::vector<String_t> m_commandLineResources;
        afl::string::Translator& m_translator;
    };

    afl::base::Ref<gfx::Canvas> generateGameBackground(afl::sys::LogListener& log, gfx::Point size, afl::string::Translator& tx)
    {
        uint32_t ticks = afl::sys::Time::getTickCounter();
        util::RandomNumberGenerator rng(ticks);
        gfx::gen::OrbitConfig config;
        config.setSize(size);
        afl::base::Ref<gfx::Canvas> result = config.render(rng)->makeCanvas();
        log.write(log.Trace, LOG_NAME, afl::string::Format(tx.translateString("Rendered game background in %d ms").c_str(), afl::sys::Time::getTickCounter() - ticks));
        return result;
    }

    afl::base::Ref<gfx::Canvas> generateBrowserBackground(afl::sys::LogListener& log, gfx::Point size, afl::string::Translator& tx)
    {
        uint32_t ticks = afl::sys::Time::getTickCounter();
        util::RandomNumberGenerator rng(ticks);
        gfx::gen::SpaceViewConfig cfg;
        cfg.setSize(size);
        cfg.setNumSuns(0);
        afl::base::Ref<gfx::Canvas> result = cfg.render(rng)->makeCanvas();
        log.write(log.Trace, LOG_NAME, afl::string::Format(tx.translateString("Rendered browser background in %d ms").c_str(), afl::sys::Time::getTickCounter() - ticks));
        return result;
    }

    void play(client::Session& session)
    {
        using client::si::OutputState;
        using client::si::InputState;
        ui::PixmapColorScheme colorScheme(session.root(), generateGameBackground(session.interface().mainLog(), session.root().getExtent().getSize(), session.translator()));
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
                // FIXME: save the game of course...
                running = false;
                break;

             case OutputState::PlayerScreen:
                client::screens::doPlayerScreen(session, in, out, colorScheme, first);
                first = false;
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::ShipScreen:
                client::screens::ControlScreen(session, game::map::Cursors::ShipScreen, client::screens::ControlScreen::ShipScreen).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::PlanetScreen:
                client::screens::ControlScreen(session, game::map::Cursors::PlanetScreen, client::screens::ControlScreen::PlanetScreen).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::BaseScreen:
                client::screens::ControlScreen(session, game::map::Cursors::BaseScreen, client::screens::ControlScreen::BaseScreen).run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;
            }
        }
    }

    class App : public gfx::Application {
     public:
        App(afl::sys::Dialog& dialog,
            afl::string::Translator& tx,
            afl::sys::Environment& env,
            afl::io::FileSystem& fs)
            : Application(dialog, tx, PROGRAM_TITLE),
              m_environment(env),
              m_fileSystem(fs)
            { }
                          
        void appMain(gfx::Engine& engine)
            {
                // Capture environment
                afl::io::FileSystem& fs = m_fileSystem;

                // Infrastructure (FIXME).
                util::ConsoleLogger console;
                console.attachWriter(true, m_environment.attachTextWriterNT(m_environment.Error));
                console.attachWriter(false, m_environment.attachTextWriterNT(m_environment.Output));
                log().addListener(console);
                util::ProfileDirectory profile(m_environment, m_fileSystem, translator(), log());

                // At this point we are safely operable.
                // Start collecting messages.
                // Starting from here, log messages will be retrievable
                util::MessageCollector collector;
                log().addListener(collector);
                console.setConfiguration("interpreter.process@Trace=hide");
                collector.setConfiguration("interpreter.process@Trace=hide");

                // Parse command line.
                CommandLineParameters params(translator());
                params.parse(m_environment.getCommandLine(), dialog());
                log().write(log().Info, LOG_NAME, afl::string::Format("[%s]", PROGRAM_TITLE));

                // Derived environment
                afl::base::Ref<afl::io::Directory> resourceDirectory    = fs.openDirectory(fs.makePathName(fs.makePathName(m_environment.getInstallationDirectoryName(), "share"), "resource"));
                afl::base::Ref<afl::io::Directory> defaultSpecDirectory = fs.openDirectory(fs.makePathName(fs.makePathName(m_environment.getInstallationDirectoryName(), "share"), "specs"));

                // Set up GUI
                // - objects
                log().write(log().Debug, LOG_NAME, translator().translateString("Starting GUI..."));
                ui::res::Manager mgr;
                mgr.addNewImageLoader(new ui::res::EngineImageLoader(engine));
                mgr.addNewImageLoader(new ui::res::CCImageLoader());
                mgr.addNewProvider(new ui::res::DirectoryProvider(resourceDirectory), "(MAIN)");
                mgr.addNewProvider(new ui::res::GeneratedPlanetProvider(), "(MAIN-PLANETS)");

                // - window parameters
                gfx::WindowParameters windowParams = params.rootOptions().getWindowParameters();
                windowParams.icon = mgr.loadImage("playvcr"); // loads playvcr.bmp

                // - window
                ui::DefaultResourceProvider provider(mgr, resourceDirectory, engine.dispatcher(), translator(), log());
                ui::Root root(engine, provider, windowParams);
                mgr.setScreenSize(root.getExtent().getSize());
                root.sig_screenshot.addNewClosure(new ui::ScreenshotListener(fs, log()));

                // Setup network
                afl::net::tunnel::TunnelableNetworkStack net(afl::net::NetworkStack::getInstance());
                if (const String_t* p = params.getProxyAddress().get()) {
                    net.add(*p);
                }

                // Set up HTTP
                // FIXME: do this here? We would have to do this elsewhere if it takes time; like, for loading config files.
                log().write(afl::sys::Log::Debug, LOG_NAME, translator().translateString("Starting network..."));
                afl::net::http::Client client;
                afl::sys::Thread clientThread("http", client);
                client.setNewConnectionProvider(new afl::net::http::DefaultConnectionProvider(client, net));
                clientThread.start();
                afl::net::http::Manager httpManager(client);

                // At this point, the GUI is up and running.
                // This thread may now do nothing else than GUI.
                // All I/O accesses must from now on go through a background thread.
                // Set up session objects. None of these constructors block (I hope).
                log().write(afl::sys::Log::Debug, LOG_NAME, translator().translateString("Starting background thread..."));
                game::browser::Session browserSession(translator(), log());
                game::Session gameSession(translator(), fs);
                gameSession.log().addListener(log());

                // Set some variables
                gameSession.world().setNewGlobalValue("C2$RESOURCEDIRECTORY", interpreter::makeStringValue(resourceDirectory->getDirectoryName()));

                // Set up background thread and request receivers.
                // These must be after the session objects so that they die before them, allowing final requests to finish.
                util::RequestThread backgroundThread("game.background", log());
                util::RequestReceiver<game::browser::Session> browserReceiver(backgroundThread, browserSession);
                util::RequestReceiver<game::Session> gameReceiver(backgroundThread, gameSession);

                // Set up foreground thread.
                client::Session clientSession(root, gameReceiver.getSender(), translator(), collector, log());

                // Initialize by posting requests to the background thread.
                // (This will not take time.)
                uint32_t scriptWaitId = clientSession.interface().allocateWaitId();
                gameReceiver.getSender().postNewRequest(new PluginInitializer(resourceDirectory, profile, params.getCommandLineResources()));
                clientSession.interface().postNewRequest(new ScriptInitializer(resourceDirectory, scriptWaitId));
                browserReceiver.getSender().postNewRequest(new BrowserInitializer(fs, defaultSpecDirectory, profile, httpManager));
                {
                    String_t initialGameDirectory;
                    if (params.getGameDirectory(initialGameDirectory)) {
                        browserReceiver.getSender().postNewRequest(new BrowserPositioner(initialGameDirectory));
                    }
                }

                // Wait for completion of initialisation
                waitForInitialisation(clientSession, scriptWaitId);
                log().write(afl::sys::Log::Debug, LOG_NAME, translator().translateString("Initialisation complete"));

                // Start game browser
                // FIXME: wrap this loop in a try/catch
                // FIXME: create the background image in the background thread
                ui::PixmapColorScheme docColors(root, generateBrowserBackground(log(), root.getExtent().getSize(), translator()));
                while (1) {
                    // Helpful information
                    ui::rich::DocumentView docView(root.getExtent().getSize(), 0, root.provider());
                    docView.setExtent(gfx::Rectangle(gfx::Point(0, 0), docView.getLayoutInfo().getPreferredSize()));
                    docView.getDocument().add(util::rich::Parser::parseXml("<big>PCC2ng Milestone Four</big>"));
                    docView.getDocument().addNewline();
                    docView.getDocument().addNewline();
                    docView.getDocument().add(util::rich::Parser::parseXml("<font color=\"dim\">&#xA9; 2017-2018 Stefan Reuther &lt;streu@gmx.de&gt;</font>"));
                    docView.getDocument().addNewline();
                    docView.getDocument().finish();
                    docView.handleDocumentUpdate();
                    docView.adjustToDocumentSize();
                    docView.setExtent(root.getExtent());
                    docView.setColorScheme(docColors);
                    root.add(docView);

                    // Browser
                    client::screens::BrowserScreen browserScreen(root, browserReceiver.getSender());
                    browserScreen.sig_gameSelection.addNewClosure(new BrowserListener(browserScreen, browserReceiver.getSender(), gameReceiver.getSender()));
                    int result = browserScreen.run(docColors);
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
     private:
        afl::sys::Environment& m_environment;
        afl::io::FileSystem& m_fileSystem;
    };
}


int main(int, char** argv)
{
    // Capture environment
    afl::sys::Dialog& dialog = afl::sys::Dialog::getSystemInstance();
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();

    // Infrastructure (FIXME).
#if 0
    afl::test::Translator tx("\xC2\xAB", "\xC2\xBB");
#else
    afl::string::NullTranslator tx;
#endif
    afl::string::Translator::setSystemInstance(std::auto_ptr<afl::string::Translator>(new afl::string::ProxyTranslator(tx)));

    // Application
    return App(dialog, tx, env, fs).run();
}
