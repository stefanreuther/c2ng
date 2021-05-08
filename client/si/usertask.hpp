/**
  *  \file client/si/usertask.hpp
  *  \brief Class client::si::UserTask
  */
#ifndef C2NG_CLIENT_SI_USERTASK_HPP
#define C2NG_CLIENT_SI_USERTASK_HPP

namespace client { namespace si {

    class Control;
    class RequestLink2;
    class UserSide;

    /** Script-triggered User-Interface Task. */
    class UserTask {
     public:
        /** Virtual destructor. */
        virtual ~UserTask()
            { }

        /** Execute the task.
            This method must execute the task and continue the underlying user-interface process using one of the following methods:
            - ui.continueProcess()
            - ui.continueProcessWithFailure()
            - ui.detachProcess(), plus a call to ui.continueProcessWait() at a later time

            \param ctl Active screen's Control
            \param link Identification of the requesting process */
        virtual void handle(Control& ctl, RequestLink2 link) = 0;
    };

} }

#endif
