/**
  *  \file client/si/commands.cpp
  */

#include "client/si/commands.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/string.hpp"
#include "client/dialogs/objectselectiondialog.hpp"
#include "client/dialogs/turnlistdialog.hpp"
#include "client/si/control.hpp"
#include "client/si/dialogfunction.hpp"
#include "client/si/listboxfunction.hpp"
#include "client/si/requestlink1.hpp"
#include "client/si/requestlink2.hpp"
#include "client/si/scriptprocedure.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/usercall.hpp"
#include "client/si/userside.hpp"
#include "client/si/usertask.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/interface/plugincontext.hpp"
#include "game/interface/richtextfunctions.hpp"
#include "game/interface/richtextvalue.hpp"
#include "game/map/objecttype.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"
#include "ui/defaultresourceprovider.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/res/factory.hpp"
#include "ui/res/manager.hpp"
#include "ui/res/provider.hpp"
#include "ui/widgets/inputline.hpp"
#include "util/rich/parser.hpp"
#include "util/rich/text.hpp"

using client::si::UserTask;
using client::si::UserSide;
using client::si::Control;
using client::si::OutputState;
using client::si::RequestLink1;
using client::si::RequestLink2;
using client::si::ScriptSide;

namespace {
    class StateChangeTask : public UserTask {
     public:
        StateChangeTask(OutputState::Target target)
            : m_target(target)
            { }
        void handle(UserSide& ui, Control& ctl, RequestLink2 link)
            { ctl.handleStateChange(ui, link, m_target); }
     private:
        OutputState::Target m_target;
    };

    class PopupConsoleTask : public UserTask {
     public:
        PopupConsoleTask()
            { }
        void handle(UserSide& ui, Control& ctl, RequestLink2 link)
            { ctl.handlePopupConsole(ui, link); }
    };

    class MessageBoxTask : public UserTask {
     public:
        MessageBoxTask(game::interface::RichTextValue::Ptr_t pContent, String_t heading)
            : m_content(*pContent), m_heading(heading)
            { }
        MessageBoxTask(util::rich::Text content, String_t heading)
            : m_content(content), m_heading(heading)
            { }
        void handle(UserSide& ui, Control& ctl, RequestLink2 link)
            {
                ui::dialogs::MessageBox(m_content, m_heading, ctl.root()).doOkDialog();
                ui.continueProcess(link);
            }
     private:
        util::rich::Text m_content;
        String_t m_heading;
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
    String_t contextName = "Script";
    String_t contextDir = "";

    const afl::container::PtrVector<interpreter::Context>& ctxs = link.getProcess().getContexts();
    for (size_t i = ctxs.size(); i > 0; --i) {
        if (game::interface::PluginContext* plugContext = dynamic_cast<game::interface::PluginContext*>(ctxs[i-1])) {
            // Fetch plugin's Id and directory to produce a context
            std::auto_ptr<afl::data::Value> tmp;
            interpreter::Context::PropertyIndex_t index;
            if (interpreter::Context* indexContext = plugContext->lookup("ID", index)) {
                tmp.reset(indexContext->get(index));
                contextName = interpreter::toString(tmp.get(), false);
            }
            if (interpreter::Context* dirContext = plugContext->lookup("DIRECTORY", index)) {
                tmp.reset(dirContext->get(index));
                contextDir = interpreter::toString(tmp.get(), false);
            }
            break;
        }
    }

    // Create the resource.
    // FIXME: We are in a state where we are allowed to do I/O (and where exceptions are allowed to be thrown).
    // This may change when we add resource providers that need UI access.
    // At that time, we might have to defer creation of the provider into the RelayTask or ManagerRequest.
    std::auto_ptr<ui::res::Provider> provider(ui::res::createProvider(resourceName, contextDir, session.world().fileSystem()));

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
        void handle(UserSide& ui, Control& ctl, RequestLink2 link)
            {
                if (ui::DefaultResourceProvider* drp = dynamic_cast<ui::DefaultResourceProvider*>(&ctl.root().provider())) {
                    drp->postNewManagerRequest(new ManagerRequest(m_provider, m_contextName), true);
                }
                ui.continueProcess(link);
            }
     private:
        std::auto_ptr<ui::res::Provider> m_provider;
        const String_t m_contextName;
    };
    si.postNewTask(link, new RelayTask(provider, contextName));
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
client::si::IFSystemExitClient(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFSystemExitClient
    args.checkArgumentCount(0);
    si.postNewTask(link, new StateChangeTask(OutputState::ExitProgram));
}

/* @q System.ExitRace (Global Command)
   Leave current race.
   Saves the game and returns to the <a href="pcc2:gamesel">game selection</a> menu.
   @since PCC2 1.99.10, PCC2 2.40 */
void
client::si::IFSystemExitRace(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFSystemExitRace
    args.checkArgumentCount(0);
    si.postNewTask(link, new StateChangeTask(OutputState::ExitGame));
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
    class Task : public UserTask {
     public:
        Task(const client::dialogs::ObjectSelectionDialog& def)
            : m_def(def)
            { }
        void handle(UserSide& iface, Control& ctl, RequestLink2 link)
            {
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
                ctl.handleStateChange(iface, link, out.getTarget());
            }
     private:
        const client::dialogs::ObjectSelectionDialog& m_def;
    };

    // Parse args
    int32_t screen;
    args.checkArgumentCount(1);
    if (!interpreter::checkIntegerArg(screen, args.getNext())) {
        return;
    }

    const client::dialogs::ObjectSelectionDialog* def;
    switch (screen) {
     case 1:
     case 11:
        def = &client::dialogs::SHIP_SELECTION_DIALOG;
        break;

     case 2:
     case 12:
        def = &client::dialogs::PLANET_SELECTION_DIALOG;
        break;

     case 3:
     case 13:
        def = &client::dialogs::BASE_SELECTION_DIALOG;
        break;

//      case 6:
//         /* This needs a GPoint. iuiChartX is always valid even on the player screen,
//            unless we're on a control screen of an object that has no position, where
//            it returns EMPTY. We want it to be 0 for invalid positions, and on the player
//            screen, where there is no user-perceived valid position. */
//         {
//             IntUserInterfaceBinding& ui = IntUserInterfaceBinding::get();
//             std::auto_ptr<IntValue> vx(ui.getProperty(iuiChartX));
//             std::auto_ptr<IntValue> vy(ui.getProperty(iuiChartY));
//             std::auto_ptr<IntValue> vs(ui.getProperty(iuiScreenNumber));
//             int32_t x, y, screen;
//             if (checkIntArg(x, vx.get()) && checkIntArg(y, vy.get()) && checkIntArg(screen, vs.get()) && screen > 0) {
//                 result = chooseHistoryShip(GPoint(x, y));
//             } else {
//                 result = chooseHistoryShip(GPoint(0, 0));
//             }
//         }
//         break;
//      case 10:
//         result = chooseFleet();
//         break;

     default:
        def = 0;
    }

    if (!def) {
        throw interpreter::Error::rangeError();
    }

    if (game::Game* g = session.getGame().get()) {
        session.notifyListeners();
        game::map::ObjectCursor* c = g->cursors().getCursorByNumber(def->screenNumber);
        if (c == 0 || c->getCurrentIndex() == 0) {
            // No ship selected means no ship present; clear UI.Result and show a message
            link.getProcess().setVariable("UI.RESULT", 0);
            si.postNewTask(link, new MessageBoxTask(util::rich::Parser::parseXml(session.translator().translateString(def->failMessageUT)),
                                                    session.translator().translateString(def->titleUT)));
        } else {
            game::map::ObjectType* ty = c->getObjectType();
            if (ty != 0 && ty->isUnit()) {
                // We have only one of this kind. Do not show a dialog; directly select it.
                std::auto_ptr<afl::data::Value> result(interpreter::makeIntegerValue(c->getCurrentIndex()));
                link.getProcess().setVariable("UI.RESULT", result.get());
            } else {
                // Regular task
                si.postNewTask(link, new Task(*def));
            }
        }
    } else {
        throw game::Exception(game::Exception::eUser, session.translator().translateString("No race loaded"));
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
        void handle(UserSide& iface, Control& ctl, RequestLink2 link)
            {
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
        throw game::Exception(game::Exception::eUser, session.translator().translateString("No race loaded"));
    }
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
        void handle(UserSide& ui, Control& ctl, RequestLink2 link)
            { ctl.handleEndDialog(ui, link, m_code); }
     private:
        int m_code;
    };
    si.postNewTask(link, new EndTask(code));
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
    // ex IFUIGotoScreen
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

     default:
        throw interpreter::Error::rangeError();
    }
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
    // ex IFUIInput
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
        void handle(UserSide& ui, Control& ctl, RequestLink2 link)
            {
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
                if (widget.doStandardDialog(m_title, m_prompt)) {
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
        void handle(UserSide& ui, Control& ctl, RequestLink2 link)
            {
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

/* @q UI.PopupConsole (Global Command)
   Open the <a href="pcc2:console">console</a>.
   The script continues running there.
   If your script is doing interesting output to the console,
   you can call this function to ensure the user sees it, even if he bound it to a key.

   @since PCC 1.1.2, PCC2 1.99.10, PCC2 2.40 */
void
client::si::IFUIPopupConsole(game::Session& /*session*/, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args)
{
    // ex IFUIPopupConsole
    args.checkArgumentCount(0);
    si.postNewTask(link, new PopupConsoleTask());
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
    // ex IFUIUpdate
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
            virtual void handle(UserSide& ui, Control& ctl, RequestLink2 link)
                {
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
    class Task : public util::SlaveRequest<game::Session, ScriptSide> {
     public:
        void handle(game::Session& s, ScriptSide& si)
            {
                // Values
                /* @q System.GUI:Bool (Global Property)
                   Graphical interface flag.
                   True if PCC is running with graphical interface, False if it is running in console mode. */
                s.world().setNewGlobalValue("SYSTEM.GUI", interpreter::makeBooleanValue(true));

                // // Procedures
                // s.world().setNewGlobalValue("CC$ADDTOSIM",           IFCCAddToSim);
                // s.world().setNewGlobalValue("CC$BUILDAMMO",          IFCCBuildAmmo);
                // s.world().setNewGlobalValue("CC$BUILDBASE",          IFCCBuildBase);
                // s.world().setNewGlobalValue("CC$BUILDSHIP",          IFCCBuildShip);
                // s.world().setNewGlobalValue("CC$BUILDSTRUCTURES",    IFCCBuildStructures);
                // s.world().setNewGlobalValue("CC$BUYSUPPLIES",        IFCCBuySupplies);
                // s.world().setNewGlobalValue("CC$CARGOHISTORY",       IFCCCargoHistory);
                // s.world().setNewGlobalValue("CC$CHANGEMISSION",      IFCCChangeMission);
                // s.world().setNewGlobalValue("CC$CHANGEPE",           IFCCChangePE);
                // s.world().setNewGlobalValue("CC$CHANGESPEED",        IFCCChangeSpeed);
                // s.world().setNewGlobalValue("CC$CHANGETAXES",        IFCCChangeTaxes);
                // s.world().setNewGlobalValue("CC$CHANGETECH",         IFCCChangeTech);
                // s.world().setNewGlobalValue("CC$CHANGEWAYPOINT",     IFCCChangeWaypoint);
                // s.world().setNewGlobalValue("CC$CSBROWSE",           IFCCCSBrowse);
                // s.world().setNewGlobalValue("CC$GIVE",               IFCCGive);
                // s.world().setNewGlobalValue("CC$LISTSCREENHISTORY",  IFCCListScreenHistory);
                // s.world().setNewGlobalValue("CC$POPSCREENHISTORY",   IFCCPopScreenHistory);
                // s.world().setNewGlobalValue("CC$REMOTECONTROL",      IFCCRemoteControl);
                // s.world().setNewGlobalValue("CC$RESET",              IFCCReset);
                // s.world().setNewGlobalValue("CC$SELLSUPPLIES",       IFCCSellSupplies);
                // s.world().setNewGlobalValue("CC$SENDMESSAGE",        IFCCSendMessage);
                // s.world().setNewGlobalValue("CC$SETTINGS",           IFCCSettings);
                // s.world().setNewGlobalValue("CC$SHIPCOSTCALC",       IFCCShipCostCalc),
                // s.world().setNewGlobalValue("CC$SHIPSPEC",           IFCCShipSpec);
                // s.world().setNewGlobalValue("CC$TOWFLEETMEMBER",     IFCCTowFleetMember);
                // s.world().setNewGlobalValue("CC$TRANSFERMULTI",      IFCCTransferMulti);
                // s.world().setNewGlobalValue("CC$TRANSFERPLANET",     IFCCTransferPlanet);
                // s.world().setNewGlobalValue("CC$TRANSFERSHIP",       IFCCTransferShip);
                // s.world().setNewGlobalValue("CC$TRANSFERUNLOAD",     IFCCTransferUnload);
                // s.world().setNewGlobalValue("CC$USEKEYMAP",          IFCCUseKeymap);
                // s.world().setNewGlobalValue("CC$VIEWINBOX",          IFCCViewInbox);
                // s.world().setNewGlobalValue("CHART.SETVIEW",         IFChartSetView);
                s.world().setNewGlobalValue("LOADRESOURCE",          new ScriptProcedure(s, &si, IFLoadResource));
                s.world().setNewGlobalValue("LISTBOX",               new ListboxFunction(s, &si));
                s.world().setNewGlobalValue("MESSAGEBOX",            new ScriptProcedure(s, &si, IFMessageBox));
                s.world().setNewGlobalValue("SYSTEM.EXITCLIENT",     new ScriptProcedure(s, &si, IFSystemExitClient));
                s.world().setNewGlobalValue("SYSTEM.EXITRACE",       new ScriptProcedure(s, &si, IFSystemExitRace));
                s.world().setNewGlobalValue("UI.CHOOSEOBJECT",       new ScriptProcedure(s, &si, IFUIChooseObject));
                s.world().setNewGlobalValue("UI.CHOOSETURN",         new ScriptProcedure(s, &si, IFUIChooseTurn));
                // s.world().setNewGlobalValue("UI.EDITALLIANCES",      IFUIEditAlliances);
                s.world().setNewGlobalValue("UI.DIALOG",             new DialogFunction(s, &si));
                s.world().setNewGlobalValue("UI.ENDDIALOG",          new ScriptProcedure(s, &si, IFUIEndDialog));
                // s.world().setNewGlobalValue("UI.FILEWINDOW",         IFUIFileWindow);
                // s.world().setNewGlobalValue("UI.GOTOCHART",          IFUIGotoChart);
                s.world().setNewGlobalValue("UI.GOTOSCREEN",         new ScriptProcedure(s, &si, IFUIGotoScreen));
                // s.world().setNewGlobalValue("UI.HELP",               IFUIHelp);
                s.world().setNewGlobalValue("UI.INPUT",              new ScriptProcedure(s, &si, IFUIInput));
                // s.world().setNewGlobalValue("UI.INPUTFCODE",         IFUIInputFCode);
                // s.world().setNewGlobalValue("UI.INPUTNUMBER",        IFUIInputNumber);
                // s.world().setNewGlobalValue("UI.KEYMAPINFO",         IFUIKeymapInfo);
                // s.world().setNewGlobalValue("UI.LISTFLEETS",         IFUIListFleets);
                // s.world().setNewGlobalValue("UI.LISTSHIPPREDICTION", IFUIListShipPrediction);
                // s.world().setNewGlobalValue("UI.LISTSHIPS",          IFUIListShips);
                s.world().setNewGlobalValue("UI.MESSAGE",            new ScriptProcedure(s, &si, IFUIMessage));
                // s.world().setNewGlobalValue("UI.OVERLAYMESSAGE",     IFUIOverlayMessage);
                // s.world().setNewGlobalValue("UI.PLANETINFO",         IFUIPlanetInfo);
                s.world().setNewGlobalValue("UI.POPUPCONSOLE",       new ScriptProcedure(s, &si, IFUIPopupConsole));
                // s.world().setNewGlobalValue("UI.SEARCH",             IFUISearch);
                // s.world().setNewGlobalValue("UI.SELECTIONMANAGER",   IFUISelectionManager);
                s.world().setNewGlobalValue("UI.UPDATE",             new ScriptProcedure(s, &si, IFUIUpdate));

            }
    };
    ui.postNewRequest(new Task());
}
