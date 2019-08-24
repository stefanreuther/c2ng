/**
  *  \file server/interface/usermanagement.hpp
  *  \brief Interface server::interface::UserManagement
  */
#ifndef C2NG_SERVER_INTERFACE_USERMANAGEMENT_HPP
#define C2NG_SERVER_INTERFACE_USERMANAGEMENT_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/string/string.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class UserManagement : public afl::base::Deletable {
     public:
        // ADDUSER name pass [key value...]
        virtual String_t add(String_t userName, String_t password, afl::base::Memory<const String_t> config) = 0;

        // DELUSER name
        virtual void remove(String_t userId) = 0;

        // LOGIN user pass
        virtual String_t login(String_t userName, String_t password) = 0;

        // LOOKUP
        virtual String_t getUserIdByName(String_t userName) = 0;

        // NAME uid
        virtual String_t getNameByUserId(String_t userId) = 0;

        // MNAME uid...
        virtual void getNamesByUserId(afl::base::Memory<const String_t> userIds, afl::data::StringList_t& userNames) = 0;

        // GET uid key
        virtual Value_t* getProfileRaw(String_t userId, String_t key) = 0;

        // MGET uid key...
        virtual Value_t* getProfileRaw(String_t userId, afl::base::Memory<const String_t> keys) = 0;

        // SET uid [key value...]
        virtual void setProfile(String_t userId, afl::base::Memory<const String_t> config) = 0;

        // PASSWD name pass
        virtual void setPassword(String_t userId, String_t password) = 0;

        // TODO: LIST, STAT
    };

} }

#endif
