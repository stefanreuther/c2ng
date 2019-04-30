/**
  *  \file server/interface/usertokenserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_USERTOKENSERVER_HPP
#define C2NG_SERVER_INTERFACE_USERTOKENSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class UserToken;

    class UserTokenServer : public ComposableCommandHandler {
     public:
        UserTokenServer(UserToken& impl);
        ~UserTokenServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        UserToken& m_implementation;
    };

} }

#endif
