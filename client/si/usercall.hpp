/**
  *  \file client/si/usercall.hpp
  */
#ifndef C2NG_CLIENT_SI_USERCALL_HPP
#define C2NG_CLIENT_SI_USERCALL_HPP

namespace client { namespace si {

    class Control;
    class RequestLink2;
    class UserSide;

    /** Script-triggered User-Interface Call.
        Unlike a UserTask, the UserCall executes synchronously.
        It must therefore not block. */
    class UserCall {
     public:
        /** Virtual destructor. */
        virtual ~UserCall()
            { }

        /** Execute the call.
            \param ui UserSide of script interface
            \param ctl Active screen's Control */
        virtual void handle(UserSide& ui, Control& ctl) = 0;
    };

} }

#endif
