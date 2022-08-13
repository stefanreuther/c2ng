/**
  *  \file server/user/usermanagement.hpp
  *  \brief Class server::user::UserManagement
  */
#ifndef C2NG_SERVER_USER_USERMANAGEMENT_HPP
#define C2NG_SERVER_USER_USERMANAGEMENT_HPP

#include "server/interface/usermanagement.hpp"

namespace server { namespace user {

    class Root;

    /** Implementation of UserManagement interface.
        This interface allows creating users, logging in, and accessing the user profile. */
    class UserManagement : public server::interface::UserManagement {
     public:
        /** Constructor.
            @param root Service root (database, configuration) */
        explicit UserManagement(Root& root);

        // Interface methods:
        virtual String_t add(String_t userName, String_t password, afl::base::Memory<const String_t> config);
        virtual void remove(String_t userId);
        virtual String_t login(String_t userName, String_t password);
        virtual String_t getUserIdByName(String_t userName);
        virtual String_t getNameByUserId(String_t userId);
        virtual void getNamesByUserId(afl::base::Memory<const String_t> userIds, afl::data::StringList_t& userNames);
        virtual Value_t* getProfileRaw(String_t userId, String_t key);
        virtual Value_t* getProfileRaw(String_t userId, afl::base::Memory<const String_t> keys);
        virtual void setProfile(String_t userId, afl::base::Memory<const String_t> config);
        virtual void setPassword(String_t userId, String_t password);

     private:
        Root& m_root;
    };

} }

#endif
