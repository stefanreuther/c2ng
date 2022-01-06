/**
  *  \file client/si/userside.hpp
  *  \brief Class client::si::UserSide
  */
#ifndef C2NG_CLIENT_SI_USERSIDE_HPP
#define C2NG_CLIENT_SI_USERSIDE_HPP

#include <vector>
#include "client/screenhistory.hpp"
#include "client/si/requestlink2.hpp"
#include "client/si/scripttask.hpp"
#include "client/widgets/busyindicator.hpp"
#include "game/extraidentifier.hpp"
#include "game/session.hpp"
#include "interpreter/process.hpp"
#include "ui/root.hpp"
#include "util/messagecollector.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace client { namespace si {

    class Control;
    class RequestLink2;
    class ScriptSide;
    class UserCall;
    class ContextProvider;

    extern const game::ExtraIdentifier<game::Session, ScriptSide> SCRIPTSIDE_ID;

    /** Script/UI Interaction: User-interface Side.
        Installs and communicates with a ScriptSide.

        A UserSide maintains a stack of listeners (Control) that correspond to the current UI state.
        The current context is represented by the topmost Control.
        Most patterns are implemented as a cooperation of Control and UserSide.

        Main entry points:
        - executeTaskWait()
        - continueProcess()
        - detachProcess()

        These patterns are implemented in class Control. */
    class UserSide {
     public:
        class ScriptRequest : public afl::base::Deletable {
         public:
            virtual void handle(game::Session& s, ScriptSide& si) = 0;
        };

        /** Constructor.
            @param root         UI root
            @param gameSender   RequestSender to execute stuff on a game::Session
            @param tx           Translator
            @param self         RequestDispatcher used to execute stuff on this object (UI thread; typically root.engine().dispatcher())
            @param console      Console (for configuration)
            @param mainLog      Main logger (for logging) */
        UserSide(ui::Root& root,
                 util::RequestSender<game::Session> gameSender,
                 afl::string::Translator& tx,
                 util::RequestDispatcher& self,
                 util::MessageCollector& console,
                 afl::sys::Log& mainLog);

        /** Destructor. */
        ~UserSide();

        /** Access game::Session sender.
            @return sender */
        util::RequestSender<game::Session> gameSender()
            { return m_gameSender; }

        /** Access UserSide sender.
            @return sender */
        // FIXME: can we get along without exporting this?
        util::RequestSender<UserSide> userSender()
            { return m_receiver.getSender(); }

        /** Access console.
            @return console */
        util::MessageCollector& console()
            { return m_console; }

        /** Access main logger.
            @return logger */
        afl::sys::Log& mainLog()
            { return m_mainLog; }

        /** Access screen history.
            @return history */
        ScreenHistory& history()
            { return m_history; }

        /** Access UI root.
            @return root */
        ui::Root& root()
            { return m_root; }

        /** Access translator.
            @return translator */
        afl::string::Translator& translator()
            { return m_translator; }

        /** Reset UI state. */
        void reset();


        /*!
         *  \name Requests to Script Side
         */

        /** Post a request to execute on the ScriptSide (low-level version).
            @param request Newly-allocated request */
        void postNewRequest(ScriptRequest* request);


        /*!
         *  \name Process Functions
         *
         *  These functions operate on individual processes identified by a RequestLink2.
         */
        ///@{

        /** Continue a process after UI callout.
            Call this after successfully executing a user interface request originating from process @c link.
            This will eventually continue executing the process with no other change in execution state.

            Typical use:
            - script command performs ScriptSide::postNewTask
            - ...dispatched to UserSide::processInteraction...
            - task performs UI action
            - task calls continueProcess() to continue the originating process

            @param link Identification of the process

            @see ScriptSide::continueProcess */
        void continueProcess(RequestLink2 link);

        /** Join processes into a process group.
            Moves process @c other into the same process group as @c link.
            Call continueProcess(link) next.

            Typical use:
            - script command performs ScriptSide::postNewTask
            - ...dispatched to UserSide::processInteraction...
            - task performs UI action which itself supports scripts and produces an outbound process (OutputState::getProcess())
            - task calls joinProcess(X, out.getProcess())
            - task calls continueProcess(X)

            @param link    Target process identification
            @param other   Other process identification

            @see ScriptSide::joinProcess  */
        void joinProcess(RequestLink2 link, RequestLink2 other);

        /** Join process group.
            Moves content of @c oldGroup into the same process group as @c link.
            Call continueProcess(link) next.

            @param link    Target process identification
            @param oldGroup Old process group

            @see ScriptSide::joinProcessGroup */
        void joinProcessGroup(RequestLink2 link, uint32_t oldGroup);

        /** Continue a process after UI callout with error.
            Call this after executing a user interface request to produce an error.
            This will eventually continue executing the process as if an "Abort error" statement had been executed.
            Otherwise, similar to continueProcess().

            @param link Identification of the process
            @param error Error message

            @see ScriptSide::continueProcessWithFailure */
        void continueProcessWithFailure(RequestLink2 link, String_t error);

        /** Detach from process after UI callout.
            This will emit an onTaskComplete() callback for the given process,
            but keep the process running.

            Typical use:
            - script command performs ScriptSide::postNewTask
            - ...dispatched to UserSide::processInteraction...
            - task performs detachProcess()
            - task waits for onTaskComplete() callback (important to avoid races/recursion!)
            - ...typically, context change here...
            - new context does continueProcessWait()

            @param link Identification of the process
            @see ScriptSide::detachProcess */
        void detachProcess(RequestLink2 link);

        /** Set variable in process.
            @param link Identification of the process
            @param name Variable name
            @param value Value. Must be a scalar because only scalars are allowed to pass thread boundaries.
            @see ScriptSide::setVariable */
        void setVariable(RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value);

        ///@}

        /*!
         *  \name Process Group / Wait Functions
         *
         *  These functions operate on process groups.
         *  For all these functions,
         *  - register a Control to receive callbacks and obtain a waitId using allocateWaitId()
         *  - optionally, update your own state
         *  - call a wait function, such as executeCommandWait(), passing it the wait Id, as the last thing you do.
         *
         *  This will eventually produce a onTaskComplete() callback with the given waitId.
         */
        ///@{

        /** Allocate a wait Id.
            @return newly-allocated wait Id */
        uint32_t allocateWaitId();

        /** Continue a detached process.
            Use this to continue a process detached using detachProcess().
            Completion will eventually be signalled using a onTaskComplete() callback
            when the process's process group finishes.

            @param waitId Wait Id
            @param link Process identification

            @see ScriptSide::continueProcessWait */
        void continueProcessWait(uint32_t waitId, RequestLink2 link);

        /** Execute a task.
            The task will be executed on ScriptSide, it will be given a new process group, and can populate that with processes.
            Those will be run; completion of the process group will be signalled with onTaskComplete() for the given waitId.

            @param waitId   Wait Id for the onTaskComplete() callback
            @param task     The task

            @see ScriptSide::executeTaskWait */
        void executeTaskWait(uint32_t id, std::auto_ptr<ScriptTask> task);

        /** Create ContextProvider.
            Calls the current Control's createContextProvider, if any.
            @return newly-allocated ContextProvider object */
        ContextProvider* createContextProvider();
        ///@}


        /*!
         *  \name Listener Functions
         *
         *  These functions manage listeners.
         *  Note that Control instances manage registration internally; do not call these methods yourself.
         *
         *  Listeners are stacked.
         *  Therefore, removeControl() should always remove the most recently added listener, but this is not required.
         */
        ///@{

        /** Add listener.
            \param p Listener */
        void addControl(Control& p);

        /** Remove listener.
            \param p Listener */
        void removeControl(Control& p);

        /** Get current (=topmost) control.
            @return control; null if none */
        Control* getControl();

        /** Handle successful wait (called by ScriptSide).
            Finds the associated Control, and calls it's onTaskComplete() function.
            Called by ScriptSide.
            @param id Wait Id */
        void onTaskComplete(uint32_t id);
        ///@}


        /*!
         *  \name Script-side Actions
         *
         *  These functions are invoked by ScriptSide.
         */

        /** Process an interaction.
            The interaction is allowed to interact with the user.
            @param req Request

            @see ScriptSide::postNewInteraction */
        void processInteraction(util::Request<UserSide>& req);

        /** Process a synchronous script call.
            The call is not allowed to interact with the user.
            @param t Task
            @see ScriptSide::call, ScriptSide::callAsyncNew */
        void processCall(UserCall& t);


        /*!
         *  \name Wait Indicator
         */
        ///@{

        /** Set visibility of wait-indicator.
            If set to true, UI input is deferred and user sees a "please wait" popup.

            @param enable New state
            @return old state */
        bool setWaiting(bool enable);
        ///@}

     private:
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<UserSide> m_receiver;
        util::MessageCollector& m_console;
        afl::sys::Log& m_mainLog;
        ScreenHistory m_history;
        client::widgets::BusyIndicator m_blocker;
        ui::Root& m_root;
        afl::string::Translator& m_translator;

        uint32_t m_waitIdCounter;

        std::vector<Control*> m_controls;
    };

} }

#endif
