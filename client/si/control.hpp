/**
  *  \file client/si/control.hpp
  *  \brief Class client::si::Control
  */
#ifndef C2NG_CLIENT_SI_CONTROL_HPP
#define C2NG_CLIENT_SI_CONTROL_HPP

#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/requestlink2.hpp"
#include "game/interface/contextprovider.hpp"
#include "game/reference.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "ui/eventloop.hpp"
#include "util/key.hpp"

namespace client { namespace si {

    class UserSide;
    class ScriptTask;


    /** Script/UI Interaction: per-context adaption.
        Depending on user-interface status, scripts have different effect
        (e.g. UI.PopupConsole behaves differently when the console is already open).

        A Control object receives requests from scripts.
        See "User-Interface Callouts" for details. */
    // FIXME: for testability and configurability, it makes sense to split this class into a listener half (UserSide callback)
    // and an actual implementation (EventLoop/Root etc.)
    class Control : public afl::base::Uncopyable {
     public:
        /** Constructor.
            @param us       UserSide */
        Control(UserSide& us);

        /** Destructor. */
        virtual ~Control();

        /*
         *  Associated Objects
         */

        /** Access associated root.
            @return root */
        ui::Root& root()
            { return m_root; }

        /** Access associated UserSide.
            @return UserSide */
        UserSide& interface()
            { return m_interface; }

        /** Access associated translator. */
        afl::string::Translator& translator()
            { return m_translator; }

        /** Execute a script command.
            See executeTaskWait() for details.
            @param command Command text
            @param verbose true for verbose execution (i.e. console command, result logging)
            @param name    Name of process */
        void executeCommandWait(String_t command, bool verbose, String_t name);

        /** Execute hook.
            See executeTaskWait() for details.
            @param name Hook name */
        void executeHookWait(String_t name);

        /** Execute a key command.
            Resolves the key into a command and executes that.
            See executeTaskWait() for details.
            @param keymapName Keymap name
            @param key        Key
            @param prefix     Prefix argument */
        void executeKeyCommandWait(String_t keymapName, util::Key_t key, int prefix);

        /** Execute a "UI.GotoReference" command with the given game::Reference.
            Same as executeCommandWait(), but avoids stringifying/parsing the game::Reference.
            See executeTaskWait() for details.
            @param taskName   Name of process
            @param ref        Reference */
        void executeGoToReferenceWait(String_t taskName, game::Reference ref);

        /** Execute a script task.
            Will execute the (process created by the) task.
            During the execution, user will see a wait indicator.

            This function will return when
            (a) the process/process group finished execution
            (b) a callback (handleStateChange() etc.) used UserSide::detachProcess()

            Typically, you call executeTaskWait(), then enter your event loop; see "User-Interface Callouts".

            @see UserSide::executeTaskWait(), ScriptSide::executeTaskWait() */
        void executeTaskWait(std::auto_ptr<ScriptTask> task);

        /** Continue a detached process.
            Will execute the process and others in its group.
            During the execution, user will see a wait indicator.

            This function will return when
            (a) the process/process group finished execution
            (b) a callback (handleStateChange() etc.) used UserSide::detachProcess()

            Typically, you call executeTaskWait(), then enter your event loop; see "User-Interface Callouts".

            @see UserSide::continueProcessWait(), ScriptTask::continueProcessWait() */
        void continueProcessWait(RequestLink2 link);

        /** Handle successful wait (called UserSide).
            Releases a possible waiting executeTaskWait(), continueProcessWait().

            @param waitId Wait Id */
        void onTaskComplete(uint32_t waitId);


        /*!
         *  \name User-Interface Callouts
         *
         *  Your UI context will
         *  - execute a possible inbound process using Control::continueProcessWait(),
         *    and execute script commands using Control::executeTaskWait() and friends.
         *  - enter your own event loop.
         *
         *  Each of these functions implements a context-dependant script command.
         *
         *  Each command is executed in a user-interaction context (UserSide::processInteraction).
         *  It can:
         *  - eventually call interface().continueProcess() to continue the process.
         *  - eventually call interface().detachProcess() and cause your own event loop to exit.
         *    After the event loop exit, arrange for your caller to eventually call continueProcessWait()
         *    for the detached process.
         *
         *  Your own event loop must handle receiving a stop request before being started;
         *  ui::EventLoop satisfies this requirement.
         */
        ///@{

        /** Handle state change (UI.GotoScreen).
            @param link   Originating process
            @param target Target state
            @see dialogHandleStateChange */
        virtual void handleStateChange(RequestLink2 link, OutputState::Target target) = 0;

        /** Handle UI.EndDialog command.
            @param link   Originating process
            @param code   Result code
            @see dialogHandleEndDialog */
        virtual void handleEndDialog(RequestLink2 link, int code) = 0;

        /** Handle UI.PopupConsole command.
            @param link   Originating process
            @see defaultHandlePopupConsole */
        virtual void handlePopupConsole(RequestLink2 link) = 0;

        /** Handle UI.ScanKeyboardMode command.
            @param link   Originating process
            @see defaultHandleScanKeyboardMode */
        virtual void handleScanKeyboardMode(RequestLink2 link) = 0;

        /** Handle Chart.SetView command.
            @param link   Originating process
            @param name   View name
            @param withKeymap Activate keymap with same name as well
            @see defaultHandleSetView */
        virtual void handleSetView(RequestLink2 link, String_t name, bool withKeymap) = 0;

        /** Handle UseKeymap command.
            @param link   Originating process
            @param name   Keymap name
            @param prefix Prefix argument
            @see defaultHandleUseKeymap */
        virtual void handleUseKeymap(RequestLink2 link, String_t name, int prefix) = 0;

        /** Handle UI.OverlayMessage command.
            @param link   Originating process
            @param text   Text
            @see defaultHandleOverlayMessage */
        virtual void handleOverlayMessage(RequestLink2 link, String_t text) = 0;
        ///@}

        /** Get focused object of a given type.
            Examines the user-interface focus.
            - if the focused user-interface object is of the given type, return its Id;
            - if additional objects might be in perceived focus in addition to this one, return Nothing to continue the search (see defaultGetFocusedObjectId());
            - if no additional objects are in focus (i.e. this is a control screen blocking anything below), return 0.
            @param type Desired object type
            @return Id or Nothing, as described */
        virtual afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type type) const = 0;

        /** Create context provider.
            Used for newly-created processes (e.g. command on ship screen executes in ship context).
            @return ContextProvider, may be null */
        virtual game::interface::ContextProvider* createContextProvider() = 0;


     protected:
        /** Default implementation of handlePopupConsole().
            Will show the console.
            This is a sensible implementation for all contexts except for the console.
            @param link   Originating process (link parameter for handlePopupConsole) */
        void defaultHandlePopupConsole(RequestLink2 link);

        /** Default implementation of handleScanKeyboardMode().
            Rejects the request with an error.
            This is a sensible implementation for all contexts that have no scanner.
            @param link   Originating process (link parameter for handleScanKeyboardMode) */
        void defaultHandleScanKeyboardMode(RequestLink2 link);

        /** Default implementation of handleSetView().
            Rejects the request with an error.
            This is a sensible implementation for all contexts other than the starchart.
            @param link   Originating process (link parameter for handleSetView)
            @param name   View name (name parameter for handleSetView)
            @param withKeymap Activate keymap (withKeymap for handleSetView) */
        void defaultHandleSetView(RequestLink2 link, String_t name, bool withKeymap);

        /** Default implementation of handleUseKeymap().
            Acquires a keystroke using the requested keymap and executes it.
            This is a sensible implementation for all contexts other than the keymap implementation itself.
            @param link   Originating process (link parameter for handleUseKeymap)
            @param name   Keymap name (name parameter for handleUseKeymap)
            @param prefix Prefix argument (prefix parameter for handleUseKeymap) */
        void defaultHandleUseKeymap(RequestLink2 link, String_t name, int prefix);

        /** Default implementation of handleOverlayMessage().
            Displays the message.
            This is a sensible implementation for most contexts.
            @param link   Originating process (link parameter for handleOverlayMessage)
            @param text   Text (text parameter for handleOverlayMessage) */
        void defaultHandleOverlayMessage(RequestLink2 link, String_t text);

        /** Default implementation of getFocusedObjectId().
            @param type   Desired type (parameter from getFocusedObjectId())
            @return Nothing */
        afl::base::Optional<game::Id_t> defaultGetFocusedObjectId(game::Reference::Type type) const;

        /** Implementation of handleStateChange() for dialogs.
            Use if this Control represents a dialog.
            That dialog is active in an EventLoop.
            This function will, if needed, set the OutputState object and cause the EventLoop to exit
            signalling the dialog to report that OutputState to its caller.

            @param link     Process (link parameter for handleStateChange)
            @param target   Target state (target parameter for handleStateChange)
            @param out      OutputState you're going to return
            @param loop     EventLoop
            @param n        EventLoop exit code to use */
        void dialogHandleStateChange(RequestLink2 link, OutputState::Target target, OutputState& out, ui::EventLoop& loop, int n);

        /** Implementation of handleEndDialog() for dialogs.
            Use if this Control represents a dialog.
            That dialog is active in an EventLoop.
            This function will, if needed, set the OutputState object and cause the EventLoop to exit
            signalling the dialog to report that OutputState to its caller.

            @param link     Process (link parameter for handleEndDialog)
            @param code     Code provided by script (code parameter for handleEndDialog); ignored but eats up the "unused parameter" warning
            @param out      OutputState you're going to return
            @param loop     EventLoop
            @param n        EventLoop exit code to use */
        void dialogHandleEndDialog(RequestLink2 link, int code, OutputState& out, ui::EventLoop& loop, int n);

     private:
        void executeTaskInternal(std::auto_ptr<ScriptTask> task, String_t name);

        /** UserSide instance. */
        UserSide& m_interface;

        /** Wait Id. We used to allocate an Id for each new wait, but using one per instance is enough. */
        const uint32_t m_id;

        /** Event loop. Used to implement executeTaskWait() etc. */
        ui::EventLoop m_loop;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
    };

} }

#endif
