/**
  *  \file server/interface/usermanagementclient.hpp
  *  \brief Class server::interface::UserManagementClient
  */
#ifndef C2NG_SERVER_INTERFACE_USERMANAGEMENTCLIENT_HPP
#define C2NG_SERVER_INTERFACE_USERMANAGEMENTCLIENT_HPP

#include "server/interface/usermanagement.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class UserManagementClient : public UserManagement {
     public:
        explicit UserManagementClient(afl::net::CommandHandler& commandHandler);

        virtual String_t add(String_t userName, String_t password, afl::base::Memory<const String_t> config);
        virtual String_t login(String_t userName, String_t password);
        virtual String_t getUserIdByName(String_t userName);
        virtual String_t getNameByUserId(String_t userId);
        virtual void getNamesByUserId(afl::base::Memory<const String_t> userIds, afl::data::StringList_t& userNames);
        virtual Value_t* getProfileRaw(String_t userId, String_t key);
        virtual Value_t* getProfileRaw(String_t userId, afl::base::Memory<const String_t> keys);
        virtual void setProfile(String_t userId, afl::base::Memory<const String_t> config);
        virtual void setPassword(String_t userId, String_t password);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
