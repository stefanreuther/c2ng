/**
  *  \file client/si/commands.cpp
  */

#include "client/si/commands.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "client/cargotransfer.hpp"
#include "client/dialogs/alliancedialog.hpp"
#include "client/dialogs/buildammo.hpp"
#include "client/dialogs/buildqueuedialog.hpp"
#include "client/dialogs/buildship.hpp"
#include "client/dialogs/buildstarbasedialog.hpp"
#include "client/dialogs/buildstructuresdialog.hpp"
#include "client/dialogs/buysuppliesdialog.hpp"
#include "client/dialogs/cargohistorydialog.hpp"
#include "client/dialogs/commandlistdialog.hpp"
#include "client/dialogs/entercoordinates.hpp"
#include "client/dialogs/fileselectiondialog.hpp"
#include "client/dialogs/friendlycodedialog.hpp"
#include "client/dialogs/helpdialog.hpp"
#include "client/dialogs/historyship.hpp"
#include "client/dialogs/hullspecification.hpp"
#include "client/dialogs/inboxdialog.hpp"
#include "client/dialogs/ionstorminfo.hpp"
#include "client/dialogs/keymapdialog.hpp"
#include "client/dialogs/messageeditor.hpp"
#include "client/dialogs/messagereceiver.hpp"
#include "client/dialogs/minefieldinfo.hpp"
#include "client/dialogs/multitransfer.hpp"
#include "client/dialogs/navchartdialog.hpp"
#include "client/dialogs/notifications.hpp"
#include "client/dialogs/objectselectiondialog.hpp"
#include "client/dialogs/outboxdialog.hpp"
#include "client/dialogs/planetinfodialog.hpp"
#include "client/dialogs/processlistdialog.hpp"
#include "client/dialogs/revertdialog.hpp"
#include "client/dialogs/scores.hpp"
#include "client/dialogs/screenhistorydialog.hpp"
#include "client/dialogs/searchdialog.hpp"
#include "client/dialogs/selectionmanager.hpp"
#include "client/dialogs/sellsuppliesdialog.hpp"
#include "client/dialogs/sessionfileselectiondialog.hpp"
#include "client/dialogs/shipspeeddialog.hpp"
#include "client/dialogs/simulationtransfer.hpp"
#include "client/dialogs/simulator.hpp"
#include "client/dialogs/specbrowserdialog.hpp"
#include "client/dialogs/taxationdialog.hpp"
#include "client/dialogs/teamsettings.hpp"
#include "client/dialogs/techupgradedialog.hpp"
#include "client/dialogs/turnlistdialog.hpp"
#include "client/dialogs/ufoinfo.hpp"
#include "client/dialogs/vcrplayer.hpp"
#include "client/dialogs/visualscandialog.hpp"
#include "client/help.hpp"
#include "client/proxy/screenhistoryproxy.hpp"
#include "client/si/control.hpp"
#include "client/si/dialogfunction.hpp"
#include "client/si/listboxfunction.hpp"
#include "client/si/remotecontrol.hpp"
#include "client/si/requestlink1.hpp"
#include "client/si/requestlink2.hpp"
#include "client/si/scriptprocedure.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/userside.hpp"
#include "client/si/usertask.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/playersetselector.hpp"
#include "game/actions/multitransfersetup.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/interface/plugincontext.hpp"
#include "game/interface/richtextfunctions.hpp"
#include "game/interface/richtextvalue.hpp"
#include "game/interface/simplefunction.hpp"
#include "game/interface/simpleprocedure.hpp"
#include "game/interface/vmfile.hpp"
#include "game/map/chunnelmission.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/shipinfo.hpp"
#include "game/map/shippredictor.hpp"
#include "game/proxy/buildammoproxy.hpp"
#include "game/proxy/chunnelproxy.hpp"
#include "game/proxy/inboxadaptor.hpp"
#include "game/proxy/maplocationproxy.hpp"
#include "game/proxy/outboxproxy.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/searchproxy.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/searchquery.hpp"
#include "game/sim/sessionextra.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/keymapvalue.hpp"
#include "interpreter/values.hpp"
#include "ui/defaultresourceprovider.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/res/factory.hpp"
#include "ui/res/manager.hpp"
#include "ui/res/provider.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "util/math.hpp"
#include "util/rich/parser.hpp"
#include "util/rich/text.hpp"
#include "util/unicodechars.hpp"

using client::si::UserTask;
using client::si::UserSide;
using client::si::Control;
using client::si::OutputState;
using client::si::RequestLink1;
using client::si::RequestLink2;
using client::si::ScriptSide;
using client::ScreenHistory;
using game::interface::SimpleFunction;
using game::interface::SimpleProcedure;

namespace {
    const char*const LOG_NAME = "client.si";

    typedef interpreter::Process::Task_t Task_t;
    typedef afl::base::Closure<void(bool)> PostSaveTask_t;

    class StateChangeTask : public UserTask {
     public:
        StateChangeTask(OutputState::Target target)
            : m_target(target)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                ctl.interface().mainLog().write(afl::sys::LogListener::Trace, LOG_NAME, afl::string::Format("<%p> handleStateChange(%s, %s)", &ctl, link.toString(), OutputState::toString(m_target)));
                ctl.handleStateChange(link, m_target);
            }
     private:
        OutputState::Target m_target;
    };

    class PopupConsoleTask : public UserTask {
     public:
        PopupConsoleTask()
            { }
        void handle(Control& ctl, RequestLink2 link)
            { ctl.handlePopupConsole(link); }
    };

    class MessageBoxTask : public UserTask {
     public:
        MessageBoxTask(game::interface::RichTextValue::Ptr_t pContent, String_t heading)
            : m_content(*pContent), m_heading(heading)
            { }
        MessageBoxTask(util::rich::Text content, String_t heading)
            : m_content(content), m_heading(heading)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                ui::dialogs::MessageBox(m_content, m_heading, ctl.root()).doOkDialog(ctl.translator());
                ctl.interface().continueProcess(link);
            }
     private:
        util::rich::Text m_content;
        String_t m_heading;
    };

    class ViewMailboxTask : public UserTask {
     public:
        ViewMailboxTask(game::proxy::InboxAdaptor_t* maker)
            : m_maker(maker)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                OutputState out;
                client::dialogs::InboxDialog(ctl.translator()("Messages"), iface.gameSender().makeTemporary(m_maker.release()), iface, ctl.root(), ctl.translator())
                    .run(out, "pcc2:msgin", ctl.translator()("No messages"));

                iface.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }
     private:
        std::auto_ptr<game::proxy::InboxAdaptor_t> m_maker;
    };

    void enterScreen(int screen, OutputState::Target target, int32_t obj, game::Session& session, ScriptSide& si, RequestLink1 link)
    {
        if (session.getGame().get() == 0) {
            throw game::Exception(game::Exception::eUser);
        }
        if (obj != 0) {
            game::map::ObjectType* ty = session.getGame()->cursors().getTypeByNumber(screen);
            if (!ty || ty->getObjectByIndex(obj) == 0) {
                throw interpreter::Error::rangeError();
            }
            game::map::ObjectCursor* cu = session.getGame()->cursors().getCursorByNumber(screen);
            if (!cu) {
                throw interpreter::Error::rangeError();
            }
            cu->setCurrentIndex(obj);
        }

        // Do it
        si.postNewTask(link, new StateChangeTask(target));
    }

    void activateReference(ScreenHistory::Reference ref, UserSide& iface, Control& ctl, RequestLink2 link)
    {
        client::proxy::ScreenHistoryProxy proxy(iface.gameSender());
        client::Downlink downLink(ctl.root(), ctl.translator());

        bool ok = false;
        if (proxy.activateReference(downLink, ref)) {
            switch (ref.getType()) {
             case ScreenHistory::Null:
                break;
             case ScreenHistory::Ship:
                ctl.handleStateChange(link, OutputState::ShipScreen);
                ok = true;
                break;
             case ScreenHistory::Planet:
                ctl.handleStateChange(link, OutputState::PlanetScreen);
                ok = true;
                break;
             case ScreenHistory::Starbase:
                ctl.handleStateChange(link, OutputState::BaseScreen);
                ok = true;
                break;
             case ScreenHistory::ShipTask:
                ctl.handleStateChange(link, OutputState::ShipTaskScreen);
                ok = true;
                break;
             case ScreenHistory::PlanetTask:
                ctl.handleStateChange(link, OutputState::PlanetTaskScreen);
                ok = true;
                break;
             case ScreenHistory::StarbaseTask:
                ctl.handleStateChange(link, OutputState::BaseTaskScreen);
                ok = true;
                break;
             case ScreenHistory::Starchart:
                ctl.handleStateChange(link, OutputState::Starchart);
                ok = true;
                break;
            }
        }
        if (!ok) {
            iface.continueProcess(link);
        }
    }

    bool isCurrentScreenRegistered(game::Session& session)
    {
        std::auto_ptr<afl::data::Value> result(session.uiPropertyStack().get(game::interface::iuiScreenRegistered));
        return interpreter::getBooleanValue(result.get()) > 0;
    }

    struct PluginContext {
        String_t name;
        String_t directory;
    };

    PluginContext findPluginContext(RequestLink1 link)
    {
        PluginContext result;
        result.name = "Script";
        result.directory = "";

        const afl::container::PtrVector<interpreter::Context>& ctxs = link.getProcess().getContexts();
        for (size_t i = ctxs.size(); i > 0; --i) {
            if (game::interface::PluginContext* plugContext = dynamic_cast<game::interface::PluginContext*>(ctxs[i-1])) {
                // Fetch plugin's Id and directory to produce a context
                std::auto_ptr<afl::data::Value> tmp;
                interpreter::Context::PropertyIndex_t index;
                if (interpreter::Context::PropertyAccessor* indexContext = plugContext->lookup("ID", index)) {
                    tmp.reset(indexContext->get(index));
                    result.name = interpreter::toString(tmp.get(), false);
                }
                if (interpreter::Context::PropertyAccessor* dirContext = plugContext->lookup("DIRECTORY", index)) {
                    tmp.reset(dirContext->get(index));
                    result.directory = interpreter::toString(tmp.get(), false);
                }
                break;
            }
        }

        return result;
    }

    /*
     *  Save & Exit
     */

    class PostSaveStateChangeAction : public util::Request<Control> {
     public:
        PostSaveStateChangeAction(OutputState::Target target, RequestLink2 link)
            : m_target(target),
              m_link(link)
            { }
        void handle(Control& ctl)
            { StateChangeTask(m_target).handle(ctl, m_link); }
     private:
        OutputState::Target m_target;
        RequestLink2 m_link;
    };

    class PostSaveAction : public PostSaveTask_t {
     public:
        PostSaveAction(game::Session& session, ScriptSide& si, RequestLink1 link, OutputState::Target target)
            : m_session(session), m_scriptSide(si), m_link(link), m_target(target)
            { }

        virtual void call(bool flag)
            {
                // Ignore flag for now - failure to save does not prevent exiting
                m_session.log().write(afl::sys::LogListener::Trace, LOG_NAME, afl::string::Format("Task: PostSaveAction(%d)", int(flag)));

                // Save VM.
                // TODO: check whether this should be after saving?
                try {
                    if (m_session.getGame().get() != 0)  {
                        game::interface::saveVM(m_session, m_session.getGame()->getViewpointPlayer());
                    }
                }
                catch (std::exception& e) {
                    m_session.log().write(afl::sys::LogListener::Error, LOG_NAME, m_session.translator()("Unable to save game"), e);
                }

                // Perform state change. This will eventually continue the process.
                m_scriptSide.callAsyncNew(new PostSaveStateChangeAction(m_target, m_link));
            }

        static std::auto_ptr<PostSaveTask_t> make(game::Session& session, ScriptSide& si, RequestLink1 link, OutputState::Target target)
            { return std::auto_ptr<PostSaveTask_t>(new PostSaveAction(session, si, link, target)); }

     private:
        game::Session& m_session;
        ScriptSide& m_scriptSide;
        RequestLink1 m_link;
        OutputState::Target m_target;
    };

    class NullSaveAction : public Task_t {
     public:
        NullSaveAction(std::auto_ptr<PostSaveTask_t> then)
            : m_then(then)
            { }
        virtual void call()
            { m_then->call(true); }
     private:
        std::auto_ptr<PostSaveTask_t> m_then;
    };

    void trySaveSession(game::Session& session, ScriptSide& si, RequestLink1 link, OutputState::Target target)
    {
        // TODO: for now, always saves a final turn. Should make some UI!
        std::auto_ptr<Task_t> action(session.save(game::TurnLoader::SaveOptions_t(), PostSaveAction::make(session, si, link, target)));
        if (action.get() == 0) {
            action.reset(new NullSaveAction(PostSaveAction::make(session, si, link, target)));
        }
        link.getProcess().suspend(action);
    }

    void doConfiguredTransfer(ScriptSide& si, RequestLink1 link, game::actions::CargoTransferSetup setup)
    {
        class DialogTask : public UserTask {
         public:
            DialogTask(game::actions::CargoTransferSetup setup)
                : m_setup(setup)
                { }
            virtual void handle(Control& ctl, RequestLink2 link)
                {
                    UserSide& iface = ctl.interface();
                    client::doCargoTransfer(ctl.root(), iface.gameSender(), ctl.translator(), m_setup);
                    iface.continueProcess(link);
                }
         private:
            game::actions::CargoTransferSetup m_setup;
        };
        if (!setup.isValid()) {
            throw game::Exception(game::Exception::ePerm);
        }
        si.postNewTask(link, new DialogTask(setup));
    }

    game::Reference getCurrentShipOrPlanetReference(const game::map::Object* obj)
    {
        if (const game::map::Planet* pl = dynamic_cast<const game::map::Planet*>(obj)) {
            return game::Reference(game::Reference::Planet, pl->getId());
        } else if (const game::map::Ship* sh = dynamic_cast<const game::map::Ship*>(obj)) {
            return game::Reference(game::Reference::Ship, sh->getId());
        } else {
            return game::Reference();
        }
    }

    /* Common part of UI.ChooseObject commands */
    void doStandardObjectSelection(const client::dialogs::ObjectSelectionDialog& def, game::Session& session, client::si::ScriptSide& si, client::si::RequestLink1 link)
    {
        class Task : public UserTask {
         public:
            Task(const client::dialogs::ObjectSelectionDialog& def)
                : m_def(def)
                { }
            void handle(Control& ctl, RequestLink2 link)
                {
                    UserSide& iface = ctl.interface();

                    // Perform dialog
                    OutputState out;
                    int n = client::dialogs::doObjectSelectionDialog(m_def, iface, ctl, out);

                    // Produce result for calling process
                    std::auto_ptr<afl::data::Value> result;
                    if (n != 0) {
                        result.reset(interpreter::makeIntegerValue(n));
                    }
                    iface.setVariable(link, "UI.RESULT", result);

                    // Finish
                    // - if out has no process, joinProcess() is a no-op
                    // - if out has no target, handleStateChange() will just resume
                    iface.joinProcess(link, out.getProcess());
                    ctl.handleStateChange(link, out.getTarget());
                }
         private:
            const client::dialogs::ObjectSelectionDialog& m_def;
        };

        if (game::Game* g = session.getGame().get()) {
            session.notifyListeners();
            game::map::ObjectCursor* c = g->cursors().getCursorByNumber(def.screenNumber);
            if (c == 0 || c->getCurrentIndex() == 0) {
                // No ship selected means no ship present; clear UI.Result and show a message
                link.getProcess().setVariable("UI.RESULT", 0);
                si.postNewTask(link, new MessageBoxTask(util::rich::Parser::parseXml(session.translator().translateString(def.failMessageUT)),
                                                        session.translator().translateString(def.titleUT)));
            } else {
                game::map::ObjectType* ty = c->getObjectType();
                if (ty != 0 && ty->isUnit()) {
                    // We have only one of this kind. Do not show a dialog; directly select it.
                    std::auto_ptr<afl::data::Value> result(interpreter::makeIntegerValue(c->getCurrentIndex()));
                    link.getProcess().setVariable("UI.RESULT", result.get());
                } else {
                    // Regular task
                    si.postNewTask(link, new Task(def));
                }
            }
        } else {
            throw game::Exception(game::Exception::eUser);
        }
    }

    /* Part of UI.ChooseObject command */
    void doHistoryShipSelection(game::Session& session, client::si::ScriptSide& si, client::si::RequestLink1 link)
    {
        using game::ref::HistoryShipSelection;

        // Task
        class Task : public UserTask {
         public:
            Task(HistoryShipSelection sel, HistoryShipSelection::Modes_t modes)
                : m_sel(sel), m_modes(modes)
                { }
            void handle(Control& ctl, RequestLink2 link)
                {
                    UserSide& iface = ctl.interface();
                    int n = client::dialogs::chooseHistoryShip(m_sel, m_modes, ctl.root(), ctl.translator(), iface.gameSender());

                    // Produce result for calling process
                    std::auto_ptr<afl::data::Value> result;
                    if (n != 0) {
                        result.reset(interpreter::makeIntegerValue(n));
                    }
                    iface.setVariable(link, "UI.RESULT", result);
                    iface.continueProcess(link);
                }

         private:
            HistoryShipSelection m_sel;
            HistoryShipSelection::Modes_t m_modes;
        };

        // Prepare initial position.
        // Take over a position when there is a user-perceived position.
        // iuiChartX/iuiChartY are valid on every control screen, plus on player screen.
        // Therefore, check screen number as well.
        HistoryShipSelection sel;

        std::auto_ptr<afl::data::Value> vx(session.uiPropertyStack().get(game::interface::iuiChartX));
        std::auto_ptr<afl::data::Value> vy(session.uiPropertyStack().get(game::interface::iuiChartY));
        std::auto_ptr<afl::data::Value> vs(session.uiPropertyStack().get(game::interface::iuiScreenNumber));
        int x, y, screen;
        bool hasPosition = false;
        if (interpreter::checkIntegerArg(x, vx.get()) && interpreter::checkIntegerArg(y, vy.get()) && interpreter::checkIntegerArg(screen, vs.get()) && screen > 0) {
            sel.setPosition(game::map::Point(x, y));
            hasPosition = true;
        }

        // Prepare initial mode
        const game::map::Universe& univ = game::actions::mustHaveGame(session).currentTurn().universe();
        const game::TeamSettings& teams = game::actions::mustHaveGame(session).teamSettings();
        HistoryShipSelection::Modes_t modes = sel.getAvailableModes(univ, teams);
        if (modes.empty() || (hasPosition && !modes.contains(HistoryShipSelection::LocalShips) && !modes.contains(HistoryShipSelection::ExactShips))) {
            // No valid modes means we have no applicable ships.
            // When we have a position, we want a location-based mode first.
            link.getProcess().setVariable("UI.RESULT", 0);
        } else {
            // Normal operation
            sel.setMode(sel.getInitialMode(univ, teams));
            si.postNewTask(link, new Task(sel, modes));
        }
    }
}

/* @q LoadResource name:Str (Global Command)
   Load a resource.
   You can specify all items you can also specify in <tt>cc-res.cfg</tt>.
   - name of a *.res file (optionally prefixed by "res:")
   - "wp:" followed by the name of a Winplan "BMP" directory
   - "wpvcr:" followed by the name of a Winplan "WPVCR.DLL" file
   - "dir:" followed by a directory name
   File and directory names should be absolute.

   The command may not report failure as an exception if an invalid resource specification is given.
   This depends on the actual PCC implementation;
   some errors are only reported as console messages while the actual command succeeds.

   Because PCC2 caches loaded resource elements,
   you should call this command as early as possible (in <tt>pcc2init.q</tt>, usually).

   For loading *.res files, also see the chapter on <a href="int:statement:plugins">plugins</a>;
   loading a *.res file using the <tt>ResourceFile</tt> plugin directive will also work in PlayVCR
   which has no script interpreter.

   @since PCC 1.0.19, PCC2 1.99.25, PCC2 2.40.1 */
void
client::si::IFLoadResource(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFLoadResource
    args.checkArgumentCount(1);

    // Fetch argument
    String_t resourceName;
    if (!interpreter::checkStringArg(resourceName, args.getNext())) {
        return;
    }

    // Check context.
    // We want "LoadResource" statements from a plugin's "Exec" to be registered
    // with the same origin as resources loaded directly.
    // For c2ng, we also use the plugin's base directory.
    PluginContext context = findPluginContext(link);

    // Create the resource.
    // FIXME: We are in a state where we are allowed to do I/O (and where exceptions are allowed to be thrown).
    // This may change when we add resource providers that need UI access.
    // At that time, we might have to defer creation of the provider into the RelayTask or ManagerRequest.
    std::auto_ptr<ui::res::Provider> provider(ui::res::createProvider(resourceName, context.directory, session.world().fileSystem(), session.log(), session.translator()));

    // The Manager task that adds the created provider into the resource manager.
    class ManagerRequest : public util::Request<ui::res::Manager> {
     public:
        ManagerRequest(std::auto_ptr<ui::res::Provider> provider, String_t contextName)
            : m_provider(provider),
              m_contextName(contextName)
            { }
        virtual void handle(ui::res::Manager& mgr)
            { mgr.addNewProvider(m_provider.release(), m_contextName); }
     private:
        std::auto_ptr<ui::res::Provider> m_provider;
        const String_t m_contextName;
    };

    // The relay task. This task's only job is to post the ManagerRequest into the resource manager.
    // We can only do that from a UserTask/UserSide, because we cannot access the manager from the ScriptSide.
    // This also means that LoadResource briefly suspends the process.
    class RelayTask : public UserTask {
     public:
        RelayTask(std::auto_ptr<ui::res::Provider> provider, String_t contextName)
            : m_provider(provider),
              m_contextName(contextName)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();
                if (ui::DefaultResourceProvider* drp = dynamic_cast<ui::DefaultResourceProvider*>(&ctl.root().provider())) {
                    drp->postNewManagerRequest(new ManagerRequest(m_provider, m_contextName), true);
                }
                ui.continueProcess(link);
            }
     private:
        std::auto_ptr<ui::res::Provider> m_provider;
        const String_t m_contextName;
    };
    si.postNewTask(link, new RelayTask(provider, context.name));
}

/* @q LoadHelpFile name:Str (Global Command)
   Load a help file.
   PCC2 help files are files in a custom XML format.
   Help files can be added using plugins (HelpFile= directive), or using this command.

   This command does not verify that the file actually exists;
   if the given name does not refer to a valid help file,
   a console message will be printed, but the command will not fail.

   @since PCC2 2.0.5, PCC2 2.40.5 */
void
client::si::IFLoadHelpFile(game::Session& session, ScriptSide& /*si*/, RequestLink1 link, interpreter::Arguments& args)
{
    args.checkArgumentCount(1);

    // Fetch argument
    String_t arg;
    if (!interpreter::checkStringArg(arg, args.getNext())) {
        return;
    }

    // Check context
    PluginContext context = findPluginContext(link);

    // Add it
    getHelpIndex(session).addFile(session.world().fileSystem().makePathName(context.directory, arg), context.name);
}


/* @q MessageBox text:Str, Optional heading:Str (Global Command)
   Display a message.
   In the graphical interface, displays an "OK" message box.
   In console mode, just prints out a message and continues.

   @change PCC2ng accepts rich text as the message box content.

   @see UI.Message
   @since PCC 1.0.6, PCC2 1.99.9, PCC2 2.40 */
void
client::si::IFMessageBox(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex globint.pas:Global_Messagebox

    // Validate arguments
    args.checkArgumentCount(1, 2);

    game::interface::RichTextValue::Ptr_t pContent;
    String_t heading = session.translator().translateString("Message");
    if (!game::interface::checkRichArg(pContent, args.getNext())) {
        return;
    }
    interpreter::checkStringArg(heading, args.getNext());

    // Do it
    session.notifyListeners();
    si.postNewTask(link, new MessageBoxTask(pContent, heading));
}

/* @q System.ExitClient (Global Command)
   Leave PCC2.
   Saves the game and closes the program.
   This command will also terminate the current process (as if the {End} command had been used).
   @since PCC2 1.99.26, PCC2 2.40 */
void
client::si::IFSystemExitClient(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFSystemExitClient
    args.checkArgumentCount(0);
    trySaveSession(session, si, link, OutputState::ExitProgram);
}

/* @q System.ExitRace (Global Command)
   Leave current race.
   Saves the game and returns to the <a href="pcc2:gamesel">game selection</a> menu.
   @since PCC2 1.99.10, PCC2 2.40 */
void
client::si::IFSystemExitRace(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFSystemExitRace
    args.checkArgumentCount(0);
    trySaveSession(session, si, link, OutputState::ExitGame);
}

// @since PCC2 2.40.10
void
client::si::IFCCAddToSim(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCAddToSim
    bool ask = true;
    args.checkArgumentCount(0, 1);
    interpreter::checkBooleanArg(ask, args.getNext());
    session.notifyListeners();

    class Task : public UserTask {
     public:
        Task(game::Reference ref, bool ask)
            : m_reference(ref), m_ask(ask)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::addObjectToSimulation(ctl.root(), iface.gameSender(), m_reference, m_ask, ctl.translator());
                iface.continueProcess(link);
            }
     private:
        game::Reference m_reference;
        bool m_ask;
    };

    game::Reference ref = getCurrentShipOrPlanetReference(link.getProcess().getCurrentObject());
    if (ref.isSet()) {
        si.postNewTask(link, new Task(ref, ask));
    } else {
        throw interpreter::Error::contextError();
    }
}

// @since PCC2 2.40.11
void
client::si::IFCCBuildAmmo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCBuildAmmo
    // FIXME: add a version for ships
    class Task : public UserTask {
     public:
        Task(game::Id_t pid)
            : m_pid(pid)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                game::proxy::BuildAmmoProxy proxy(iface.gameSender(), ctl.root().engine().dispatcher(), m_pid);
                proxy.setPlanet();
                client::dialogs::doBuildAmmo(ctl.root(), proxy, iface.gameSender(), m_pid, ctl.translator());
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_pid;
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    game::map::Planet* pl = dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject());
    if (pl != 0 && pl->isPlayable(game::map::Object::Playable)) {
        si.postNewTask(link, new Task(pl->getId()));
    } else {
        throw interpreter::Error::contextError();
    }
}

// @since PCC2 2.40.8
void
client::si::IFCCBuildBase(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCBuildBase
    class Task : public UserTask {
     public:
        Task(game::Id_t pid)
            : m_pid(pid)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doBuildStarbaseDialog(ctl.root(), iface.gameSender(), ctl.translator(), m_pid);
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_pid;
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    game::map::Planet* pl = dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject());
    if (pl != 0 && pl->isPlayable(game::map::Object::Playable)) {
        si.postNewTask(link, new Task(pl->getId()));
    } else {
        throw interpreter::Error::contextError();
    }
}

// @since PCC2 2.40.10
void
client::si::IFCCBuildShip(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCBuildShip
    class Task : public UserTask {
     public:
        Task(game::Id_t pid)
            : m_pid(pid)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doBuildShip(ctl.root(), iface.gameSender(), m_pid, ctl.translator());
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_pid;
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    game::map::Planet* pl = dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject());
    if (pl == 0) {
        throw interpreter::Error::contextError();
    }
    game::actions::mustHavePlayedBase(*pl);
    si.postNewTask(link, new Task(pl->getId()));
}

// @since PCC2 2.40.8
void
client::si::IFCCBuildStructures(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    class Task : public UserTask {
     public:
        Task(game::Id_t pid, int page)
            : m_pid(pid), m_page(page)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doBuildStructuresDialog(ctl.root(), iface.gameSender(), ctl.translator(), m_pid, m_page);
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_pid;
        int m_page;
    };

    args.checkArgumentCount(0, 1);
    int32_t page = 0;
    interpreter::checkIntegerArg(page, args.getNext(), 0, 2);

    session.notifyListeners();
    game::actions::mustHaveGame(session);

    game::map::Planet& pl = game::actions::mustExist(dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject()));
    game::actions::mustBePlayed(pl);

    si.postNewTask(link, new Task(pl.getId(), page));
}

// @since PCC2 2.40.8
void
client::si::IFCCBuySupplies(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCBuySupplies
    args.checkArgumentCount(0);

    // Must be our planet
    game::map::Planet* pPlanet = dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject());
    if (pPlanet == 0) {
        throw interpreter::Error::contextError();
    }
    game::actions::mustBePlayed(*pPlanet);

    // Do it
    class DialogTask : public UserTask {
     public:
        DialogTask(game::Id_t id)
            : m_id(id)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doBuySuppliesDialog(ctl.root(), iface.gameSender(), m_id, 0, 0, ctl.translator());
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_id;
    };
    si.postNewTask(link, new DialogTask(pPlanet->getId()));
}


// @since PCC2 2.40.8
void
client::si::IFCCCargoHistory(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCCargoHistory
    args.checkArgumentCount(0);

    // Must be on a ship
    const game::map::Ship* pShip = dynamic_cast<game::map::Ship*>(link.getProcess().getCurrentObject());
    if (pShip == 0) {
        throw interpreter::Error::contextError();
    }
    session.notifyListeners();

    // Do it
    class DialogTask : public UserTask {
     public:
        DialogTask(game::Session& session, const game::map::Ship& ship)
            : m_data()
            {
                int currentTurn = game::actions::mustHaveGame(session).currentTurn().getTurnNumber();
                util::NumberFormatter fmt = game::actions::mustHaveRoot(session).userConfiguration().getNumberFormatter();
                game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);
                afl::string::Translator& tx = session.translator();

                packShipLastKnownCargo(m_data, ship, currentTurn, fmt, shipList, tx);
                packShipMassRanges    (m_data, ship,              fmt, shipList, tx);
            }

        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doCargoHistory(m_data, ctl.root(), ctl.translator());
                iface.continueProcess(link);
            }

     private:
        game::map::ShipCargoInfos_t m_data;
    };
    si.postNewTask(link, new DialogTask(session, *pShip));
}


// @since PCC2 2.40.6
void
client::si::IFCCChangeSpeed(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCChangeSpeed
    class Task : public UserTask {
     public:
        Task(game::Id_t sid)
            : m_sid(sid)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doShipSpeedDialog(m_sid, ctl.root(), ctl.translator(), iface.gameSender());
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_sid;
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    game::map::Ship* sh = dynamic_cast<game::map::Ship*>(link.getProcess().getCurrentObject());
    if (sh != 0 && sh->isPlayable(game::map::Object::Playable)) {
        if (sh->isFleetMember()) {
            throw game::Exception(game::Exception::eFleet);
        } else {
            si.postNewTask(link, new Task(sh->getId()));
        }
    } else {
        throw interpreter::Error::contextError();
    }
}

// @since PCC2 2.40.7
void
client::si::IFCCChangeTaxes(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCChangeTaxes
    class Task : public UserTask {
     public:
        Task(game::Id_t pid)
            : m_pid(pid)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doTaxationDialog(m_pid, afl::base::Nothing, ctl.root(), ctl.translator(), iface.gameSender());
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_pid;
    };

    args.checkArgumentCount(0);
    game::actions::mustHaveGame(session);

    game::map::Planet* pl = dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject());
    if (pl != 0 && pl->isPlayable(game::map::Object::Playable)) {
        si.postNewTask(link, new Task(pl->getId()));
    } else {
        throw interpreter::Error::contextError();
    }
}

// @since PCC2 2.40.6
void
client::si::IFCCChangeTech(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCChangeTech
    class Task : public UserTask {
     public:
        Task(game::Id_t pid)
            : m_pid(pid)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doTechUpgradeDialog(ctl.root(), ctl.translator(), iface.gameSender(), m_pid);
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_pid;
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    game::map::Planet* pl = dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject());
    if (pl != 0 && pl->isPlayable(game::map::Object::Playable) && pl->hasBase()) {
        si.postNewTask(link, new Task(pl->getId()));
    } else {
        throw interpreter::Error::contextError();
    }
}

// @since PCC2 2.40.8
void
client::si::IFCCChangeWaypoint(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCChangeWaypoint
    // FIXME: can we publish doNavigationChart in its entirety and do the post-processing in a script?

    class WaypointTask : public util::Request<game::Session> {
     public:
        WaypointTask(game::Id_t id, game::map::Point pos)
            : m_id(id), m_position(pos)
            { }

        virtual void handle(game::Session& session)
            {
                game::Root& r = game::actions::mustHaveRoot(session);
                game::spec::ShipList& sl = game::actions::mustHaveShipList(session);
                game::Game& g = game::actions::mustHaveGame(session);
                game::map::Universe& univ = g.currentTurn().universe();
                game::map::Ship& sh = game::actions::mustExist(univ.ships().get(m_id));
                game::map::FleetMember fm(univ, sh);

                fm.setWaypoint(m_position, r.hostConfiguration(), sl);

                // Set optimum warp
                // ex setOptimumWarp
                game::map::Point shipPos;
                if (!sh.isHyperdriving(g.shipScores(), sl, r.hostConfiguration()) && sh.getPosition(shipPos) && shipPos != m_position) {
                    // Determine optimum warp factor
                    int speed = getOptimumWarp(univ, sh.getId(), shipPos, m_position, g.shipScores(), sl, r);
                    fm.setWarpFactor(speed, r.hostConfiguration(), sl);
                }
            }

     private:
        game::Id_t m_id;
        game::map::Point m_position;
    };


    class Task : public UserTask {
     public:
        Task(const client::dialogs::NavChartState& in)
            : m_state(in)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                afl::string::Translator& tx = ctl.translator();
                client::dialogs::NavChartResult result;
                client::dialogs::doNavigationChart(result, m_state, iface, ctl.root(), tx);
                switch (result.result) {
                 case client::dialogs::NavChartResult::Location: {
                    Downlink link(ctl.root(), tx);
                    WaypointTask t(m_state.shipId, result.position);
                    link.call(iface.gameSender(), t);
                    break;
                 }
                 case client::dialogs::NavChartResult::Chunnel: {
                    game::proxy::ChunnelProxy proxy(iface.gameSender(), ctl.root().engine().dispatcher());
                    Downlink link(ctl.root(), tx);

                    afl::data::StringList_t status = proxy.setupChunnel(link, m_state.shipId, result.shipId);
                    if (!status.empty()) {
                        String_t msg = tx("Please fix the following problems to make the chunnel work:");
                        for (size_t i = 0; i < status.size(); ++i) {
                            msg += "\n" UTF_BULLET " ";
                            msg += status[i];
                        }
                        ui::dialogs::MessageBox(msg, tx("Chunnel"), ctl.root()).doOkDialog(tx);
                    }
                    break;
                 }
                 case client::dialogs::NavChartResult::Ship:
                 case client::dialogs::NavChartResult::Canceled:
                    break;
                }

                iface.joinProcess(link, result.outputState.getProcess());
                ctl.handleStateChange(link, result.outputState.getTarget());
            }
     private:
        client::dialogs::NavChartState m_state;
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);
    game::Root& root = game::actions::mustHaveRoot(session);
    game::Game& g = game::actions::mustHaveGame(session);

    game::map::Ship* sh = dynamic_cast<game::map::Ship*>(link.getProcess().getCurrentObject());
    game::map::Point pos;
    if (sh != 0 && sh->isPlayable(game::map::Object::Playable) && sh->getPosition(pos)) {
        if (sh->isFleetMember()) {
            throw game::Exception(game::Exception::eFleet);
        } else {
            client::dialogs::NavChartState in;
            in.title = session.translator()("Select Waypoint");
            in.center = pos;
            in.origin = in.center;
            in.target = sh->getWaypoint().orElse(in.origin);
            in.shipId = sh->getId();
            in.hyperjumping = sh->isHyperdriving(g.shipScores(), shipList, root.hostConfiguration());
            in.speed = util::squareInteger(sh->getWarpFactor().orElse(0));
            if (sh->hasSpecialFunction(game::spec::BasicHullFunction::Gravitonic, g.shipScores(), shipList, root.hostConfiguration())) {
                in.speed *= 2;
            }

            in.acceptLocation = true;
            in.acceptShip = false;
            in.acceptChunnel = sh->hasSpecialFunction(game::spec::BasicHullFunction::FirecloudChunnel, g.shipScores(), shipList, root.hostConfiguration())
                || sh->hasSpecialFunction(game::spec::BasicHullFunction::ChunnelSelf, g.shipScores(), shipList, root.hostConfiguration())
                || sh->hasSpecialFunction(game::spec::BasicHullFunction::ChunnelOthers, g.shipScores(), shipList, root.hostConfiguration());

            game::map::ChunnelMission chm;
            game::map::Universe& univ = g.currentTurn().universe(); // FIXME: is this the same where the ship is from?
            if (chm.check(*sh, univ, g.shipScores(), shipList, root)) {
                if (game::map::Ship* mate = univ.ships().get(chm.getTargetId())) {
                    in.chunnelMode = true;

                    game::map::Point matePos;
                    if (mate->getPosition(matePos)) {
                        in.target = univ.config().getSimpleNearestAlias(matePos, pos);
                    }
                }
            }
            si.postNewTask(link, new Task(in));
        }
    } else {
        throw interpreter::Error::contextError();
    }
}

/* @q CC$ChooseInterceptTarget title:Str, Optional flags:Str (Internal)
   Choose intercept target on mini-map.

   Flags can include
   - a ship id: do not allow choosing this ship
   - "F": allow choosing foreign ships (default: only playable)

   The flags parameter is supported since 2.40.12.

   @since PCC2 2.40.8 */
void
client::si::IFCCChooseInterceptTarget(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex WShipArgWindow::init (sort-of)
    class Task : public UserTask {
     public:
        Task(const client::dialogs::NavChartState& in)
            : m_state(in)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                afl::string::Translator& tx = ctl.translator();
                client::dialogs::NavChartResult result;
                client::dialogs::doNavigationChart(result, m_state, iface, ctl.root(), tx);

                std::auto_ptr<afl::data::Value> r;
                if (result.result == client::dialogs::NavChartResult::Ship) {
                    r.reset(interpreter::makeIntegerValue(result.shipId));
                }
                iface.setVariable(link, "UI.RESULT", r);
                iface.joinProcess(link, result.outputState.getProcess());
                ctl.handleStateChange(link, result.outputState.getTarget());
            }
     private:
        client::dialogs::NavChartState m_state;
    };

    // Parameters
    args.checkArgumentCount(1, 2);

    // - Title
    String_t title;
    if (!interpreter::checkStringArg(title, args.getNext())) {
        return;
    }

    // - Flags
    int32_t flags = 0;
    int32_t excludeShip = 0;
    interpreter::checkFlagArg(flags, &excludeShip, args.getNext(), "F");
    enum { AllShipFlag = 1 };

    // Do it
    session.notifyListeners();
    game::actions::mustHaveShipList(session);
    game::actions::mustHaveRoot(session);
    game::actions::mustHaveGame(session);

    game::map::Ship* sh = dynamic_cast<game::map::Ship*>(link.getProcess().getCurrentObject());
    game::map::Point pos;
    if (sh != 0 && sh->isPlayable(game::map::Object::Playable) && sh->getPosition(pos)) {
        client::dialogs::NavChartState in;
        in.title = title;
        in.center = pos;
        in.origin = in.center;
        in.target = sh->getWaypoint().orElse(in.origin);
        in.shipId = sh->getId();
        in.hyperjumping = false;
        in.speed = 0;
        in.acceptLocation = false;
        in.acceptShip = true;
        in.acceptChunnel = false;
        in.excludeShip = excludeShip;
        in.requireOwnShip = (flags & AllShipFlag) == 0;
        si.postNewTask(link, new Task(in));
    } else {
        throw interpreter::Error::contextError();
    }
}

// @since PCC2 2.40.9
void
client::si::IFCCEditCommands(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    class Task : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                OutputState out;
                client::dialogs::editCommands(ctl.root(), iface, out, ctl.translator());

                iface.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }
     private:
        bool m_excludeCurrent;
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    si.postNewTask(link, new Task());
}

// @since PCC2 2.40.10
void
client::si::IFCCGotoCoordinates(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex client/chart/standardmode.cc:doGoTo
    class Task : public UserTask {
     public:
        Task(const game::map::Configuration& config)
            : m_config(config)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                game::map::Point pt;
                if (client::dialogs::doEnterCoordinatesDialog(pt, m_config, ctl.root(), ctl.interface().gameSender(), ctl.translator())) {
                    game::proxy::MapLocationProxy(ctl.interface().gameSender(), ctl.root().engine().dispatcher())
                        .setPosition(pt);
                    ctl.handleStateChange(link, OutputState::Starchart);
                } else {
                    ctl.interface().continueProcess(link);
                }
            }
     private:
        game::map::Configuration m_config;
    };

    args.checkArgumentCount(0);
    game::Game& g = game::actions::mustHaveGame(session);
    game::Turn* t = g.getViewpointTurn().get();
    if (t != 0) {
        si.postNewTask(link, new Task(t->universe().config()));
    }
}

// @since PCC2 2.40.10
void
client::si::IFCCIonStormInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex doIonStormScreen
    args.checkArgumentCount(0);
    game::actions::mustHaveGame(session);

    class Task : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                OutputState out;
                client::dialogs::doIonStormInfoDialog(iface, ctl.root(), ctl.translator(), out);
                iface.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }
    };
    session.notifyListeners();
    si.postNewTask(link, new Task());
}

// @since PCC2 2.40.6
void
client::si::IFCCListScreenHistory(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCListScreenHistory
    class Task : public UserTask {
     public:
        Task(bool excludeCurrent)
            : m_excludeCurrent(excludeCurrent)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                int32_t n = client::dialogs::doScreenHistoryDialog(ctl.root(), ctl.translator(), iface.gameSender(), iface.history(), m_excludeCurrent);
                if (n >= 0) {
                    for (int32_t i = 0; i < n; ++i) {
                        iface.history().rotate();
                    }
                    activateReference(iface.history().pop(), iface, ctl, link);
                } else {
                    iface.continueProcess(link);
                }
            }
     private:
        bool m_excludeCurrent;
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    si.postNewTask(link, new Task(isCurrentScreenRegistered(session)));
}

// @since PCC2 2.40.8
void
client::si::IFCCManageBuildQueue(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    // Focus on planet if possible (but don't fail if not)
    game::Id_t planetId = 0;
    game::map::Planet* pPlanet = dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject());
    if (pPlanet != 0) {
        planetId = pPlanet->getId();
    }

    // Do it
    class Task : public UserTask {
     public:
        Task(game::Id_t planetId)
            : m_planetId(planetId)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                ScreenHistory::Reference ref = client::dialogs::doBuildQueueDialog(m_planetId, ctl.root(), iface.gameSender(), ctl.translator());
                activateReference(ref, iface, ctl, link);
            }
     private:
        game::Id_t m_planetId;
    };
    si.postNewTask(link, new Task(planetId));
}

// @since PCC2 2.40.10
void
client::si::IFCCMinefieldInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // For now, nullary. It would make sense to give this guy a "minefield Id" parameter.
    // FIXME: check whether we have minefields
    args.checkArgumentCount(0);
    game::actions::mustHaveGame(session);

    class Task : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                OutputState out;
                client::dialogs::doMinefieldInfoDialog(iface, ctl.root(), ctl.translator(), out);
                iface.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }
    };
    session.notifyListeners();
    si.postNewTask(link, new Task());
}

// @since PCC2 2.40.6
void
client::si::IFCCPopScreenHistory(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCPopScreenHistory
    class Task : public UserTask {
     public:
        Task(bool excludeCurrent)
            : m_excludeCurrent(excludeCurrent)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                if (m_excludeCurrent) {
                    iface.history().rotate();
                }
                activateReference(iface.history().pop(), iface, ctl, link);
            }
     private:
        bool m_excludeCurrent;
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    si.postNewTask(link, new Task(isCurrentScreenRegistered(session)));
}

// @since PCC2 2.40.9
void
client::si::IFCCProcessManager(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);

    class Task : public UserTask {
     public:
        Task(game::Reference ref)
            : m_ref(ref)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::si::OutputState out;
                client::dialogs::doProcessListDialog(m_ref, iface, ctl, out);

                iface.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }
     private:
        game::Reference m_ref;
    };

    // FIXME: put a sensible value into game::Reference()
    si.postNewTask(link, new Task(game::Reference()));
}

/* @q CC$Reset x:Int, y:Int (Internal)
   Reset location dialog.
   @since PCC2 2.40.9 */
void
client::si::IFCCReset(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCReset
    args.checkArgumentCount(2);

    // Fetch location
    int32_t x, y;
    if (!interpreter::checkIntegerArg(x, args.getNext()) || !interpreter::checkIntegerArg(y, args.getNext())) {
        return;
    }

    // ReverterProxy will validate further preconditions
    class Task : public UserTask {
     public:
        Task(game::map::Point pos)
            : m_pos(pos)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doRevertLocation(ctl.root(), iface.gameSender(), ctl.translator(), m_pos);
                iface.continueProcess(link);
            }
     private:
        game::map::Point m_pos;
    };
    si.postNewTask(link, new Task(game::map::Point(x, y)));
}

// @since PCC2 2.40.8
void
client::si::IFCCSellSupplies(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCSellSupplies
    args.checkArgumentCount(0);

    // Must be our planet
    game::map::Planet* pPlanet = dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject());
    if (pPlanet == 0) {
        throw interpreter::Error::contextError();
    }
    game::actions::mustBePlayed(*pPlanet);

    // Do it
    class DialogTask : public UserTask {
     public:
        DialogTask(game::Id_t id)
            : m_id(id)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doSellSuppliesDialog(ctl.root(), iface.gameSender(), m_id, 0, 0, ctl.translator());
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_id;
    };
    si.postNewTask(link, new DialogTask(pPlanet->getId()));
}

// @since PCC2 2.40.11
void
client::si::IFCCSendMessage(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    class DialogTask : public UserTask {
     public:
        DialogTask(int viewpointPlayer, bool hasMessages)
            : m_viewpointPlayer(viewpointPlayer), m_hasMessages(hasMessages)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                // ex chooseNewMessageReceiver (sort-of)
                afl::string::Translator& tx = ctl.translator();
                ui::Root& root = ctl.root();

                // Initialize data (this could have already been done on the script side?)
                game::proxy::PlayerProxy proxy(ctl.interface().gameSender());
                Downlink ind(root, tx);

                game::PlayerArray<String_t> names = proxy.getPlayerNames(ind, game::Player::ShortName);
                game::PlayerSet_t players = proxy.getAllPlayers(ind);

                // Widget
                client::widgets::HelpWidget help(root, tx, ctl.interface().gameSender(), "pcc2:msgout");
                client::widgets::PlayerSetSelector setSelect(ctl.root(), names, players + 0, tx);
                client::dialogs::MessageReceiver dlg(tx("Send Message"), setSelect, ctl.root(), tx);
                dlg.addUniversalToggle(players);
                dlg.addHelp(help);
                if (m_hasMessages) {
                    dlg.addExtra(util::KeyString(tx("Revise...")), 2);
                }
                dlg.pack();
                ctl.root().centerWidget(dlg);
                switch (dlg.run()) {
                 case 1: {
                    // Send
                    game::proxy::OutboxProxy outProxy(ctl.interface().gameSender());
                    client::dialogs::MessageEditor ed(root, outProxy, ctl.interface().gameSender(), tx);
                    ed.setSender(m_viewpointPlayer);
                    ed.setReceivers(setSelect.getSelectedPlayers());
                    if (ed.run()) {
                        outProxy.addMessage(ed.getSender(), ed.getText(), ed.getReceivers());
                    }
                    ctl.interface().continueProcess(link);
                    break;
                 }

                 case 2: {
                    // Revise
                    client::dialogs::OutboxDialog dlg(tx("Revise Messages"), ctl.interface(), root, "pcc2:revise", tx);
                    OutputState out;
                    dlg.run(out, tx("No messages sent so far"));
                    ctl.interface().joinProcess(link, out.getProcess());
                    ctl.handleStateChange(link, out.getTarget());
                    break;
                 }

                 default:
                    // Cancel etc.
                    ctl.interface().continueProcess(link);
                    break;
                }

            }
     private:
        int m_viewpointPlayer;
        bool m_hasMessages;
    };
    game::Game& g = game::actions::mustHaveGame(session);
    si.postNewTask(link, new DialogTask(g.getViewpointPlayer(), g.currentTurn().outbox().getNumMessages() != 0));
}

// @since PCC2 2.40.11
void
client::si::IFCCShipSpec(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);

    // Must be a known ship
    game::map::Ship* pShip = dynamic_cast<game::map::Ship*>(link.getProcess().getCurrentObject());
    if (pShip == 0 || !pShip->getHull().isValid()) {
        throw interpreter::Error::contextError();
    }

    // Show dialog
    class DialogTask : public UserTask {
     public:
        DialogTask(game::Id_t id)
            : m_id(id)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::showHullSpecificationForShip(m_id, ctl.root(), ctl.translator(), iface.gameSender());
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_id;
    };
    si.postNewTask(link, new DialogTask(pShip->getId()));
}

// @since PCC2 2.40.9
void
client::si::IFCCSpecBrowser(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    class DialogTask : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doSpecificationBrowserDialog(ctl.root(), iface.gameSender(), ctl.translator());
                iface.continueProcess(link);
            }
    };
    si.postNewTask(link, new DialogTask());
}

// @since PCC2 2.40.10
void
client::si::IFCCTransferMulti(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCTransferMulti
    args.checkArgumentCount(0, 1);

    // Fleet flag
    bool flag = false;
    interpreter::checkBooleanArg(flag, args.getNext());

    // Must be our ship
    game::map::Ship* pShip = dynamic_cast<game::map::Ship*>(link.getProcess().getCurrentObject());
    if (pShip == 0) {
        throw interpreter::Error::contextError();
    }
    game::actions::mustBePlayed(*pShip);

    // Other preconditions
    game::Game& g = game::actions::mustHaveGame(session);
    game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);

    // Validate fleet request
    if (flag && pShip->getFleetNumber() == 0) {
        throw game::Exception(game::Exception::eNotFleet);
    }

    // Prepare initial MultiTransferSetup object
    game::actions::MultiTransferSetup setup;
    setup.setShipId(pShip->getId());
    setup.setFleetOnly(flag);

    // Task
    class Task : public UserTask {
     public:
        Task(game::actions::MultiTransferSetup setup, const game::map::Universe& univ, const game::spec::ShipList& shipList, afl::string::Translator& tx)
            : m_setup(setup), m_cargoTypes()
            {
                // Pack element types
                // (no need to verify non-emptiness; it won't be empty, and if it were, UI would deal with it.)
                game::ElementTypes_t types = setup.getSupportedElementTypes(univ, shipList);
                for (game::Element::Type i = game::Element::begin(), e = game::Element::end(shipList); i != e; ++i) {
                    if (types.contains(i)) {
                        m_cargoTypes.add(i, game::Element::getName(i, tx, shipList));
                    }
                }
            }

        virtual void handle(Control& ctl, RequestLink2 link)
            {
                // ex askCargoType
                // Choose element type
                afl::string::Translator& tx = ctl.translator();
                ui::widgets::StringListbox box(ctl.root().provider(), ctl.root().colorScheme());
                box.swapItems(m_cargoTypes);

                if (ui::widgets::doStandardDialog(tx("Cargo Transfer"), tx("Transfer which cargo?"), box, true, ctl.root(), tx)) {
                    int32_t key;
                    String_t name;
                    if (box.getStringList().get(box.getCurrentItem(), key, name)) {
                        m_setup.setElementType(static_cast<game::Element::Type>(key));
                        client::dialogs::doMultiTransfer(m_setup, ctl.interface().gameSender(), name, ctl.root(), tx);
                    }
                }

                // Proceed task
                ctl.interface().continueProcess(link);
            }

     private:
        game::actions::MultiTransferSetup m_setup;
        util::StringList m_cargoTypes;
    };

    si.postNewTask(link, new Task(setup, g.currentTurn().universe(), shipList, session.translator()));
}

// @since PCC2 2.40.6
void
client::si::IFCCTransferPlanet(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCTransferPlanet
    bool unload;
    args.checkArgumentCount(1);
    if (!interpreter::checkBooleanArg(unload, args.getNext())) {
        return;
    }

    // Must be our planet
    game::map::Planet* pPlanet = dynamic_cast<game::map::Planet*>(link.getProcess().getCurrentObject());
    if (pPlanet == 0) {
        throw interpreter::Error::contextError();
    }
    game::actions::mustBePlayed(*pPlanet);

    // Do it
    class DialogTask : public UserTask {
     public:
        DialogTask(game::Id_t id, bool unload)
            : m_id(id), m_unload(unload)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                doPlanetCargoTransfer(ctl.root(), iface.gameSender(), ctl.translator(), m_id, m_unload);
                iface.continueProcess(link);
            }
     private:
        game::Id_t m_id;
        bool m_unload;
    };
    si.postNewTask(link, new DialogTask(pPlanet->getId(), unload));
}

// @since PCC2 2.40.6
void
client::si::IFCCTransferShip(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCTransferShip
    // Args:
    //   int: kind of transfer (0=ask, 1=ship, 2=planet)
    //   int: target (planet 0: unload)
    int32_t mode, target;
    args.checkArgumentCount(2);
    if (!interpreter::checkIntegerArg(mode, args.getNext(), 0, 2) || !interpreter::checkIntegerArg(target, args.getNext())) {
        return;
    }

    // Must be our ship
    game::map::Ship* pShip = dynamic_cast<game::map::Ship*>(link.getProcess().getCurrentObject());
    if (pShip == 0) {
        throw interpreter::Error::contextError();
    }
    game::actions::mustBePlayed(*pShip);

    // Parse mode/target
    game::map::Universe& univ = game::actions::mustHaveGame(session).currentTurn().universe();
    switch (mode) {
     case 0:
        // Choose target
        class DialogTask : public UserTask {
         public:
            DialogTask(game::Id_t id)
                : m_id(id)
                { }
            virtual void handle(Control& ctl, RequestLink2 link)
                {
                    UserSide& iface = ctl.interface();
                    doShipCargoTransfer(ctl.root(), iface.gameSender(), ctl.translator(), m_id);
                    iface.continueProcess(link);
                }
         private:
            game::Id_t m_id;
        };
        si.postNewTask(link, new DialogTask(pShip->getId()));
        break;

     case 1:
        // Transfer to ship
        doConfiguredTransfer(si, link, game::actions::CargoTransferSetup::fromShipShip(univ, pShip->getId(), target));
        break;

     case 2:
        // Transfer to planet or jettison
        if (target == 0) {
            doConfiguredTransfer(si, link, game::actions::CargoTransferSetup::fromShipJettison(univ, pShip->getId()));
        } else {
            game::actions::CargoTransferSetup setup = game::actions::CargoTransferSetup::fromPlanetShip(univ, target, pShip->getId());
            setup.swapSides();
            doConfiguredTransfer(si, link, setup);
        }
        break;
    }
}

// @since PCC2 2.40.6
void
client::si::IFCCTransferUnload(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCTransferUnload
    args.checkArgumentCount(0);

    // Must be our ship
    game::map::Ship* pShip = dynamic_cast<game::map::Ship*>(link.getProcess().getCurrentObject());
    if (pShip == 0) {
        throw interpreter::Error::contextError();
    }
    game::actions::mustBePlayed(*pShip);

    // Ship must have a position
    game::map::Point shipPos;
    bool ok = pShip->getPosition(shipPos);
    afl::except::checkAssertion(ok, "pShip->getPosition");

    // Find planet
    game::map::Universe& univ = game::actions::mustHaveGame(session).currentTurn().universe();
    game::Id_t pid = univ.findPlanetAt(shipPos);
    if (pid == 0) {
        throw game::Exception(game::Exception::ePos);
    }

    // Do it
    game::actions::CargoTransferSetup setup = game::actions::CargoTransferSetup::fromPlanetShip(univ, pid, pShip->getId());
    setup.swapSides();
    doConfiguredTransfer(si, link, setup);
}

// @since PCC2 2.40.10
void
client::si::IFCCUfoInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // For now, nullary. It would make sense to give this guy a "Ufo Id" parameter.
    // FIXME: check whether we have Ufos
    args.checkArgumentCount(0);
    game::actions::mustHaveGame(session);

    class Task : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                OutputState out;
                client::dialogs::doUfoInfoDialog(iface, ctl.root(), ctl.translator(), out);
                iface.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }
    };
    session.notifyListeners();
    si.postNewTask(link, new Task());
}

/* @q CC$UseKeymap keymap:Str, prefix:Int (Internal)
   This is the backend to {UseKeymap}.
   It causes the next keystroke to be processed using the %keymap,
   and gives it %prefix as the prefix argument ({UI.Prefix}).
   @since PCC2 1.99.22, PCC2 2.40.10 */
void
client::si::IFCCUseKeymap(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // Read arguments
    args.checkArgumentCount(2);

    // Fetch args
    // - keymap must be given
    const afl::data::Value* value = args.getNext();
    if (value == 0) {
        return;
    }
    const interpreter::KeymapValue* kv = dynamic_cast<const interpreter::KeymapValue*>(value);
    if (kv == 0) {
        throw interpreter::Error::typeError(interpreter::Error::ExpectKeymap);
    }

    // - accept null prefix
    int prefix = 0;
    interpreter::checkIntegerArg(prefix, args.getNext());

    // Hand to user side
    class Task : public UserTask {
     public:
        Task(String_t keymapName, int prefix)
            : m_keymapName(keymapName), m_prefix(prefix)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            { ctl.handleUseKeymap(link, m_keymapName, m_prefix); }
     private:
        String_t m_keymapName;
        int m_prefix;
    };
    session.notifyListeners();
    si.postNewTask(link, new Task(kv->getKeymap()->getName(), prefix));
}

// @since PCC2 2.40.5
void
client::si::IFCCViewCombat(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    /* @q CCUI$CurrentVCR:Int (Internal Variable)
       Zero-based index of current VCR.
       @since PCC2 2.40.4 */
    static const char*const INDEX_VAR_NAME = "CCUI$CURRENTVCR";

    args.checkArgumentCount(0);
    session.notifyListeners();

    // Verify that we have all components. If we don't, Adaptor/VcrDatabaseProxy will run in totally degraded mode,
    // so it's better to prevent this.
    game::Game& g = game::actions::mustHaveGame(session);
    game::actions::mustHaveRoot(session);
    game::actions::mustHaveShipList(session);

    // Likewise, there needs to be a VCR database. Not having one is not an error, though.
    game::vcr::Database* db = g.currentTurn().getBattles().get();
    if (db == 0) {
        return;
    }

    // Adaptor for VcrDatabaseProxy
    class Adaptor : public game::proxy::VcrDatabaseAdaptor {
     public:
        Adaptor(game::Session& session)
            : m_session(session)
            { }
        virtual const game::Root& root() const
            { return game::actions::mustHaveRoot(m_session); }
        virtual const game::spec::ShipList& shipList() const
            { return game::actions::mustHaveShipList(m_session); }
        virtual const game::TeamSettings* getTeamSettings() const
            { return &game::actions::mustHaveGame(m_session).teamSettings(); }
        virtual game::vcr::Database& battles()
            {
                game::vcr::Database* db = game::actions::mustHaveGame(m_session).currentTurn().getBattles().get();
                afl::except::checkAssertion(db != 0, "VCR db present");
                return *db;
            }
        virtual afl::string::Translator& translator()
            { return m_session.translator(); }
        virtual afl::sys::LogListener& log()
            { return m_session.log(); }
        virtual size_t getCurrentBattle() const
            {
                try {
                    int32_t i;
                    if (interpreter::checkIntegerArg(i, m_session.world().getGlobalValue(INDEX_VAR_NAME))) {
                        return static_cast<size_t>(i);
                    }
                }
                catch (...)
                { }
                return 0;
            }

        virtual void setCurrentBattle(size_t n)
            { m_session.world().setNewGlobalValue(INDEX_VAR_NAME, interpreter::makeIntegerValue(int32_t(n))); }
        virtual game::sim::Setup* getSimulationSetup() const
            { return &game::sim::getSimulatorSession(m_session)->setup(); }
        virtual bool isGameObject(const game::vcr::Object& obj) const
            {
                game::Game* g = m_session.getGame().get();
                return g != 0 && g->isGameObject(obj, shipList().hulls());
            }
     private:
        game::Session& m_session;
    };

    class AdaptorFromSession : public afl::base::Closure<game::proxy::VcrDatabaseAdaptor*(game::Session&)> {
     public:
        virtual Adaptor* call(game::Session& session)
            { return new Adaptor(session); }
    };

    class JoiningControl : public Control {
     public:
        JoiningControl(Control& parent, RequestLink2 link)
            : Control(parent.interface()),
              m_parent(parent),
              m_link(link)
            { }
        virtual void handleStateChange(RequestLink2 link, OutputState::Target target)
            {
                if (target == OutputState::NoChange) {
                    interface().continueProcess(link);
                } else {
                    interface().detachProcess(link);
                    interface().joinProcess(m_link, link);
                    m_parent.handleStateChange(m_link, target);
                }
            }
        virtual void handleEndDialog(RequestLink2 link, int /*code*/)
            { interface().continueProcess(link); }
        virtual void handlePopupConsole(RequestLink2 link)
            { interface().continueProcess(link); }
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link)
            { defaultHandleScanKeyboardMode(link); }
        virtual void handleSetView(RequestLink2 link, String_t name, bool withKeymap)
            { defaultHandleSetView(link, name, withKeymap); }
        virtual void handleUseKeymap(RequestLink2 link, String_t name, int prefix)
            { defaultHandleUseKeymap(link, name, prefix); }
        void handleOverlayMessage(RequestLink2 link, String_t text)
            { defaultHandleOverlayMessage(link, text); }
        virtual ContextProvider* createContextProvider()
            { return 0; }
     private:
        Control& m_parent;
        RequestLink2 m_link;
    };

    class CombatTask : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                client::si::UserSide& us = ctl.interface();
                game::Reference ref = client::dialogs::playCombat(ctl.root(), ctl.translator(), us.gameSender().makeTemporary(new AdaptorFromSession()), us.gameSender(), us.mainLog());
                if (ref.isSet()) {
                    // Re-using the existing executeGoToReferenceWait function requires use of a Control,
                    // and will produce a potential second process that we need to join with ours.
                    JoiningControl(ctl, link).executeGoToReferenceWait("(Battle Simulator)", ref);
                }
                us.continueProcess(link);
            }
    };

    si.postNewTask(link, new CombatTask());
}

// @since PCC2 2.40.5
void
client::si::IFCCViewInbox(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCViewInbox
    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);
    si.postNewTask(link, new ViewMailboxTask(game::proxy::makeInboxAdaptor()));
}

// @since PCC2 2.40.10
void
client::si::IFCCViewMessages(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFCCViewMessages
    args.checkArgumentCount(0);
    session.notifyListeners();
    game::actions::mustHaveGame(session);

    std::auto_ptr<game::proxy::InboxAdaptor_t> p;
    const game::map::Object* obj = link.getProcess().getCurrentObject();
    if (dynamic_cast<const game::map::Ship*>(obj) != 0) {
        p.reset(game::proxy::makeShipInboxAdaptor(obj->getId()));
    } else if (dynamic_cast<const game::map::Planet*>(obj) != 0) {
        p.reset(game::proxy::makePlanetInboxAdaptor(obj->getId()));
    } else {
        throw interpreter::Error::contextError();
    }

    si.postNewTask(link, new ViewMailboxTask(p.release()));
}

// @since PCC2 2.40.10
void
client::si::IFCCViewNotifications(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    class Task : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();

                // ProcessListProxy to collect updates
                Downlink ind(ctl.root(), ctl.translator());
                game::proxy::ProcessListProxy plProxy(iface.gameSender(), ctl.root().engine().dispatcher());

                // Actual dialog
                OutputState out;
                client::dialogs::showNotifications(afl::base::Nothing, plProxy, iface, ctl.root(), ctl.translator(), out);

                // Collect updates
                uint32_t pgid = plProxy.commit(ind);

                // Join process created by notification dialog into ours
                // (i.e., if notification dialog started a script, that runs after the script which invoked this command.)
                iface.joinProcess(link, out.getProcess());

                // Join process group created by ProcessListProxy into ours
                // (i.e. resumed processes run after this script.)
                iface.joinProcessGroup(link, pgid);

                // Proceed in UI (i.e. resume the process group).
                ctl.handleStateChange(link, out.getTarget());
            }
    };

    args.checkArgumentCount(0);
    session.notifyListeners();
    si.postNewTask(link, new Task());
}


/* @q Chart.SetView name:Str (Global Command)
   Set current view in starchart.
   This determines the visible panels and active keymaps.
   @since PCC2 1.99.10, PCC2 2.40.6 */
void
client::si::IFChartSetView(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFChartSetView
    // Parse args
    String_t name;
    args.checkArgumentCount(1);
    if (!interpreter::checkStringArg(name, args.getNext())) {
        return;
    }
    name = afl::string::strUCase(name);

    // Do we have a keymap named like this?
    bool hasKeymap = (session.world().keymaps().getKeymapByName(name) != 0);

    class Task : public UserTask {
     public:
        Task(const String_t& name, bool hasKeymap)
            : m_name(name), m_hasKeymap(hasKeymap)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            { ctl.handleSetView(link, m_name, m_hasKeymap); }
     private:
        String_t m_name;
        bool m_hasKeymap;
    };
    si.postNewTask(link, new Task(name, hasKeymap));
}

/* @q UI.BattleSimulator (Global Command)
   Open battle simulator.
   @since PCC2 2.40.10 */
void
client::si::IFUIBattleSimulator(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    game::actions::mustHaveGame(session);

    class Task : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();
                OutputState out;
                client::dialogs::doBattleSimulator(ui, ctl, out);
                ui.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }
    };
    session.notifyListeners();
    si.postNewTask(link, new Task());
}

/* @q UI.ChooseObject screen:Int (Global Command)
   Choose game object.
   You specify a screen number to choose the object for:
   <table>
    <tr><td width="4">1, 11</td><td width="10">Own starships</td></tr>
    <tr><td width="4">2, 12</td><td width="10">Own planets</td></tr>
    <tr><td width="4">3, 13</td><td width="10">Own starbases</td></tr>
    <tr><td width="4">6</td>    <td width="10">History starships</td></tr>
    <tr><td width="4">10</td>   <td width="10">Fleets</td></tr>
   </table>

  For example, <tt>UI.ChooseObject 1</tt> does the same as the <kbd>F1</kbd> key in most screens.
  When there is just one ship, no dialog is displayed.

  The chosen object Id is returned in {UI.Result}; the result is
  EMPTY if the user canceled.
  This command does not work in text mode.

  @since PCC 1.1.1, PCC2 1.99.9, PCC2 2.40 */
void
client::si::IFUIChooseObject(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex userint.pas:UserInt_UI_ChooseObject
    // Parse args
    int32_t screen;
    args.checkArgumentCount(1);
    if (!interpreter::checkIntegerArg(screen, args.getNext())) {
        return;
    }

    switch (screen) {
     case 1:
     case 11:
        doStandardObjectSelection(client::dialogs::SHIP_SELECTION_DIALOG, session, si, link);
        break;

     case 2:
     case 12:
        doStandardObjectSelection(client::dialogs::PLANET_SELECTION_DIALOG, session, si, link);
        break;

     case 3:
     case 13:
        doStandardObjectSelection(client::dialogs::BASE_SELECTION_DIALOG, session, si, link);
        break;

     case 6:
        doHistoryShipSelection(session, si, link);
        break;
//      case 10:
//         result = chooseFleet();
//         break;

     default:
        throw interpreter::Error::rangeError();
    }
}

/* @q UI.ChooseTurn [delta:Int] (Global Command)
   Choose a turn from the game history.
   You can optionally specify an initial scroll position (e.g. -1 to place the cursor on the previous turn).

   The chosen turn number is returned in {UI.Result}; the result is EMPTY if the user canceled.
   This command does not work in text mode.

   @since PCC2 2.40 */
void
client::si::IFUIChooseTurn(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    class Task : public UserTask {
     public:
        Task(int delta)
            : m_delta(delta)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();

                // Execute dialog
                int n = client::dialogs::TurnListDialog(ctl.root(), ctl.translator(), iface.gameSender(), m_delta).run();

                // Produce result for calling process
                std::auto_ptr<afl::data::Value> result;
                if (n != 0) {
                    result.reset(interpreter::makeIntegerValue(n));
                }
                iface.setVariable(link, "UI.RESULT", result);

                // Finish
                iface.continueProcess(link);
            }
     private:
        const int m_delta;
    };

    // Parse args
    int32_t delta = 0;
    args.checkArgumentCount(0, 1);
    interpreter::checkIntegerArg(delta, args.getNext());

    if (session.getGame().get() != 0) {
        // Regular task
        session.notifyListeners();
        si.postNewTask(link, new Task(delta));
    } else {
        throw game::Exception(game::Exception::eUser);
    }
}

/* @q UI.EditAlliances (Global Command)
   Alliance editor dialog.
   Brings up a dialog that allows the user to <a href="pcc2:allies">edit alliances</a>.
   This command takes no further parameters.

   @since PCC2 1.99.23, PCC2 2.40.5 */
void
client::si::IFUIEditAlliances(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    class Task : public UserTask {
     public:
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::AllianceDialog(ctl.root(), iface.gameSender(), ctl.translator())
                    .run(iface.gameSender(), ctl.translator());
                iface.continueProcess(link);
            }
    };

    // Preconditions
    args.checkArgumentCount(0);
    game::actions::mustHaveGame(session);
    session.notifyListeners();                   // ex flushUI

    // Do it
    si.postNewTask(link, new Task());
}

/* @q UI.EditTeams (Global Command)
   Team editor dialog.

   @since PCC2 2.40.10 */
void
client::si::IFUIEditTeams(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    class Task : public UserTask {
     public:
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::editTeams(ctl.root(), iface.gameSender(), ctl.translator());
                iface.continueProcess(link);
            }
    };

    // Preconditions
    args.checkArgumentCount(0);
    game::actions::mustHaveGame(session);
    session.notifyListeners();                   // ex flushUI

    // Do it
    si.postNewTask(link, new Task());
}

/* @q UI.EndDialog Optional code:Int (Global Command)
   Closes the dialog if there currently is one open.
   If there is no dialog, this command is ignored.

   The optional %code parameter specifies the return code for the dialog.
   Typical values are 0=cancel, 1=ok.

   @since PCC2 2.40 */
void
client::si::IFUIEndDialog(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // Read arguments
    int32_t code = 0;
    args.checkArgumentCount(0, 1);
    interpreter::checkIntegerArg(code, args.getNext(), 0, 10000);

    // Do it
    class EndTask : public UserTask {
     public:
        EndTask(int code)
            : m_code(code)
            { }
        void handle(Control& ctl, RequestLink2 link)
            { ctl.handleEndDialog(link, m_code); }
     private:
        int m_code;
    };
    si.postNewTask(link, new EndTask(code));
}

/* @q UI.FileWindow title:Str, wildcard:Str, Optional helpId:Str (Global Command)
   File selection.

   Opens  the "select a file" dialog and lets the user choose a file.
   The %title argument specifies what to show in the window title,
   the %wildcard is a wildcard which specifies the default filter.
   For example, to choose a log file, do
   | UI.FileWindow "Choose Log File", "*.log"

   The optional third argument specifies a help page to use,
   it defaults to the help page for the file window.
   See {UI.Help} for more information.

   When the user hits "OK", this command returns the chosen file in
   {UI.Result}; when the user cancels, UI.Result is set to EMPTY.

   The file dialog uses the variable {UI.Directory} to initialize and store the current directory.

   In text mode, this command gives a simple, no-frills input line ({UI.Input}).

   @c This command also accepts a fourth argument that deals with
   @c predefined file names; details are still undocumented at this place.

   @diff In PCC 1.x, the help Id is an integer. In PCC2, it is a string.
   @since PCC2 1.99.21, PCC 1.0.15, PCC2 2.40.7 */
void
client::si::IFUIFileWindow(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIFileWindow
    // ex userint.pas:UserInt_UI_FileWindow
    // FIXME: fourth argument, select-id (integer) in PCC1
    class Task : public UserTask {
     public:
        Task(const String_t& title, const String_t& pattern, const String_t& helpId, const String_t& dirName)
            : m_title(title),
              m_pattern(pattern),
              m_helpId(helpId),
              m_dirName(dirName)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();
                std::auto_ptr<ui::Widget> pHelp;
                client::dialogs::SessionFileSelectionDialog dlg(ctl.root(), ctl.translator(), ui.gameSender(), m_title);
                dlg.setFolder(m_dirName);
                dlg.setPattern(m_pattern);
                if (!m_helpId.empty()) {
                    pHelp.reset(new client::widgets::HelpWidget(ctl.root(), ctl.translator(), ui.gameSender(), m_helpId));
                    dlg.setHelpWidget(*pHelp);
                }

                bool ok = dlg.run();

                // Set UI.RESULT
                std::auto_ptr<afl::data::Value> value;
                if (ok) {
                    value.reset(interpreter::makeStringValue(dlg.getResult()));
                }
                ui.setVariable(link, "UI.RESULT", value);

                // Update UI.DIRECTORY
                value.reset(interpreter::makeStringValue(dlg.getFolder()));
                ui.setVariable(link, "UI.DIRECTORY", value);

                // Continue
                ui.continueProcess(link);
            }
     private:
        String_t m_title, m_pattern, m_helpId, m_dirName;
    };

    // Parse args
    args.checkArgumentCount(2, 3);

    String_t title, pattern, helpId;
    if (!interpreter::checkStringArg(title, args.getNext()) || !interpreter::checkStringArg(pattern, args.getNext())) {
        return;
    }
    interpreter::checkStringArg(helpId, args.getNext());

    // Get current directory
    String_t dirName = interpreter::toString(link.getProcess().getVariable("UI.DIRECTORY"), false);
    if (dirName.empty()) {
        dirName = session.world().fileSystem().getWorkingDirectoryName();
    }

    si.postNewTask(link, new Task(title, pattern, helpId, dirName));
}


/* @q UI.GotoChart x:Int, y:Int (Global Command)
   Go to starchart.
   This command activates the <a href="pcc2:starchart">starchart</a> at the specified position.
   If the coordinates are out of range, they are corrected.
   To switch to a the starcharts without affecting the current position, use
   | UI.GotoScreen 4

   @see UI.GotoScreen
   @since PCC 1.0.14, PCC2 1.99.10, PCC2 2.40.6 */
void
client::si::IFUIGotoChart(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIGotoChart(IntExecutionContext& exc, IntArgBlock& args), globint.pas:Global_UI_GotoChart

    // Read arguments
    args.checkArgumentCount(2);
    int32_t x, y;
    if (!interpreter::checkIntegerArg(x, args.getNext(), 0, 10000) || !interpreter::checkIntegerArg(y, args.getNext(), 0, 10000)) {
        return;
    }

    // Place cursor
    // FIXME: if X,Y refer to an object, lock onto that instead of X,Y
    game::actions::mustHaveGame(session).cursors().location().set(game::map::Point(x, y));

    // Change screen
    si.postNewTask(link, new StateChangeTask(OutputState::Starchart));
}

/* @q UI.GotoScreen screen:Int, Optional id:Int (Global Command)
   Go to control screen.
   This command activates the specified screen.
   If the %id is specified and valid, shows that object.

   <table>
    <tr><th width="2">Id</th><th width="20" align="left">Screen</th></tr>
    <tr><td>0</td> <td><a href="pcc2:racescreen">Race screen</a>. You can not specify an %id here.</td></tr>
    <tr><td>1</td> <td><a href="pcc2:shipscreen">Ship screen</a>. %id is a ship Id.</td></tr>
    <tr><td>2</td> <td><a href="pcc2:planetscreen">Planet screen</a>. %id is a planet Id.</td></tr>
    <tr><td>3</td> <td><a href="pcc2:basescreen">Starbase screen</a>. %id is a starbase Id.</td></tr>
    <tr><td>4</td> <td><a href="pcc2:starchart">Starchart</a>. You can not specify an %id here. Also see {UI.GotoChart}.</td></tr>
    <tr><td>6</td> <td><a href="pcc2:historyscreen">Starship history</a>. %id is a ship Id.</td></tr>
    <tr><td>10</td><td><a href="pcc2:fleetscreen">Fleet screen</a>. %id is a fleet Id.</td></tr>
    <tr><td>11</td><td><a href="pcc2:shiptaskscreen">Ship auto task screen</a>. %id is a ship Id.</td></tr>
    <tr><td>12</td><td><a href="pcc2:planettaskscreen">Planet auto task screen</a>. %id is a planet Id.</td></tr>
    <tr><td>13</td><td><a href="pcc2:basetaskscreen">Starbase auto task screen</a>. %id is a starbase Id.</td></tr>
   </table>

   Note that this command will have immediate effect.
   It will suspend your script temporarily, switch to the new screen, and resume.

   @see UI.GotoChart
   @since PCC 1.0.14, PCC2 1.99.10, PCC2 2.40 */
void
client::si::IFUIGotoScreen(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIGotoScreen, globint.pas:Global_UI_GotoScreen
    // Read arguments
    args.checkArgumentCount(1, 2);
    int32_t screen = 0;
    int32_t obj = 0;
    if (!interpreter::checkIntegerArg(screen, args.getNext())) {
        return;
    }

    // If second argument is specified but empty, ignore command!
    if (args.getNumArgs() > 0 && !interpreter::checkIntegerArg(obj, args.getNext(), 0, 10000)) { // FIXME: magic number
        return;
    }

    switch (screen) {
     case 0:
        si.postNewTask(link, new StateChangeTask(OutputState::PlayerScreen));
        break;

     case 1:
        enterScreen(game::map::Cursors::ShipScreen, OutputState::ShipScreen, obj, session, si, link);
        break;

     case 2:
        enterScreen(game::map::Cursors::PlanetScreen, OutputState::PlanetScreen, obj, session, si, link);
        break;

     case 3:
        enterScreen(game::map::Cursors::BaseScreen, OutputState::BaseScreen, obj, session, si, link);
        break;

     case 4:
        si.postNewTask(link, new StateChangeTask(OutputState::Starchart));
        break;

     // FIXME: case 10:

     case 11:
        enterScreen(game::map::Cursors::ShipScreen, OutputState::ShipTaskScreen, obj, session, si, link);
        break;

     case 12:
        enterScreen(game::map::Cursors::PlanetScreen, OutputState::PlanetTaskScreen, obj, session, si, link);
        break;

     case 13:
        enterScreen(game::map::Cursors::BaseScreen, OutputState::BaseTaskScreen, obj, session, si, link);
        break;

     default:
        throw interpreter::Error::rangeError();
    }
}

/* @q UI.Help page:Str (Global Command)
   Open help screen.
   The help page name is passed as a parameter.

   - PCC2: Help pages names are strings.
     For example, <tt>UI.Help "int:name:ui.help"</tt> displays this help page.
   - PCC 1.x: Help pages are identified by numbers.

   @diff In PCC 1.x, it is a script error if the page does not exist.
   PCC2 silently displays an error page.

   @since PCC2 1.99.15, PCC 1.0.15, PCC2 2.40.6 */
void
client::si::IFUIHelp(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIHelp
    // ex userint.pas:UserInt_UI_Help
    String_t pageName;
    args.checkArgumentCount(1);
    if (!interpreter::checkStringArg(pageName, args.getNext())) {
        return;
    }

    class Task : public UserTask {
     public:
        Task(const String_t& pageName)
            : m_pageName(pageName)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& iface = ctl.interface();
                client::dialogs::doHelpDialog(ctl.root(), ctl.translator(), iface.gameSender(), m_pageName);
                iface.continueProcess(link);
            }
     private:
        String_t m_pageName;
    };
    session.notifyListeners();
    si.postNewTask(link, new Task(pageName));
}

/* @q UI.Input prompt:Str, Optional title:Str, max:Int, flags:Any, def:Str (Global Command)
   Text input.

   Displays a standard text input dialog.
   All but the first parameter are optional, and have the following meaning:
   - %prompt: the prompt to show in the dialog.
   - %title: the window title. Defaults to the value of %prompt.
   - %length: maximum number of characters to accept, defaults to 255.
   - %flags: some additional flags that affect the behaviour of this dialog (see below).
   - %def: initial contents of text input field.

   The %flags parameter is a string that can contain the following:
   - "n": only accept numeric input (digits).
   - "h": do not accept high-ASCII characters (umlauts, accents, etc.).
   - "p": password input: display stars instead of actual input.
   - "f": draw a frame around the input line.
   - "g": only accept characters from game character set.
   - "m": the width (next item) is specified in ems (default: pixels).
   - a number: width of input line.
  For example, "h450" gives an input line which is 450 pixels wide and does not accept high-ASCII input,
  "30m" displays an input line which is 30 ems wide.
  If you only want to set the width, you can also pass an integer instead of a string.

  The result is returned in {UI.Result}:
  if the user hits <kbd>Enter</kbd>, {UI.Result} contains the input.
  If the user hits <kbd>ESC</kbd>, {UI.Result} will be EMPTY.

  Example: this is the "rename ship" function <kbd>N</kbd> on the ship screen:
  | UI.Input "Enter new name:", "Rename Starship #" & Id, 20, 320, Name
  | SetName UI.Result
  ({SetName (Ship Command)|SetName} will not do anything when passed an EMPTY parameter).

  In text mode, this command makes a simple input line using the %prompt only.
  @since PCC 1.0.9, PCC2 1.99.9, PCC2 2.40 */
void
client::si::IFUIInput(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIInput, globint.pas:Global_UI_Input
    /* UI.Input <prompt>[, <title>, <maxChars>, <flags>, <default>] */
    args.checkArgumentCount(1, 5);

    String_t prompt;
    String_t title;
    String_t defaultText;
    int32_t  maxChars = 255;
    int32_t  flags = 0;
    int32_t  width = 0; // FIXME: ui_root->getExtent().w / 2;

    // Mandatory argument
    if (!interpreter::checkStringArg(prompt, args.getNext())) {
        return;
    }

    // Optional arguments
    title = prompt;
    interpreter::checkStringArg(title, args.getNext());
    interpreter::checkIntegerArg(maxChars, args.getNext(), 0, 32000);
    interpreter::checkFlagArg(flags, &width, args.getNext(), "NHPFGM");
    interpreter::checkStringArg(defaultText, args.getNext());

    // Post command
    class Task : public UserTask {
     public:
        Task(String_t prompt,
             String_t title,
             String_t defaultText,
             int32_t maxChars,
             int32_t flags,
             int32_t width)
            : m_prompt(prompt),
              m_title(title),
              m_defaultText(defaultText),
              m_maxChars(maxChars),
              m_flags(flags),
              m_width(width)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();

                // Font
                gfx::FontRequest font;
                font.addSize(1);

                // Convert length
                int width;
                if (m_width == 0) {
                    width = ctl.root().getExtent().getWidth() / 2;
                } else {
                    width = m_width;
                }
                if (m_width == 0 || (m_flags & 32) == 0) {
                    if (int em = ctl.root().provider().getFont(font)->getEmWidth()) {
                        width /= em;
                    }
                }

                // Build a widget
                ui::widgets::InputLine widget(m_maxChars, m_width, ctl.root());
                widget.setFont(font);

                // Convert flags
                //   N = numeric
                //   H = on high ASCII
                //   P = password masking
                //   F = frame
                //   G = game charset
                //   M = width is in ems
                if ((m_flags & 1) != 0) {
                    widget.setFlag(widget.NumbersOnly, true);
                }
                if ((m_flags & 2) != 0) {
                    widget.setFlag(widget.NoHi, true);
                }
                if ((m_flags & 4) != 0) {
                    widget.setFlag(widget.Hidden, true);
                }
                // FIXME: flag 'F' (framed) must be implemented differently
                // if ((m_flags & 8) != 0) {
                //     widget.setFlag(widget.Framed, true);
                // }
                if ((m_flags & 16) != 0) {
                    widget.setFlag(widget.GameChars, true);
                }

                widget.setText(m_defaultText);
                std::auto_ptr<afl::data::Value> result;
                if (widget.doStandardDialog(m_title, m_prompt, ctl.translator())) {
                    result.reset(interpreter::makeStringValue(widget.getText()));
                }
                ui.setVariable(link, "UI.RESULT", result);
                ui.continueProcess(link);
            }
     private:
        String_t m_prompt;
        String_t m_title;
        String_t m_defaultText;
        int32_t m_maxChars;
        int32_t m_flags;
        int32_t m_width;
    };
    session.notifyListeners();
    si.postNewTask(link, new Task(prompt, title, defaultText, maxChars, flags, width));
}

/* @q UI.InputFCode flags:Any, Optional default:Str (Global Command)
   Ask for friendly code input.
   This uses the regular friendly code input window with a list of friendly code.

   The %flags parameter is a string that can contain the following options:
   - <tt>"S"</tt>: offer ship friendly codes
   - <tt>"P"</tt>: offer planet friendly codes
   - <tt>"B"</tt>: offer starbase friendly codes
   - <tt>"C"</tt>: offer friendly codes for capital ships
   - <tt>"A"</tt>: offer friendly codes for alchemy ships
   - <tt>"D"</tt>: offer default context-dependant selection.
     This examines the current context and offers matching codes.
     That is, when this command is called from a ship, offers matching ship codes.
     All other options are ignored in this case.
   - a number: offer friendly codes available to the specified player.
     Defaults to the currently loaded player if omitted or zero.

   You should specify either "D", or at least one of "S", "B" and "P".

   The optional %default parameter specifies the current value of the friendly code.
   The code starts as empty if this argument is omitted.

   The result will be stored in %UI.Result, as usual for user interface commands.
   If the dialog is canceled, %UI.Result will be EMPTY.

   In text mode, this command yields a simple input line, like this:
   | UI.Input "Friendly Code", "", 3, "h", default

   @diff The "D" flag is supported in PCC2 (and PCC2ng) only.
   @since PCC2 1.99.21, PCC 1.0.17, PCC2 2.40.6 */
void
client::si::IFUIInputFCode(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIInputFCode
    // ex userint.pas:UserInt_UI_InputFCode
    using game::spec::FriendlyCodeList;
    using game::spec::FriendlyCode;

    enum {
        ShipFlag       = 1,
        PlanetFlag     = 2,
        BaseFlag       = 4,
        CapitalFlag    = 8,
        AlchemyFlag    = 16,
        DefaultFlag    = 32
    };

    // Check arguments
    args.checkArgumentCount(1, 2);

    int32_t flags = 0, player = 0;
    if (!interpreter::checkFlagArg(flags, &player, args.getNext(), "SPBCAD")) {
        return;
    }

    String_t current;
    interpreter::checkStringArg(current, args.getNext());

    // Validate
    game::Root& r = game::actions::mustHaveRoot(session);
    game::Game& g = game::actions::mustHaveGame(session);
    game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);
    if (player < 0 || player > game::MAX_PLAYERS) {
        throw interpreter::Error::rangeError();
    }
    if (player == 0) {
        player = g.getViewpointPlayer();
    }

    // Construct a friendly-code list
    std::auto_ptr<game::spec::FriendlyCodeList::Infos_t> list(new game::spec::FriendlyCodeList::Infos_t());
    if ((flags & DefaultFlag) != 0) {
        // Default mode
        game::map::Object* obj = link.getProcess().getCurrentObject();
        if (obj == 0) {
            throw interpreter::Error::contextError();
        }

        FriendlyCodeList(shipList.friendlyCodes(), *obj, g.shipScores(), shipList, r.hostConfiguration())
            .pack(*list, r.playerList());
    } else {
        // Parameterized mode
        // Determine type flags
        FriendlyCode::FlagSet_t typeFlags;
        if ((flags & ShipFlag) != 0) {
            typeFlags += FriendlyCode::ShipCode;
        }
        if ((flags & PlanetFlag) != 0) {
            typeFlags += FriendlyCode::PlanetCode;
        }
        if ((flags & BaseFlag) != 0) {
            typeFlags += FriendlyCode::StarbaseCode;
        }

        // Determine property flags
        FriendlyCode::FlagSet_t propertyFlags;
        FriendlyCode::FlagSet_t propertyMask = FriendlyCode::FlagSet_t() + FriendlyCode::CapitalShipCode + FriendlyCode::AlchemyShipCode;
        if ((flags & CapitalFlag) != 0) {
            propertyFlags += FriendlyCode::CapitalShipCode;
        }
        if ((flags & AlchemyFlag) != 0) {
            propertyFlags += FriendlyCode::AlchemyShipCode;
        }

        // Build filtered list
        FriendlyCodeList filteredList;
        const FriendlyCodeList& originalList = shipList.friendlyCodes();
        for (FriendlyCodeList::Iterator_t gi = originalList.begin(); gi != originalList.end(); ++gi) {
            /* An fcode is accepted if
               - flags have ANY of the TypeFlags required by the code
               - flags have ALL of the PropertyFlags required by the code */
            FriendlyCode::FlagSet_t fcFlags = (*gi)->getFlags();
            if (!(fcFlags & typeFlags).empty()
                && ((fcFlags & propertyMask) - propertyFlags).empty()
                && (!fcFlags.contains(FriendlyCode::RegisteredCode) || r.registrationKey().getStatus() == game::RegistrationKey::Registered)
                && (*gi)->getRaces().contains(player))
            {
                filteredList.addCode(**gi);
            }
        }
        filteredList.sort();
        filteredList.pack(*list, r.playerList());
    }

    // Do it.
    class Task : public UserTask {
     public:
        Task(std::auto_ptr<game::spec::FriendlyCodeList::Infos_t> list, const String_t& current)
            : m_list(list),
              m_current(current)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();
                client::dialogs::FriendlyCodeDialog dlg(ctl.root(), ctl.translator(), ctl.translator()("Change Friendly Code"), *m_list, ui.gameSender());
                dlg.setFriendlyCode(m_current);
                bool ok = dlg.run();

                // Result
                std::auto_ptr<afl::data::Value> result;
                if (ok) {
                    result.reset(interpreter::makeStringValue(dlg.getFriendlyCode()));
                }
                ui.setVariable(link, "UI.RESULT", result);
                ui.continueProcess(link);
            }
     private:
        std::auto_ptr<game::spec::FriendlyCodeList::Infos_t> m_list;
        String_t m_current;
    };
    session.notifyListeners();
    // FIXME: if (exc.checkForBreak()) return;
    si.postNewTask(link, new Task(list, current));
}


/* @q UI.InputNumber title:Str, Optional min:Int, max:Int, current:Int, help:Any, label:Str (Global Command)
   Number input.
   This command prompts for a number, using the standard number input window.

   The parameters are
   - %title: the title/prompt shown in the dialog.
   - %min,%max: the acceptable range of numbers (defaults to 0..10000).
   - %current: the current value (defaults to 0).
   - %help: help page to associate with dialog.
   - %label: prompt, if you want it different from the title (PCC 2.40.8+)
   Only the first parameter is mandatory.

   The result will be returned in {UI.Result}.
   It will be an integer within the requested range, or EMPTY if the user canceled the dialog.

   For example, to change a ship's warp factor, you could use
   | UI.InputNumber "Warp", 0, 9, Speed$
   | SetSpeed UI.Result
   (Note that {SetSpeed (Ship Command)|SetSpeed} is implicitly ignored if its parameter is EMPTY).

   This command currently does not work in text mode.

   @since PCC 1.1.16, PCC2 1.99.9, PCC2 2.40.6 */
void
client::si::IFUIInputNumber(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIInputNumber
    // ex userint.pas:UserInt_UI_InputNumber
    // Check arguments
    args.checkArgumentCount(1, 6);

    String_t title;
    if (!interpreter::checkStringArg(title, args.getNext())) {
        return;
    }

    int32_t min = 0;
    interpreter::checkIntegerArg(min, args.getNext());

    int32_t max = 10000;
    interpreter::checkIntegerArg(max, args.getNext());

    int32_t current = 0;
    interpreter::checkIntegerArg(current, args.getNext());

    String_t help;
    interpreter::checkStringArg(help, args.getNext());

    String_t prompt = title;
    interpreter::checkStringArg(prompt, args.getNext());

    // Adjust arguments
    if (max < min) {
        std::swap(min, max);
    }

    // Do it.
    class Task : public UserTask {
     public:
        Task(const String_t& title, int32_t min, int32_t max, int32_t current, const String_t& help, const String_t& prompt)
            : m_title(title), m_min(min), m_max(max), m_current(current), m_help(help), m_prompt(prompt)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();

                // Set up
                afl::base::Observable<int32_t> value;
                ui::widgets::DecimalSelector sel(ctl.root(), ctl.translator(), value, m_min, m_max, 10);
                sel.setValue(m_current);

                // Dialog
                // FIXME: honor 'm_help'
                afl::base::Deleter del;
                bool ok = ui::widgets::doStandardDialog(m_title, m_prompt, sel.addButtons(del, ctl.root()), false, ctl.root(), ctl.translator());

                // Result
                std::auto_ptr<afl::data::Value> result;
                if (ok) {
                    result.reset(interpreter::makeIntegerValue(sel.getValue()));
                }
                ui.setVariable(link, "UI.RESULT", result);
                ui.continueProcess(link);
            }
     private:
        String_t m_title;
        int32_t m_min;
        int32_t m_max;
        int32_t m_current;
        String_t m_help;
        String_t m_prompt;
    };
    session.notifyListeners();
    // FIXME: if (exc.checkForBreak()) return;
    si.postNewTask(link, new Task(title, min, max, current, help, prompt));
}

/* @q UI.KeymapInfo [name:Str] (Global Command)
   Open <a href="pcc2:keymap">keymap debugger</a>.
   If the name is specified, it is the name of the keymap to display.
   Otherwise, displays the keymap of the current screen.
   @see UI.Keymap
   @since PCC2 1.99.10, PCC2 2.40.6 */
void
client::si::IFUIKeymapInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIKeymapInfo
    class Task : public UserTask {
     public:
        Task(const String_t& keymapName)
            : m_keymapName(keymapName)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();
                client::dialogs::doKeymapDialog(ctl.root(), ctl.translator(), ctl.interface().gameSender(), m_keymapName);
                ui.continueProcess(link);
            }
     private:
        String_t m_keymapName;
    };

    args.checkArgumentCount(0, 1);

    String_t keymapName;
    if (!interpreter::checkStringArg(keymapName, args.getNext())) {
        std::auto_ptr<afl::data::Value> screenKeymapName(session.uiPropertyStack().get(game::interface::iuiKeymap));
        keymapName = interpreter::toString(screenKeymapName.get(), false);
    }

    if (!keymapName.empty()) {
        session.notifyListeners();
        si.postNewTask(link, new Task(keymapName));
    }
}

/* @q UI.ListShipPrediction x:Int, y:Int, Optional sid:Int, ok:Str, heading:Str (Global Command)
   List ship prediction (visual scanner).

   Computes future positions of all (known) ships and lists all those that will be at %x,%y
   using the <a href="pcc2:listship">Visual Scan</a> window.
   When the %sid parameter is given and refers to a valid ship Id, uses that ship's predicted
   position instead of %x,%y.

   The last three parameters are optional and modify behaviour details.
   The %ok string specifies the name of the "OK" button, it defaults to <tt>"OK"</tt>.
   Likewise, the %heading specifies the window title, it defaults to <tt>"Ship Prediction"</tt>.

   The chosen ship Id (or EMPTY if the user canceled) is returned in {UI.Result}.
   If no ship matches, a dialog is displayed and EMPTY is returned.
   This command can't be used in text mode.

   @since PCC2 1.99.26, PCC2 2.40.9
   @see UI.ChooseObject, UI.ListShips */
void
client::si::IFUIListShipPrediction(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    /* UI.ListShipPrediction x, y[, sid, title, okname] */
    args.checkArgumentCount(2, 5);

    // Read args
    int32_t x, y;
    int32_t fromShip = 0;
    String_t ok = session.translator()("OK");
    String_t heading = session.translator()("Ship Prediction");

    if (!interpreter::checkIntegerArg(x, args.getNext(), 0, 10000)) {
        return;
    }
    if (!interpreter::checkIntegerArg(y, args.getNext(), 0, 10000)) {
        return;
    }
    interpreter::checkIntegerArg(fromShip, args.getNext(), 0, 10000);
    interpreter::checkStringArg(ok, args.getNext());
    interpreter::checkStringArg(heading, args.getNext());

    // Validate
    game::actions::mustHaveGame(session);

    // Post command
    class Task : public UserTask {
     public:
        Task(game::map::Point pos, game::Id_t fromShip, String_t okName, String_t title)
            : m_pos(pos), m_fromShip(fromShip), m_okName(okName), m_title(title)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();

                // Configure dialog
                client::dialogs::VisualScanDialog dialog(ui, ctl.root(), ctl.translator());
                dialog.setTitle(m_title);
                dialog.setOkName(m_okName);
                dialog.setAllowForeignShips(true);
                dialog.setEarlyExit(false);

                game::ref::List::Options_t opts;
                opts += game::ref::List::IncludeForeignShips;

                // Execute dialog
                // @change In c2ng, loadNext() initializes with the current ship and updates the scanner.
                client::Downlink downLink(ctl.root(), ctl.translator());
                game::Reference resultReference;
                if (dialog.loadNext(downLink, m_pos, m_fromShip, opts)) {
                    resultReference = dialog.run();
                }

                // Process result
                std::auto_ptr<afl::data::Value> resultValue;
                if (resultReference.isSet()) {
                    resultValue.reset(interpreter::makeIntegerValue(resultReference.getId()));
                }
                ui.setVariable(link, "UI.RESULT", resultValue);

                // Handle output state
                const OutputState& out = dialog.outputState();
                ui.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }

     private:
        game::map::Point m_pos;
        game::Id_t m_fromShip;
        String_t m_okName;
        String_t m_title;
    };

    session.notifyListeners();
    si.postNewTask(link, new Task(game::map::Point(x, y), fromShip, ok, heading));
}

/* @q UI.ListShips x:Int, y:Int, Optional flags:Any, ok:Str, heading:Str (Global Command)
   List ships (visual scanner).

   Lists all ships at position %x,%y using the <a href="pcc2:listship">Visual Scan</a> window.
   The last three parameters are optional and modify behaviour details.

   The %flags parameter contains a list of flag letters:
   - "f": allow the user to choose foreign ships. If this is not specified,
     the "OK" button will be disabled for foreign ships. This flag implies "A".
   - "a": list all ships at the specified location. By default, only your ships are listed.
   - "e": when there is only one matching ship, return it and do not display the dialog at all.
   - "s": only show ships that we "safely" see, i.e. no guessed ships.
   - a ship Id to exclude. That ship will not be listed.

   The %ok string specifies the name of the "OK" button, it defaults to <tt>"OK"</tt>.
   Likewise, the %heading specifies the window title, it defaults to <tt>"List Ships"</tt>.

   The chosen ship Id (or EMPTY if the user canceled) is returned in {UI.Result}.
   If no ship matches, a dialog is displayed and EMPTY is returned.
   This command can't be used in text mode.

   For example, this command sequence sets a "Tow" mission:
   | UI.ListShips Loc.X, Loc.Y, "fae" & Id, "Choose", "Tow Ship"
   | If UI.Result Then SetMission 7, 0, UI.Result
   This command is equivalent to the <kbd>Ctrl-F1</kbd> key command (switch to ship):
   | UI.ListShips UI.X, UI.Y, "e" & Id
   | If UI.Result Then UI.GotoScreen 1, UI.Result

   @since PCC 1.1.1, PCC2 1.99.10, PCC2 2.0.5
   @see UI.ChooseObject, UI.ListShipPrediction */
void
client::si::IFUIListShips(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex userint.pas:UserInt_UI_ListShips
    /* UI.ListShips x, y[, flags, ok, heading]
       Flags: F = allow selection of foreign ships; implies A
              A = list all ships
              E = do not display a dialog if there is only one ship
              S = only safe ships, no guessed ones */
    args.checkArgumentCount(2, 5);

    // Read args
    int32_t x, y;
    int32_t flags = 0, except = 0;
    String_t ok = session.translator().translateString("OK");
    String_t heading = session.translator().translateString("List Ships");

    if (!interpreter::checkIntegerArg(x, args.getNext(), 0, 10000)) {
        return;
    }
    if (!interpreter::checkIntegerArg(y, args.getNext(), 0, 10000)) {
        return;
    }
    interpreter::checkFlagArg(flags, &except, args.getNext(), "FAES");
    interpreter::checkStringArg(ok, args.getNext());
    interpreter::checkStringArg(heading, args.getNext());

    // Validate
    game::actions::mustHaveGame(session);

    // Post command
    class Task : public UserTask {
     public:
        Task(game::map::Point pos,
             int flags, game::Id_t excludeShip,
             String_t okName, String_t title)
            : m_pos(pos), m_flags(flags), m_excludeShip(excludeShip), m_okName(okName), m_title(title)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();

                // Configure dialog
                client::dialogs::VisualScanDialog dialog(ui, ctl.root(), ctl.translator());
                dialog.setTitle(m_title);
                dialog.setOkName(m_okName);
                dialog.setAllowForeignShips((m_flags & 1) != 0);
                dialog.setEarlyExit((m_flags & 4) != 0);

                game::ref::List::Options_t opts;
                if ((m_flags & 1) != 0 || (m_flags & 2) != 0) {
                    opts += game::ref::List::IncludeForeignShips;
                }
                if ((m_flags & 8) != 0) {
                    opts += game::ref::List::SafeShipsOnly;
                }

                // Execute dialog
                client::Downlink downLink(ctl.root(), ctl.translator());
                game::Reference resultReference;
                if (dialog.loadCurrent(downLink, m_pos, opts, m_excludeShip)) {
                    resultReference = dialog.run();
                }

                // Process result
                std::auto_ptr<afl::data::Value> resultValue;
                if (resultReference.isSet()) {
                    resultValue.reset(interpreter::makeIntegerValue(resultReference.getId()));
                }
                ui.setVariable(link, "UI.RESULT", resultValue);

                // Handle output state
                const OutputState& out = dialog.outputState();
                ui.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }

     private:
        game::map::Point m_pos;
        int m_flags;
        game::Id_t m_excludeShip;
        String_t m_okName;
        String_t m_title;
    };

    session.notifyListeners();
    si.postNewTask(link, new Task(game::map::Point(x, y), flags, except, ok, heading));
}

/* @q UI.Message text:RichText, Optional title:Str, buttons:Str (Global Command)
   Display a message.
   This displays a standard message box with the specified %text and %title,
   and the specified %buttons.
   For example,
   | UI.Message "Choose a color", "Question", "Red Green Blue"
   displays a message box with three buttons: "Red", "Green", and "Blue".

   The buttons can be activated by hitting their first letter.
   In addition, the <kbd>Enter</kbd> key activates the first button, <kbd>ESC</kbd> activates the last one.

   This command returns the index of the pressed button in {UI.Result}.
   For example, if the user chose "Red" above, {UI.Result} will have the value 1 afterwards.

   In text mode, displays text and heading, and a list of first letters
   of the buttons, and waits for a matching keystroke.

   The last two parameters are optional and default to "Message" and "OK".

   This command differs from {MessageBox} in that it modifies {UI.Result}, and waits for a keystroke in text mode.

   @diff PCC 1.x allows up to 10 buttons; PCC2 has no such limit
   (but you are adviced to keep the number of buttons and the length of the texts short anyway).

   @see MessageBox, UI.Input
   @since PCC 1.0.9, PCC2 1.99.9, PCC2 2.40 */
void
client::si::IFUIMessage(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIMessage
    args.checkArgumentCount(1, 3);

    // Read arguments
    game::interface::RichTextValue::Ptr_t pContent;
    String_t title   = session.translator().translateString("Message");
    String_t buttons = session.translator().translateString("OK");
    if (!game::interface::checkRichArg(pContent, args.getNext())) {
        return;
    }
    interpreter::checkStringArg(title, args.getNext());
    interpreter::checkStringArg(buttons, args.getNext());

    class Task : public UserTask {
     public:
        Task(game::interface::RichTextValue::Ptr_t pContent, String_t heading, String_t buttons)
            : m_content(*pContent), m_heading(heading), m_buttons(buttons)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();

                // Build dialog
                ui::dialogs::MessageBox dlg(m_content, m_heading, ctl.root());
                int id = 0;
                do {
                    String_t b = afl::string::strFirst(m_buttons, " ");
                    if (!b.empty()) {
                        ++id;
                        dlg.addButton(id, b, afl::charset::getLowerCase(afl::charset::Utf8().charAt(b, 0)));
                    }
                } while (afl::string::strRemove(m_buttons, " "));
                dlg.addKey(1, util::Key_Return);
                dlg.addKey(1, ' ');
                dlg.addKey(id, util::Key_Escape);

                // Do it
                std::auto_ptr<afl::data::Value> result;
                if (id != 0) {
                    // ...only if we actually got some buttons
                    // FIXME: replacement for flushUI, checkForBreak()?
                    //     flushUI();
                    //     if (exc.checkForBreak())
                    int n = dlg.run();
                    result.reset(interpreter::makeIntegerValue(n));
                }

                // Continue
                ui.setVariable(link, "UI.RESULT", result);
                ui.continueProcess(link);
            }
     private:
        util::rich::Text m_content;
        String_t m_heading;
        String_t m_buttons;
    };
    session.notifyListeners();
    si.postNewTask(link, new Task(pContent, title, afl::string::strTrim(buttons)));
}

/* @q UI.OverlayMessage msg:Str (Global Command)
   Display an overlay message.
   The message is shown centered on the screen, and automatically decays.
   Because it's not a window, the user doesn't have to explicitly confirm it.
   Use this for status updates from scripts that are not otherwise interactive.
   For example, this command is used to report changes of the current selection layer.
   @since PCC2 1.99.10, PCC2 2.40.10 */
void
client::si::IFUIOverlayMessage(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIOverlayMessage
    args.checkArgumentCount(1);

    String_t msg;
    if (!interpreter::checkStringArg(msg, args.getNext()) || msg.empty()) {
        return;
    }

    class Task : public UserTask {
     public:
        Task(const String_t& msg)
            : m_message(msg)
            { }
        virtual void handle(Control& ctl, RequestLink2 link)
            { ctl.handleOverlayMessage(link, m_message); }
     private:
        String_t m_message;
    };
    session.notifyListeners();
    si.postNewTask(link, new Task(msg));
}

/* @q UI.PlanetInfo pid:Int (Global Command)
   Open <a href="pcc2:envscreen">planet information</a> for planet %pid.
   @since PCC2 1.99.10, PCC2 2.40.8 */
void
client::si::IFUIPlanetInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIPlanetInfo
    args.checkArgumentCount(1);

    int32_t pid;
    if (!interpreter::checkIntegerArg(pid, args.getNext())) {
        return;
    }
    if (!game::actions::mustHaveGame(session).currentTurn().universe().planets().get(pid)) {
        throw interpreter::Error::rangeError();
    }

    class Task : public UserTask {
     public:
        Task(game::Id_t id)
            : m_id(id)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();
                client::dialogs::doPlanetInfoDialog(ctl.root(),
                                                    ui.gameSender(),
                                                    m_id,
                                                    ctl.translator());
                ui.continueProcess(link);
            }
     private:
        game::Id_t m_id;
    };
    session.notifyListeners();
    si.postNewTask(link, new Task(pid));
}


/* @q UI.PopupConsole (Global Command)
   Open the <a href="pcc2:console">console</a>.
   The script continues running there.
   If your script is doing interesting output to the console,
   you can call this function to ensure the user sees it, even if he bound it to a key.

   @since PCC 1.1.2, PCC2 1.99.10, PCC2 2.40 */
void
client::si::IFUIPopupConsole(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIPopupConsole, globint.pas:Global_UI_PopupConsole
    args.checkArgumentCount(0);
    si.postNewTask(link, new PopupConsoleTask());
}

/* @q UI.ScanKeyboardMode (Global Command)
   On a control screen, activates movement of the scanner using the keyboard.
   Fails with an error when called from another context.
   @since PCC2 2.40.11 */
void
client::si::IFUIScanKeyboardMode(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);

    class Task : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            { ctl.handleScanKeyboardMode(link); }
    };
    si.postNewTask(link, new Task());
}

/* @q UI.SelectionManager (Global Command)
   Open <a href="pcc2:selectionmgr">selection manager</a>.
   @since PCC2 1.99.10, PCC2 2.40.9 */
void
client::si::IFUISelectionManager(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUISelectionManager
    args.checkArgumentCount(0);
    game::actions::mustHaveGame(session);

    class Task : public UserTask {
     public:
        virtual void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();
                OutputState out;
                client::dialogs::doSelectionManager(ui, ctl, out);
                ui.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }
    };
    session.notifyListeners();
    si.postNewTask(link, new Task());
}

/* @q UI.Search Optional query:Str, flags:Any (Global Command)
   Search.

   When called with no parameters, just opens the <a href="pcc2:search">Search</a> dialog.
   When a search query is present, it is immmediately evaluated.
   The %query parameter is the search string, the %flags specify the kind of search:
   - "P": include planets in search.
   - "S": include ships in search.
   - "B": include starbases in search.
   - "U": include UFOs in search.
   - "O": include the other objects in search.
   - "1": search for name or Id.
   - "2": search for expression which is true (default).
   - "3": search for expression which is false.
   - "4": search for location.
   Briefly, letters correspond to the checklist in the top-left of the search window,
   digits correspond to the selection list in the top-right.
   You can specify any number of letters but only one digit.
   By default, all objects are searched for an expression which is true.

   @since PCC2 1.99.10, PCC 1.1.2, PCC2 2.40.7 */
void
client::si::IFUISearch(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUISearch
    // ex userint.pas:UserInt_UI_Search
    /* UI.Search [text, flags]
       flags: PSBUO = planets, ships, bases, ufos, others
              1234  = name, true, false, location */
    using game::SearchQuery;
    SearchQuery q = game::proxy::SearchProxy::savedQuery(session);

    bool immediate = false;
    args.checkArgumentCount(0, 2);
    if (args.getNumArgs() > 0) {
        // Fetch text
        String_t text;
        immediate = true;
        if (!interpreter::checkStringArg(text, args.getNext())) {
            return;
        }
        q.setQuery(text);
    }
    if (args.getNumArgs() > 0) {
        // Fetch flags
        int32_t kind = 1;
        int32_t objs = 0;
        static_assert(SearchQuery::SearchShips   == 0, "SearchShips");
        static_assert(SearchQuery::SearchPlanets == 1, "SearchPlanets");
        static_assert(SearchQuery::SearchBases   == 2, "SearchBases");
        static_assert(SearchQuery::SearchUfos    == 3, "SearchUfos");
        static_assert(SearchQuery::SearchOthers  == 4, "SearchOthers");

        if (!interpreter::checkFlagArg(objs, &kind, args.getNext(), "SPBUO")) {
            return;
        }

        // Kind
        switch (kind) {
         case 1:  q.setMatchType(SearchQuery::MatchName);     break;
         case 2:  q.setMatchType(SearchQuery::MatchTrue);     break;
         case 3:  q.setMatchType(SearchQuery::MatchFalse);    break;
         case 4:  q.setMatchType(SearchQuery::MatchLocation); break;
         default: throw interpreter::Error::rangeError();
        }

        // Objects
        q.setSearchObjects(SearchQuery::SearchObjects_t::fromInteger(objs));
    }

    class Task : public UserTask {
     public:
        Task(const SearchQuery& q, game::Reference currentObject, bool immediate)
            : m_query(q), m_currentObject(currentObject), m_immediate(immediate)
            { }
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();
                OutputState out;
                client::dialogs::doSearchDialog(m_query, m_currentObject, m_immediate, ui, ctl.root(), ctl.translator(), out);
                ui.joinProcess(link, out.getProcess());
                ctl.handleStateChange(link, out.getTarget());
            }
     private:
        SearchQuery m_query;
        game::Reference m_currentObject;
        bool m_immediate;
    };
    session.notifyListeners();
    si.postNewTask(link, new Task(q, getCurrentShipOrPlanetReference(link.getProcess().getCurrentObject()), immediate));
}

/* @q UI.ShowScores (Global Command)
   Displays the score history.
   @since PCC2 2.40.10 */
void
client::si::IFUIShowScores(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);

    class Task : public UserTask {
     public:
        void handle(Control& ctl, RequestLink2 link)
            {
                UserSide& ui = ctl.interface();
                client::dialogs::showScores(ctl.root(), ui.gameSender(), ctl.translator());
                ui.continueProcess(link);
            }
    };
    session.notifyListeners();
    si.postNewTask(link, new Task());
}


/* @q UI.Update Optional flag:Bool (Global Command)
   Update graphical user interface.
   This causes all the screen to be redrawn.
   With the flag specified as %True, redraws even if there are no changes.

   In console mode, this function does nothing.
   @since PCC 1.0.13, PCC2 1.99.9, PCC2 2.40.1 */
void
client::si::IFUIUpdate(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIUpdate, globint.pas:Global_UI_Update
    /* UI.Update [<forceFullRedrawFlag>] */
    args.checkArgumentCount(0, 1);

    int mode = 0;
    if (args.getNumArgs() > 0) {
        mode = interpreter::getBooleanValue(args.getNext());
    }

    if (mode >= 0) {
        // We need to do two things for redraw:
        // - notifyListeners() to flush out all pending signalisations.
        //   This is enough to redraw components implemented in C++, directly sitting on some signal.
        //   Those will post their updates directly into the UserSide.
        session.notifyListeners();

        // - briefly suspend the currently-running process.
        //   This will give room for components implemented in CCScript to update,
        //   which are triggered by notifyListeners() on the ScriptSide.
        // The UI.Update command also allows to request a blanket redraw, which we can easily do.
        class UpdateTask : public UserTask {
         public:
            UpdateTask(bool flag)
                : m_flag(flag)
                { }
            virtual void handle(Control& ctl, RequestLink2 link)
                {
                    UserSide& ui = ctl.interface();
                    if (m_flag) {
                        ctl.root().requestRedraw();
                    }
                    ui.continueProcess(link);
                }
         private:
            bool m_flag;
        };
        si.postNewTask(link, new UpdateTask(mode > 0));
    }
}

void
client::si::registerCommands(UserSide& ui)
{
    // ex int/if/guiif.cc:initInterpreterGuiInterface
    class Task : public UserSide::ScriptRequest {
     public:
        void handle(ScriptSide& si)
            {
                game::Session& s = si.session();

                // Values
                /* @q System.GUI:Bool (Global Property)
                   Graphical interface flag.
                   True if PCC is running with graphical interface, False if it is running in console mode. */
                s.world().setNewGlobalValue("SYSTEM.GUI", interpreter::makeBooleanValue(true));

                // Procedures
                s.world().setNewGlobalValue("CC$ADDTOSIM",           new ScriptProcedure(s, &si, IFCCAddToSim));
                s.world().setNewGlobalValue("CC$BUILDAMMO",          new ScriptProcedure(s, &si, IFCCBuildAmmo));
                s.world().setNewGlobalValue("CC$BUILDBASE",          new ScriptProcedure(s, &si, IFCCBuildBase));
                s.world().setNewGlobalValue("CC$BUILDSHIP",          new ScriptProcedure(s, &si, IFCCBuildShip));
                s.world().setNewGlobalValue("CC$BUILDSTRUCTURES",    new ScriptProcedure(s, &si, IFCCBuildStructures));
                s.world().setNewGlobalValue("CC$BUYSUPPLIES",        new ScriptProcedure(s, &si, IFCCBuySupplies));
                s.world().setNewGlobalValue("CC$CARGOHISTORY",       new ScriptProcedure(s, &si, IFCCCargoHistory));
                // s.world().setNewGlobalValue("CC$CHANGEMISSION",      new ScriptProcedure(s, &si, IFCCChangeMission));
                // s.world().setNewGlobalValue("CC$CHANGEPE",           new ScriptProcedure(s, &si, IFCCChangePE));
                s.world().setNewGlobalValue("CC$CHANGESPEED",        new ScriptProcedure(s, &si, IFCCChangeSpeed));
                s.world().setNewGlobalValue("CC$CHANGETAXES",        new ScriptProcedure(s, &si, IFCCChangeTaxes));
                s.world().setNewGlobalValue("CC$CHANGETECH",         new ScriptProcedure(s, &si, IFCCChangeTech));
                s.world().setNewGlobalValue("CC$CHANGEWAYPOINT",     new ScriptProcedure(s, &si, IFCCChangeWaypoint));
                s.world().setNewGlobalValue("CC$CHOOSEINTERCEPTTARGET", new ScriptProcedure(s, &si, IFCCChooseInterceptTarget));
                // s.world().setNewGlobalValue("CC$CSBROWSE",           new ScriptProcedure(s, &si, IFCCCSBrowse));
                s.world().setNewGlobalValue("CC$EDITCOMMANDS",       new ScriptProcedure(s, &si, IFCCEditCommands));
                // s.world().setNewGlobalValue("CC$GIVE",               new ScriptProcedure(s, &si, IFCCGive));
                s.world().setNewGlobalValue("CC$GOTOCOORDINATES",    new ScriptProcedure(s, &si, IFCCGotoCoordinates));
                s.world().setNewGlobalValue("CC$IONSTORMINFO",       new ScriptProcedure(s, &si, IFCCIonStormInfo));
                s.world().setNewGlobalValue("CC$LISTSCREENHISTORY",  new ScriptProcedure(s, &si, IFCCListScreenHistory));
                s.world().setNewGlobalValue("CC$MANAGEBUILDQUEUE",   new ScriptProcedure(s, &si, IFCCManageBuildQueue));
                s.world().setNewGlobalValue("CC$MINEFIELDINFO",      new ScriptProcedure(s, &si, IFCCMinefieldInfo));
                s.world().setNewGlobalValue("CC$POPSCREENHISTORY",   new ScriptProcedure(s, &si, IFCCPopScreenHistory));
                s.world().setNewGlobalValue("CC$PROCESSMANAGER",     new ScriptProcedure(s, &si, IFCCProcessManager));
                // s.world().setNewGlobalValue("CC$REMOTECONTROL",      new ScriptProcedure(s, &si, IFCCRemoteControl));
                s.world().setNewGlobalValue("CC$RESET",              new ScriptProcedure(s, &si, IFCCReset));
                s.world().setNewGlobalValue("CC$REMOTEGETCOLOR",     new SimpleFunction(s, IFCCRemoteGetColor));
                s.world().setNewGlobalValue("CC$REMOTEGETQUESTION",  new SimpleFunction(s, IFCCRemoteGetQuestion));
                s.world().setNewGlobalValue("CC$REMOTETOGGLE",       new SimpleProcedure(s, IFCCRemoteToggle));
                s.world().setNewGlobalValue("CC$SELLSUPPLIES",       new ScriptProcedure(s, &si, IFCCSellSupplies));
                s.world().setNewGlobalValue("CC$SENDMESSAGE",        new ScriptProcedure(s, &si, IFCCSendMessage));
                // s.world().setNewGlobalValue("CC$SETTINGS",           new ScriptProcedure(s, &si, IFCCSettings));
                // s.world().setNewGlobalValue("CC$SHIPCOSTCALC",       new ScriptProcedure(s, &si, IFCCShipCostCalc),
                s.world().setNewGlobalValue("CC$SHIPSPEC",           new ScriptProcedure(s, &si, IFCCShipSpec));
                s.world().setNewGlobalValue("CC$SPECBROWSER",        new ScriptProcedure(s, &si, IFCCSpecBrowser));
                // s.world().setNewGlobalValue("CC$TOWFLEETMEMBER",     new ScriptProcedure(s, &si, IFCCTowFleetMember));
                s.world().setNewGlobalValue("CC$TRANSFERMULTI",      new ScriptProcedure(s, &si, IFCCTransferMulti));
                s.world().setNewGlobalValue("CC$TRANSFERPLANET",     new ScriptProcedure(s, &si, IFCCTransferPlanet));
                s.world().setNewGlobalValue("CC$TRANSFERSHIP",       new ScriptProcedure(s, &si, IFCCTransferShip));
                s.world().setNewGlobalValue("CC$TRANSFERUNLOAD",     new ScriptProcedure(s, &si, IFCCTransferUnload));
                s.world().setNewGlobalValue("CC$UFOINFO",            new ScriptProcedure(s, &si, IFCCUfoInfo));
                s.world().setNewGlobalValue("CC$USEKEYMAP",          new ScriptProcedure(s, &si, IFCCUseKeymap));
                s.world().setNewGlobalValue("CC$VIEWCOMBAT",         new ScriptProcedure(s, &si, IFCCViewCombat));
                s.world().setNewGlobalValue("CC$VIEWINBOX",          new ScriptProcedure(s, &si, IFCCViewInbox));
                s.world().setNewGlobalValue("CC$VIEWMESSAGES",       new ScriptProcedure(s, &si, IFCCViewMessages));
                s.world().setNewGlobalValue("CC$VIEWNOTIFICATIONS",  new ScriptProcedure(s, &si, IFCCViewNotifications));
                s.world().setNewGlobalValue("CHART.SETVIEW",         new ScriptProcedure(s, &si, IFChartSetView));
                s.world().setNewGlobalValue("LOADRESOURCE",          new ScriptProcedure(s, &si, IFLoadResource));
                s.world().setNewGlobalValue("LOADHELPFILE",          new ScriptProcedure(s, &si, IFLoadHelpFile));
                s.world().setNewGlobalValue("LISTBOX",               new ListboxFunction(s, &si));
                s.world().setNewGlobalValue("MESSAGEBOX",            new ScriptProcedure(s, &si, IFMessageBox));
                s.world().setNewGlobalValue("SYSTEM.EXITCLIENT",     new ScriptProcedure(s, &si, IFSystemExitClient));
                s.world().setNewGlobalValue("SYSTEM.EXITRACE",       new ScriptProcedure(s, &si, IFSystemExitRace));
                s.world().setNewGlobalValue("UI.BATTLESIMULATOR",    new ScriptProcedure(s, &si, IFUIBattleSimulator));
                s.world().setNewGlobalValue("UI.CHOOSEOBJECT",       new ScriptProcedure(s, &si, IFUIChooseObject));
                s.world().setNewGlobalValue("UI.CHOOSETURN",         new ScriptProcedure(s, &si, IFUIChooseTurn));
                s.world().setNewGlobalValue("UI.EDITALLIANCES",      new ScriptProcedure(s, &si, IFUIEditAlliances));
                s.world().setNewGlobalValue("UI.EDITTEAMS",          new ScriptProcedure(s, &si, IFUIEditTeams));
                s.world().setNewGlobalValue("UI.DIALOG",             new DialogFunction(s, &si));
                s.world().setNewGlobalValue("UI.ENDDIALOG",          new ScriptProcedure(s, &si, IFUIEndDialog));
                s.world().setNewGlobalValue("UI.FILEWINDOW",         new ScriptProcedure(s, &si, IFUIFileWindow));
                s.world().setNewGlobalValue("UI.GOTOCHART",          new ScriptProcedure(s, &si, IFUIGotoChart));
                s.world().setNewGlobalValue("UI.GOTOSCREEN",         new ScriptProcedure(s, &si, IFUIGotoScreen));
                s.world().setNewGlobalValue("UI.HELP",               new ScriptProcedure(s, &si, IFUIHelp));
                s.world().setNewGlobalValue("UI.INPUT",              new ScriptProcedure(s, &si, IFUIInput));
                s.world().setNewGlobalValue("UI.INPUTFCODE",         new ScriptProcedure(s, &si, IFUIInputFCode));
                s.world().setNewGlobalValue("UI.INPUTNUMBER",        new ScriptProcedure(s, &si, IFUIInputNumber));
                s.world().setNewGlobalValue("UI.KEYMAPINFO",         new ScriptProcedure(s, &si, IFUIKeymapInfo));
                // s.world().setNewGlobalValue("UI.LISTFLEETS",         IFUIListFleets);
                s.world().setNewGlobalValue("UI.LISTSHIPPREDICTION", new ScriptProcedure(s, &si, IFUIListShipPrediction));
                s.world().setNewGlobalValue("UI.LISTSHIPS",          new ScriptProcedure(s, &si, IFUIListShips));
                s.world().setNewGlobalValue("UI.MESSAGE",            new ScriptProcedure(s, &si, IFUIMessage));
                s.world().setNewGlobalValue("UI.OVERLAYMESSAGE",     new ScriptProcedure(s, &si, IFUIOverlayMessage));
                s.world().setNewGlobalValue("UI.PLANETINFO",         new ScriptProcedure(s, &si, IFUIPlanetInfo));
                s.world().setNewGlobalValue("UI.POPUPCONSOLE",       new ScriptProcedure(s, &si, IFUIPopupConsole));
                s.world().setNewGlobalValue("UI.SCANKEYBOARDMODE",   new ScriptProcedure(s, &si, IFUIScanKeyboardMode));
                s.world().setNewGlobalValue("UI.SEARCH",             new ScriptProcedure(s, &si, IFUISearch));
                s.world().setNewGlobalValue("UI.SELECTIONMANAGER",   new ScriptProcedure(s, &si, IFUISelectionManager));
                s.world().setNewGlobalValue("UI.SHOWSCORES",         new ScriptProcedure(s, &si, IFUIShowScores));
                s.world().setNewGlobalValue("UI.UPDATE",             new ScriptProcedure(s, &si, IFUIUpdate));
            }
    };
    ui.postNewRequest(new Task());
}
