/**
  *  \file client/si/usercall.hpp
  */
#ifndef C2NG_CLIENT_SI_USERCALL_HPP
#define C2NG_CLIENT_SI_USERCALL_HPP

namespace client { namespace si {

    class Control;
    class RequestLink2;

    /** Script-triggered User-Interface Call.
        Unlike a UserTask, the UserCall executes synchronously.
        It must therefore not block. */
    class UserCall {
     public:
        /** Virtual destructor. */
        virtual ~UserCall()
            { }

        /** Execute the call.
            \param ctl Active screen's Control */
        virtual void handle(Control& ctl) = 0;
    };

} }

#endif
