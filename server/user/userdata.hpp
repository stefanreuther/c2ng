/**
  *  \file server/user/userdata.hpp
  *  \brief Class server::user::UserData
  */
#ifndef C2NG_SERVER_USER_USERDATA_HPP
#define C2NG_SERVER_USER_USERDATA_HPP

#include "server/interface/userdata.hpp"

namespace server { namespace user {

    class Root;

    /** Implementation of UserData interface.
        This used to be implemented in api/user.cgi. */
    class UserData : public server::interface::UserData {
     public:
        /** Constructor.
            @param root Service root */
        explicit UserData(Root& root);

        // Interface methods:
        virtual void set(String_t userId, String_t key, String_t value);
        virtual String_t get(String_t userId, String_t key);

     public:
        Root& m_root;
    };

} }

#endif
