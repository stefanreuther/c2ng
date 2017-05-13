/**
  *  \file server/interface/baseclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_BASECLIENT_HPP
#define C2NG_SERVER_INTERFACE_BASECLIENT_HPP

#include "server/interface/base.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class BaseClient : public Base {
     public:
        BaseClient(afl::net::CommandHandler& commandHandler);
        ~BaseClient();

        virtual String_t ping();
        virtual void setUserContext(String_t user);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
