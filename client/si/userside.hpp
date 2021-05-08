/**
  *  \file client/si/userside.hpp
  *  \brief Class client::si::UserSide
  */
#ifndef C2NG_CLIENT_SI_USERSIDE_HPP
#define C2NG_CLIENT_SI_USERSIDE_HPP

#include <vector>
#include "game/session.hpp"
#include "interpreter/process.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "client/si/requestlink2.hpp"
#include "game/extraidentifier.hpp"
#include "util/messagecollector.hpp"
#include "client/screenhistory.hpp"
#include "client/si/scripttask.hpp"

namespace client { namespace si {

    class Control;
    class RequestLink2;
    class ScriptSide;
    class UserTask;
    class UserCall;
    class ContextProvider;

    extern const game::ExtraIdentifier<game::Session, ScriptSide> SCRIPTSIDE_ID;

    /** User-side of script interface.
        This implements multiple views on scripts.

        - Process Functions: these functions deal with processes and are used by user-interface callbacks.
          A callback is initiated by a process that expects a result.
        - Process Group / Wait Functions: these functions deal with process groups.
          A user-initiated action causes the start of a process group, which the user interface will wait on.
        - Listener Functions: these manage a (stack of) listeners. */
    class UserSide {
     public:
        class ScriptRequest : public afl::base::Deletable {
         public:
            virtual void handle(game::Session& s, ScriptSide& si) = 0;
        };

        /** Constructor.
            \param gameSender RequestSender to execute stuff on a game::Session
            \param self RequestDispatcher used to execute stuff on this object (UI thread) */
        UserSide(util::RequestSender<game::Session> gameSender,
                 util::RequestDispatcher& self,
                 util::MessageCollector& console,
                 afl::sys::Log& mainLog);

        /** Destructor. */
        ~UserSide();

        /** Post a request to execute on the ScriptSide.
            \param request Newly-allocated request */
        void postNewRequest(ScriptRequest* request);

        util::RequestSender<game::Session> gameSender()
            { return m_gameSender; }

        // FIXME: can we get along without exporting this?
        util::RequestSender<UserSide> userSender()
            { return m_receiver.getSender(); }

        util::MessageCollector& console()
            { return m_console; }

        afl::sys::Log& mainLog()
            { return m_mainLog; }

        ScreenHistory& history()
            { return m_history; }

        void reset();


        /*!
         *  \name Process Functions
         *
         *  These functions operate on individual processes identified by a RequestLink2.
         */
        ///@{

        /** Continue a process after UI callout.
            Call this after successfully executing a user interface request originating from process \c link.
            This will eventually continue executing the process with no other change in execution state.
            \param link Identification of the process */
        void continueProcess(RequestLink2 link);

        void joinProcess(RequestLink2 link, RequestLink2 other);

        void joinProcessGroup(RequestLink2 link, uint32_t oldGroup);

        /** Continue a process after UI callout with error.
            Call this after executing a user interface request to produce an error.
            This will eventually continue executing the process as if an "Abort error" statement had been executed.
            \param link Identification of the process
            \param error Error message */
        void continueProcessWithFailure(RequestLink2 link, String_t error);

        /** Detach from process after UI callout.
            This will satisfy the process' wait using state Waiting.
            It will not continue the process.
            To do that, call continueProcessWait() later on.
            \param link Identification of the process */
        void detachProcess(RequestLink2 link);

        /** Process a UserTask.
            Invokes UserTask::handle() with the right parameters.
            \param t Task to execute
            \param link Identification of the invoking process */
        void processTask(UserTask& t, RequestLink2 link);
        void processCall(UserCall& t);

        /** Set variable in process.
            \param link Identification of the process
            \param name Variable name
            \param value Value. Must be a scalar because only scalars are allowed to pass thread boundaries. */
        void setVariable(RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value);

        ///@}

        /*!
         *  \name Process Group / Wait Functions
         *
         *  These functions operate on process groups and wait for them.
         *  For all these functions,
         *  - call allocateWaitId() and store the Id in a member variable
         *  - optionally, update your own state
         *  - call a wait function, such as executeCommandWait(), passing it the wait Id, as the last thing you do.
         *  The callback might occur anytime during or after the wait function.
         */
        ///@{

        /** Allocate a wait Id.
            Each wait Id can be used for one wait operation.
            \return newly-allocated wait Id */
        uint32_t allocateWaitId();

        /** Continue a detached process and setup wait.
            Use this to continue a process detached using detachProcess().
            Completion will eventually be signalled using a handleWait() callback.
            \param id Wait Id allocated with allocateWaitId().
            \param link Process identification */
        void continueProcessWait(uint32_t id, RequestLink2 link);

        /** Execute a task.
            This will provide the given task with a new process group to run in and run it.
            Completion of the process group will eventually be signalled using a handleWait() callback.
            \param waitId   Wait Id for the handleWait() callback
            \param task     The task */
        void executeTaskWait(uint32_t id, std::auto_ptr<ScriptTask> task);

        /** Handle successful wait.
            Finds the associated Control, and calls it's handleWait() function.
            Called by ScriptSide.
            \param id Wait Id */
        void handleWait(uint32_t id);

        /** Create ContextProvider.
            Calls the current Control's createContextProvider, if any.
            \return newly-allocated ContextProvider object */
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
        ///@}

     private:
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<UserSide> m_receiver;
        util::MessageCollector& m_console;
        afl::sys::Log& m_mainLog;
        ScreenHistory m_history;

        uint32_t m_waitIdCounter;

        std::vector<Control*> m_controls;
    };

} }

#endif
