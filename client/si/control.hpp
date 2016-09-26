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

namespace client { namespace si {

    class UserSide;
    class ContextProvider;

    // FIXME: for testability and configurability, it makes sense to split this class into a listener half (UserSide callback)
    // and an actual implementation (EventLoop/Root etc.)
    class Control : public afl::base::Uncopyable {
     public:
        Control(UserSide& iface, ui::Root& root, afl::string::Translator& tx);
        virtual ~Control();

        void executeCommandWait(String_t command, bool verbose, String_t name);
        void executeKeyCommandWait(String_t keymapName, util::Key_t key, int prefix);
        void continueProcessWait(RequestLink2 link);
        void handleWait(uint32_t id, interpreter::Process::State state, interpreter::Error error);
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
        virtual void handleStateChange(UserSide& ui, RequestLink2 link, OutputState::Target target) = 0;
        virtual void handleEndDialog(UserSide& ui, RequestLink2 link, int code) = 0;
        virtual void handlePopupConsole(UserSide& ui, RequestLink2 link) = 0;
        virtual ContextProvider* createContextProvider() = 0;

     protected:
        void defaultHandlePopupConsole(UserSide& ui, RequestLink2 link);

     private:
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
