/**
  *  \file server/interface/userdataclient.hpp
  *  \brief Class server::interface::UserDataClient
  */
#ifndef C2NG_SERVER_INTERFACE_USERDATACLIENT_HPP
#define C2NG_SERVER_INTERFACE_USERDATACLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/userdata.hpp"

namespace server { namespace interface {

    /** Client for UserData interface.
        Accesses a UserDataServer by the means of a CommandHandler interface. */
    class UserDataClient : public UserData {
     public:
        /** Constructor.
            \param commandHandler CommandHandler interface implementing the client/server transition. */
        explicit UserDataClient(afl::net::CommandHandler& commandHandler);

        // UserData:
        virtual void set(String_t userId, String_t key, String_t value);
        virtual String_t get(String_t userId, String_t key);

     private:
        afl::net::CommandHandler& m_commandHandler;

    };

} }

#endif
