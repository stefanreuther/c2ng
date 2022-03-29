/**
  *  \file main/c2ng.cpp
  */

#include <memory>
#include <ctime>
#include <stdlib.h>

#include "afl/base/closure.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/http/client.hpp"
#include "afl/net/http/clientconnection.hpp"
#include "afl/net/http/clientconnectionprovider.hpp"
#include "afl/net/http/manager.hpp"
#include "afl/net/securenetworkstack.hpp"
#include "afl/net/tunnel/tunnelablenetworkstack.hpp"
#include "afl/string/format.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/proxytranslator.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/longcommandlineparser.hpp"
#include "afl/sys/mutexguard.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/translator.hpp"
#include "client/map/screen.hpp"
#include "client/plugins.hpp"
#include "client/screens/browserscreen.hpp"
#include "client/screens/controlscreen.hpp"
#include "client/screens/playerscreen.hpp"
#include "client/si/commands.hpp"
#include "client/si/control.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "client/usercallback.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/directoryhandler.hpp"
#include "game/game.hpp"
#include "game/interface/vmfile.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/pcc/browserhandler.hpp"
#include "game/proxy/browserproxy.hpp"
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
#include "game/actions/preconditions.hpp"
#include "game/interface/simpleprocedure.hpp"

namespace {
    const char LOG_NAME[] = "main";

    const char PROGRAM_TITLE[] = "PCC2 v" PCC2_VERSION;

    class NullControl : public client::si::Control {
     public:
        NullControl(client::si::UserSide& us)
            : Control(us)
            { }
        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target /*target*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleEndDialog(client::si::RequestLink2 link, int /*code*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handlePopupConsole(client::si::RequestLink2 link)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleSetView(client::si::RequestLink2 link, String_t /*name*/, bool /*withKeymap*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t /*name*/, int /*prefix*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t /*text*/)
            { interface().continueProcessWithFailure(link, "Context error"); }
        virtual game::interface::ContextProvider* createContextProvider()
            { return 0; }
    };

    class ScriptInitializer : public client::si::ScriptTask {
     public:
        ScriptInitializer(afl::base::Ref<afl::io::Directory> resourceDirectory)
            : m_resourceDirectory(resourceDirectory)
            { }
        virtual void execute(uint32_t pgid, game::Session& t)
            {
                // Configure load directory
                t.world().setSystemLoadDirectory(m_resourceDirectory.asPtr());

                // Get process list
                interpreter::ProcessList& processList = t.processList();

                // Create process to load core.q
                interpreter::Process& coreProcess = processList.create(t.world(), "<Core>");
                coreProcess.pushFrame(client::createFileLoader("core.q", "core.q"), false);

                // Create process to load plugins
                interpreter::Process& pluginProcess = processList.create(t.world(), "<PluginLoader>");
                pluginProcess.pushFrame(client::createLoaderForUnloadedPlugins(t.plugins()), false);

                // Execute both processes
                processList.resumeProcess(coreProcess, pgid);
                processList.resumeProcess(pluginProcess, pgid);
            }
     private:
        afl::base::Ref<afl::io::Directory> m_resourceDirectory;
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
     *  The BrowserListener is invoked when the user wants to open a game.
     *  This is currently pretty ugly spaghetti code waiting to be factored into nice re-usable methods.
     */
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
                    class Then : public game::Task_t {
                     public:
                        Then(game::browser::Session& session, int player, util::RequestSender<client::screens::BrowserScreen> uiSender, util::RequestSender<game::Session> gameSender)
                            : m_session(session), m_player(player), m_uiSender(uiSender), m_gameSender(gameSender)
                            { }
                        virtual void call()
                            {
                                m_session.log().write(afl::sys::LogListener::Trace, LOG_NAME, "Task: LoadRequest");
                                m_gameSender.postNewRequest(new LoadRequest2(m_player, m_session.browser().getSelectedRoot(), m_uiSender));
                                m_session.finishTask();
                            }
                     private:
                        game::browser::Session& m_session;
                        int m_player;
                        util::RequestSender<client::screens::BrowserScreen> m_uiSender;
                        util::RequestSender<game::Session> m_gameSender;
                    };

                    session.addTask(session.browser().loadChildRoot(std::auto_ptr<game::Task_t>(new Then(session, m_player, m_uiSender, m_gameSender))));
                }

         private:
            int m_player;
            util::RequestSender<client::screens::BrowserScreen> m_uiSender;
            util::RequestSender<game::Session> m_gameSender;
        };

        static void IFLoadTurn(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args)
            {
                args.checkArgumentCount(1);
                int32_t player;
                if (!interpreter::checkIntegerArg(player, args.getNext())) {
                    throw interpreter::Error::typeError(interpreter::Error::ExpectInteger);
                }

                class Fail : public game::Task_t {
                 public:
                    Fail(interpreter::Process& proc, game::Session& session)
                        : m_process(proc), m_session(session)
                        { }
                    virtual void call()
                        {
                            // Make a copy of m_session.
                            // The continueProcessWithFailure() will destroy the Task.
                            game::Session& session = m_session;
                            session.log().write(afl::sys::LogListener::Trace, LOG_NAME, "LoadRequest.Fail");
                            session.processList().continueProcessWithFailure(m_process, "Load error");
                            session.sig_runRequest.raise();
                        }
                 private:
                    interpreter::Process& m_process;
                    game::Session& m_session;
                };

                class Task : public game::Task_t {
                 public:
                    Task(interpreter::Process& proc, game::Session& session, int player)
                        : m_process(proc), m_session(session), m_player(player)
                        { }
                    virtual void call()
                        {
                            m_session.log().write(afl::sys::LogListener::Trace, LOG_NAME, "LoadRequest.Task");
                            game::Root& root = game::actions::mustHaveRoot(m_session);
                            m_session.getGame()->setViewpointPlayer(m_player);

                            if (root.userConfiguration()[game::config::UserConfiguration::Team_AutoSync]()) {
                                m_session.getGame()->synchronizeTeamsFromAlliances();
                            }

                            game::map::Object::Playability playability;
                            game::Session::AreaSet_t editableAreas;
                            if (root.getPossibleActions().contains(game::Root::aLoadEditable) && !root.userConfiguration()[game::config::UserConfiguration::Game_ReadOnly]()) {
                                if (root.userConfiguration()[game::config::UserConfiguration::Game_Finished]()) {
                                    // Finished game
                                    playability = game::map::Object::ReadOnly;
                                } else {
                                    // Active game
                                    playability = game::map::Object::Playable;
                                    editableAreas += game::Session::CommandArea;
                                }
                                editableAreas += game::Session::LocalDataArea;
                            } else {
                                // View only
                                playability = game::map::Object::ReadOnly;
                            }

                            m_session.setEditableAreas(editableAreas);
                            m_session.log().write(afl::sys::LogListener::Error, LOG_NAME, m_session.translator()("Compiling starchart..."));
                            m_session.getGame()->currentTurn().universe().postprocess(game::PlayerSet_t(m_player), game::PlayerSet_t(m_player), playability,
                                                                                      root.hostVersion(), root.hostConfiguration(),
                                                                                      m_session.getGame()->currentTurn().getTurnNumber(),
                                                                                      *m_session.getShipList(),
                                                                                      m_session.translator(), m_session.log());
                            m_session.getGame()->currentTurn().alliances().postprocess();

                            // Load VM
                            try {
                                game::interface::loadVM(m_session, m_player);
                            }
                            catch (std::exception& e) {
                                m_session.log().write(afl::sys::LogListener::Error, LOG_NAME, m_session.translator()("Unable to scripts and auto-tasks"), e);
                            }

                            // Resume
                            // Make a copy of m_session.
                            // The continueProcessWithFailure() will destroy the Task.
                            game::Session& session = m_session;
                            session.processList().continueProcess(m_process);
                            session.sig_runRequest.raise();
                        }
                 private:
                    interpreter::Process& m_process;
                    game::Session& m_session;
                    int m_player;
                };

                game::Root& root = game::actions::mustHaveRoot(session);
                proc.suspend(root.specificationLoader().loadShipList(
                                 *session.getShipList(), root,
                                 game::makeConditionalTask(
                                     root.getTurnLoader()->loadCurrentTurn(
                                         session.getGame()->currentTurn(), *session.getGame(), player, root, session,
                                         game::makeConditionalTask(std::auto_ptr<game::Task_t>(new Task(proc, session, player)),
                                                                   std::auto_ptr<game::Task_t>(new Fail(proc, session)))),
                                     std::auto_ptr<game::Task_t>(new Fail(proc, session)))));
            }

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
                            // We need a process context to be able to suspend, so we do the bulk in a function IFLoadTurn,
                            // and use a dummy process to invoke that.
                            // (An alternative would have been to run this as a browser task.)
                            class Finalizer : public interpreter::Process::Finalizer {
                             public:
                                Finalizer(util::RequestSender<client::screens::BrowserScreen> uiSender)
                                    : m_uiSender(uiSender)
                                    { }
                                ~Finalizer()
                                    { /* TODO: unexpected stop */ }
                                virtual void finalizeProcess(interpreter::Process& p)
                                    {
                                        if (p.getState() == interpreter::Process::Ended) {
                                            m_uiSender.postNewRequest(new ConfirmRequest(true));
                                        } else {
                                            m_uiSender.postNewRequest(new ConfirmRequest(false));
                                        }
                                    }
                             private:
                                util::RequestSender<client::screens::BrowserScreen> m_uiSender;
                            };

                            session.setGame(new game::Game());
                            session.setRoot(m_root);
                            session.setShipList(new game::spec::ShipList());

                            interpreter::Process& proc = session.processList().create(session.world(), "<Loader>");
                            interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
                            proc.pushNewValue(interpreter::makeIntegerValue(m_player));
                            proc.pushNewValue(new game::interface::SimpleProcedure(session, IFLoadTurn));
                            bco->addInstruction(interpreter::Opcode::maIndirect, interpreter::Opcode::miIMCall, 1);
                            proc.pushFrame(bco, false);
                            proc.setNewFinalizer(new Finalizer(m_uiSender));

                            uint32_t pgid = session.processList().allocateProcessGroup();
                            session.processList().resumeProcess(proc, pgid);
                            session.processList().startProcessGroup(pgid);
                            session.processList().run();
                            session.processList().removeTerminatedProcesses();
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
                m_params.title = m_translator("Planets Command Center II (c2ng)");
            }
        String_t getHelp()
            {
                return m_translator("-fullscreen"     "\tRun fullscreen\n"
                                    "-windowed"       "\tRun in a window\n"
                                    "-bpp=N"          "\tUse color depth of N bits per pixel\n"
                                    "-size=W[xH]"     "\tUse resolution of WxH pixels\n"
                                    "-nomousegrab"    "\tDon't grab (lock into window) mouse pointer\n");
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
                } else if (option == "nomousegrab") {
                    m_params.disableGrab = true;
                    return true;
                } else if (option == "bpp") {
                    // ex gfx/init.cc:optSetBpp
                    util::StringParser sp(parser.getRequiredParameter(option));
                    int bpp = 0;
                    if (!sp.parseInt(bpp) || !sp.parseEnd()) {
                        throw afl::except::CommandLineException(m_translator("Invalid parameter to \"-bpp\""));
                    }
                    if (bpp != 8 && bpp != 16 && bpp != 32) {
                        throw afl::except::CommandLineException(m_translator("Parameter to \"-bpp\" must be 8, 16 or 32"));
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
                        throw afl::except::CommandLineException(m_translator("Invalid parameter to \"-size\""));
                    }
                    if (sp.parseCharacter('X') || sp.parseCharacter('x') || sp.parseCharacter('*')) {
                        if (!sp.parseInt(h)) {
                            throw afl::except::CommandLineException(m_translator("Invalid parameter to \"-size\""));
                        }
                    } else {
                        // FIXME: PCC2 had a special case to recognize 1200 as 1200x1024, which is the only non-4:3 resolution.
                        h = 3*w/4;
                    }
                    if (!sp.parseEnd()) {
                        throw afl::except::CommandLineException(m_translator("Invalid parameter to \"-size\""));
                    }
                    if (w < MIN_WIDTH || h < MIN_HEIGHT || w > MAX_DIM || h > MAX_DIM) {
                        throw afl::except::CommandLineException(m_translator("Parameter to \"-size\" is out of range"));
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
              m_traceConfig(),
              m_proxyAddress(),
              m_commandLineResources(),
              m_translator(tx),
              m_requestThreadDelay(0)
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
                        } else if (text == "log") {
                            util::addListItem(m_traceConfig, ":", parser.getRequiredParameter(text));
                        } else if (text == "debug-request-delay") {
                            int value = 0;
                            if (!afl::string::strToInteger(parser.getRequiredParameter(text), value) || value < 0) {
                                throw afl::except::CommandLineException(afl::string::Format(m_translator("Invalid argument to command line parameter \"-%s\""), text));
                            }
                            m_requestThreadDelay = value;
                        } else {
                            throw afl::except::CommandLineException(afl::string::Format(m_translator("Unknown command line parameter \"-%s\""), text));
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

        int getRequestThreadDelay() const
            { return m_requestThreadDelay; }

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
                help += m_translator("Usage: c2ng [-options] gamedir");
                help += "\n\n";
                help += m_translator("Options:");
                help += "\n";
                help += util::formatOptions(m_translator("-resource=NAME\tAdd resource provider\n"
                                                         "-proxy=URL\tSet network proxy\n")
                                            + m_rootOptions.getHelp());
                help += "\n";
                help += m_translator("(c) copyright 2017-2022 Stefan Reuther <streu@gmx.de>");
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

        const String_t& getTraceConfiguration() const
            { return m_traceConfig; }

     private:
        RootOptions m_rootOptions;
        bool m_haveGameDirectory;
        String_t m_gameDirectory;
        String_t m_traceConfig;
        afl::base::Optional<String_t> m_proxyAddress;
        std::vector<String_t> m_commandLineResources;
        afl::string::Translator& m_translator;
        int m_requestThreadDelay;
    };

    afl::base::Ref<gfx::Canvas> generateGameBackground(afl::sys::LogListener& log, gfx::Point size, afl::string::Translator& tx)
    {
        uint32_t ticks = afl::sys::Time::getTickCounter();
        util::RandomNumberGenerator rng(ticks);
        gfx::gen::OrbitConfig config;
        config.setSize(size);
        afl::base::Ref<gfx::Canvas> result = config.render(rng)->makeCanvas();
        log.write(log.Trace, LOG_NAME, afl::string::Format(tx("Rendered game background in %d ms"), afl::sys::Time::getTickCounter() - ticks));
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
        log.write(log.Trace, LOG_NAME, afl::string::Format(tx("Rendered browser background in %d ms"), afl::sys::Time::getTickCounter() - ticks));
        return result;
    }

    void play(client::si::UserSide& us)
    {
        using client::si::OutputState;
        using client::si::InputState;
        ui::PixmapColorScheme colorScheme(us.root(), generateGameBackground(us.mainLog(), us.root().getExtent().getSize(), us.translator()));
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
                // FIXME: run EXIT hook
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

             case OutputState::ShipTaskScreen:
                client::screens::ControlScreen(us, game::map::Cursors::ShipScreen, client::screens::ControlScreen::ShipTaskScreen)
                    .withTaskEditor(interpreter::Process::pkShipTask)
                    .run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::PlanetTaskScreen:
                client::screens::ControlScreen(us, game::map::Cursors::PlanetScreen, client::screens::ControlScreen::PlanetTaskScreen)
                    .withTaskEditor(interpreter::Process::pkPlanetTask)
                    .run(in, out);
                in = InputState();
                in.setProcess(out.getProcess());
                state = out.getTarget();
                break;

             case OutputState::BaseTaskScreen:
                client::screens::ControlScreen(us, game::map::Cursors::BaseScreen, client::screens::ControlScreen::BaseTaskScreen)
                    .withTaskEditor(interpreter::Process::pkBaseTask)
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
                console.setConfiguration("*@Trace=hide");
                collector.setConfiguration("*@Trace=hide");

                // Parse command line.
                CommandLineParameters params(translator());
                params.parse(m_environment.getCommandLine(), dialog());
                if (!params.getTraceConfiguration().empty()) {
                    console.setConfiguration(params.getTraceConfiguration());
                    collector.setConfiguration(params.getTraceConfiguration());
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
                gfx::WindowParameters windowParams = params.rootOptions().getWindowParameters();
                windowParams.icon = mgr.loadImage("playvcr"); // loads playvcr.bmp

                // - window
                ui::DefaultResourceProvider provider(mgr, resourceDirectory, engine.dispatcher(), translator(), log());
                ui::Root root(engine, provider, windowParams);
                mgr.setScreenSize(root.getExtent().getSize());
                mgr.addNewProvider(new ui::res::GeneratedEngineProvider(provider.getFont("-"), translator()), "(MAIN-ENGINES)");
                root.sig_screenshot.addNewClosure(new ui::ScreenshotListener(fs, log(), translator()));

                // Setup network
                afl::net::tunnel::TunnelableNetworkStack net(afl::net::NetworkStack::getInstance());
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

                // Set some variables
                gameSession.world().setNewGlobalValue("C2$RESOURCEDIRECTORY", interpreter::makeStringValue(resourceDirectory->getDirectoryName()));

                // Set up background thread and request receivers.
                // These must be after the session objects so that they die before them, allowing final requests to finish.
                util::RequestThread backgroundThread("game.background", log(), translator(), params.getRequestThreadDelay());
                util::RequestReceiver<game::Session> gameReceiver(backgroundThread, gameSession);
                util::RequestSender<game::browser::Session> browserSender = gameReceiver.getSender().makeTemporary(new BrowserInitializer(defaultSpecDirectory, profile, httpManager));

                // Set up foreground thread.
                client::si::UserSide userSide(root, gameReceiver.getSender(), translator(), root.engine().dispatcher(), collector, log());
                registerCommands(userSide);

                // Browser proxy
                client::UserCallback browserCallback(userSide);
                game::proxy::BrowserProxy browserProxy(browserSender, root.engine().dispatcher(), browserCallback);

                // Initialize by posting requests to the background thread.
                // (This will not take time.)
                gameReceiver.getSender().postNewRequest(new PluginInitializer(resourceDirectory, profile, params.getCommandLineResources()));
                {
                    String_t initialGameDirectory;
                    if (params.getGameDirectory(initialGameDirectory)) {
                        browserProxy.openFolder(initialGameDirectory);
                        browserProxy.openParent(1);
                    }
                }

                // Script initialisation, wait for completion
                // (The NullControl will make us essentially responsive to UI from scripts.)
                {
                    NullControl ctl(userSide);
                    std::auto_ptr<client::si::ScriptTask> t(new ScriptInitializer(resourceDirectory));
                    ctl.executeTaskWait(t);
                }

                log().write(afl::sys::Log::Debug, LOG_NAME, translator()("Initialisation complete"));

                // Start game browser
                // FIXME: wrap this loop in a try/catch
                // FIXME: create the background image in the background thread
                // FIXME: run hooks
                ui::PixmapColorScheme docColors(root, generateBrowserBackground(log(), root.getExtent().getSize(), translator()));
                while (1) {
                    // Helpful information
                    ui::rich::DocumentView docView(root.getExtent().getSize(), 0, root.provider());
                    docView.setExtent(gfx::Rectangle(gfx::Point(0, 0), docView.getLayoutInfo().getPreferredSize()));
                    docView.getDocument().add(util::rich::Parser::parseXml("<big>PCC2ng</big>"));
                    docView.getDocument().addNewline();
                    docView.getDocument().addNewline();
                    docView.getDocument().add(util::rich::Parser::parseXml("<font color=\"dim\">&#xA9; 2017-2022 Stefan Reuther &lt;streu@gmx.de&gt;</font>"));
                    docView.getDocument().addNewline();
                    docView.getDocument().finish();
                    docView.handleDocumentUpdate();
                    docView.adjustToDocumentSize();
                    docView.setExtent(root.getExtent());
                    docView.setColorScheme(docColors);
                    root.add(docView);

                    // Browser
                    client::screens::BrowserScreen browserScreen(root, translator(), browserProxy, gameReceiver.getSender());
                    browserScreen.sig_gameSelection.addNewClosure(new BrowserListener(browserScreen, browserSender, gameReceiver.getSender()));

                    int result = browserScreen.run(docColors);
                    if (result != 0) {
                        // OK, play
                        play(userSide);
                        userSide.reset();
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
