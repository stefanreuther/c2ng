/**
  *  \file server/interface/usermanagementserver.hpp
  *  \brief Class server::interface::UserManagementServer
  */
#ifndef C2NG_SERVER_INTERFACE_USERMANAGEMENTSERVER_HPP
#define C2NG_SERVER_INTERFACE_USERMANAGEMENTSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class UserManagement;

    class UserManagementServer : public ComposableCommandHandler {
     public:
        explicit UserManagementServer(UserManagement& impl);
        ~UserManagementServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        UserManagement& m_implementation;
    };

} }

#endif
