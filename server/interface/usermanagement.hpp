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

    /** Interface for managing users. */
    class UserManagement : public afl::base::Deletable {
     public:
        /** Add a new user (ADDUSER).
            @param userName  User name
            @param password  Password
            @param config    List of key/value pairs of additional attributes
            @return user Id */
        virtual String_t add(String_t userName, String_t password, afl::base::Memory<const String_t> config) = 0;

        /** Delete a user (DELUSER).
            @param userId  User Id */
        virtual void remove(String_t userId) = 0;

        /** Check password (LOGIN).
            @param userName  User name
            @param password  Password
            @return user Id on success */
        virtual String_t login(String_t userName, String_t password) = 0;

        /** Look up user name (LOOKUP).
            @param userName  User name
            @return user Id */
        virtual String_t getUserIdByName(String_t userName) = 0;

        /** Retrieve name for a user Id (NAME).
            @param userId  User Id
            @return user name */
        virtual String_t getNameByUserId(String_t userId) = 0;

        /** Retrieve names for a list of user Ids (MNAME).
            @param userIds   List of user Ids
            @param userNames Names */
        virtual void getNamesByUserId(afl::base::Memory<const String_t> userIds, afl::data::StringList_t& userNames) = 0;

        /** Get user profile value (GET).
            @param userId  User Id
            @param key     Profile key
            @return newly-created value; caller assumes ownership */
        virtual Value_t* getProfileRaw(String_t userId, String_t key) = 0;

        /** Get multiple user profile values (MGET).
            @param userId  User Id
            @param keys    Profile keys
            @return newly-created value (array of values); caller assumes ownership */
        virtual Value_t* getProfileRaw(String_t userId, afl::base::Memory<const String_t> keys) = 0;

        /** Set user profile values.
            @param userId  User Id
            @param config  List of key/value pairs */
        virtual void setProfile(String_t userId, afl::base::Memory<const String_t> config) = 0;

        /** Change user password.
            @param userId   User Id
            @param password New password */
        virtual void setPassword(String_t userId, String_t password) = 0;
    };

} }

#endif
