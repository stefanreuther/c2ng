/**
  *  \file server/interface/usertokenserver.hpp
  *  \brief Class server::interface::UserTokenServer
  */
#ifndef C2NG_SERVER_INTERFACE_USERTOKENSERVER_HPP
#define C2NG_SERVER_INTERFACE_USERTOKENSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class UserToken;

    /** Server for user token access.
        Implements a ComposableCommandHandler and dispatches received commands to a UserToken implementation. */
    class UserTokenServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        UserTokenServer(UserToken& impl);

        /** Destructor. */
        ~UserTokenServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        UserToken& m_implementation;
    };

} }

#endif
