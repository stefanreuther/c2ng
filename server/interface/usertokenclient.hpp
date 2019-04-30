/**
  *  \file server/interface/usertokenclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_USERTOKENCLIENT_HPP
#define C2NG_SERVER_INTERFACE_USERTOKENCLIENT_HPP

#include "server/interface/usertoken.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class UserTokenClient : public UserToken {
     public:
        explicit UserTokenClient(afl::net::CommandHandler& commandHandler);

        virtual String_t getToken(String_t userId, String_t tokenType);
        virtual Info checkToken(String_t token, afl::base::Optional<String_t> requiredType, bool autoRenew);
        virtual void clearToken(String_t userId, afl::base::Memory<const String_t> tokenTypes);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
