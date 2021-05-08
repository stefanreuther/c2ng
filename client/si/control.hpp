/**
  *  \file client/si/control.hpp
  */
#ifndef C2NG_CLIENT_SI_CONTROL_HPP
#define C2NG_CLIENT_SI_CONTROL_HPP

#include "afl/base/types.hpp"
#include "client/widgets/busyindicator.hpp"
#include "ui/eventloop.hpp"
#include "interpreter/process.hpp"
#include "interpreter/error.hpp"
#include "client/si/requestlink2.hpp"
#include "client/si/outputstate.hpp"
#include "game/reference.hpp"

namespace client { namespace si {

    class UserSide;
    class ContextProvider;
    class ScriptTask;

    // FIXME: for testability and configurability, it makes sense to split this class into a listener half (UserSide callback)
    // and an actual implementation (EventLoop/Root etc.)
    class Control : public afl::base::Uncopyable {
     public:
        Control(UserSide& iface, ui::Root& root, afl::string::Translator& tx);
        virtual ~Control();

        void executeCommandWait(String_t command, bool verbose, String_t name);
        void executeKeyCommandWait(String_t keymapName, util::Key_t key, int prefix);
        void executeGoToReference(String_t taskName, game::Reference ref);
        void executeTaskWait(std::auto_ptr<ScriptTask> task);
        void continueProcessWait(RequestLink2 link);
        void handleWait(uint32_t id);
        void setInteracting(bool state);

        ui::Root& root()
            { return m_root; }

        UserSide& interface()
            { return m_interface; }

        afl::string::Translator& translator()
            { return m_translator; }

        // UI.GotoScreen command:
        //    - either call ui.continueProcess()
        //    - or call ui.detachProcess() [this ends the wait] and arrange for caller to eventually call continueProcessWait() [on a new instance]
        virtual void handleStateChange(RequestLink2 link, OutputState::Target target) = 0;
        virtual void handleEndDialog(RequestLink2 link, int code) = 0;
        virtual void handlePopupConsole(RequestLink2 link) = 0;
        virtual void handleSetViewRequest(RequestLink2 link, String_t name, bool withKeymap) = 0;
        virtual void handleUseKeymapRequest(RequestLink2 link, String_t name, int prefix) = 0;
        virtual void handleOverlayMessageRequest(RequestLink2 link, String_t text) = 0;
        virtual ContextProvider* createContextProvider() = 0;

     protected:
        void defaultHandlePopupConsole(RequestLink2 link);
        void defaultHandleSetViewRequest(RequestLink2 link, String_t name, bool withKeymap);

        void defaultHandleUseKeymapRequest(RequestLink2 link, String_t name, int prefix);
        void defaultHandleOverlayMessageRequest(RequestLink2 link, String_t text);

        /** Implementation of handleStateChange() for dialogs.
            Use if this Control represents a dialog.
            That dialog is active in an EventLoop.
            This function will, if needed, set the OutputState object and cause the EventLoop to exit
            signalling the dialog to report that OutputState to its caller.

            \param link     Process (link parameter for handleStateChange)
            \param target   Target state (target parameter for handleStateChange)
            \param out      OutputState you're going to return
            \param loop     EventLoop
            \param n        EventLoop exit code to use */
        void dialogHandleStateChange(RequestLink2 link, OutputState::Target target, OutputState& out, ui::EventLoop& loop, int n);

        /** Implementation of handleEndDialog() for dialogs.
            Use if this Control represents a dialog.
            That dialog is active in an EventLoop.
            This function will, if needed, set the OutputState object and cause the EventLoop to exit
            signalling the dialog to report that OutputState to its caller.

            \param link     Process (link parameter for handleEndDialog)
            \param code     Code provided by script (code parameter for handleEndDialog); ignored but eats up the "unused parameter" warning
            \param out      OutputState you're going to return
            \param loop     EventLoop
            \param n        EventLoop exit code to use */
        void dialogHandleEndDialog(RequestLink2 link, int code, OutputState& out, ui::EventLoop& loop, int n);

     private:
        void executeTaskInternal(std::auto_ptr<ScriptTask> task, String_t name);
        void updateBlocker();

        bool m_waiting;
        bool m_interacting;
        bool m_active;
        UserSide& m_interface;
        uint32_t m_id;
        client::widgets::BusyIndicator m_blocker;
        ui::EventLoop m_loop;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
    };

} }

#endif
