/**
  *  \file server/interface/usermanagementserver.hpp
  *  \brief Class server::interface::UserManagementServer
  */
#ifndef C2NG_SERVER_INTERFACE_USERMANAGEMENTSERVER_HPP
#define C2NG_SERVER_INTERFACE_USERMANAGEMENTSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class UserManagement;

    /** Server for user management.
        Implements a ComposableCommandHandler and dispatches received commands to a UserManagement implementation. */
    class UserManagementServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit UserManagementServer(UserManagement& impl);

        /** Destructor. */
        ~UserManagementServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        UserManagement& m_implementation;
    };

} }

#endif
