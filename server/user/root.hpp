/**
  *  \file server/user/root.hpp
  *  \brief Class server::user::Root
  */
#ifndef C2NG_SERVER_USER_ROOT_HPP
#define C2NG_SERVER_USER_ROOT_HPP

#include "afl/net/redis/stringsetkey.hpp"
#include "afl/sys/log.hpp"
#include "server/common/idgenerator.hpp"
#include "server/common/root.hpp"
#include "server/types.hpp"
#include "server/user/configuration.hpp"

namespace server { namespace user {

    class Token;
    class PasswordEncrypter;

    /** A user server's root state.
        Contains global configuration and state objects.
        Root is shared between all connections.

        Root contains the top-level database layout rules.
        All accesses happen through subtree or other objects given out by Root.

        <b>Usage Guidelines:</b>

        Root produces links (afl::net::redis::Subtree) to parts of the database.
        Data model objects (Forum, Group, etc.) should never keep a reference to a Root.
        Instead, when a function needs to refer to data outside its object, pass it a Root reference as parameter,
        to make these outside accesses explicit. */
    class Root : public server::common::Root {
     public:
        /** Constructor.
            \param db Database connection
            \param gen Random Id generator
            \param encrypter Password encrypter
            \param config Server configuration */
        Root(afl::net::CommandHandler& db, server::common::IdGenerator& gen, PasswordEncrypter& encrypter, const Configuration& config);

        /** Destructor. */
        ~Root();


        /*
         *  Nested Objects
         */

        /** Access logger.
            Attach a listener to receive log messages.
            \return logger */
        afl::sys::Log& log();

        /** Access IdGenerator.
            \return IdGenerator */
        server::common::IdGenerator& generator();

        /** Access password encrypter.
            \return encrypter */
        PasswordEncrypter& encrypter();

        /** Get current time.
            The time is specified in minutes-since-epoch.
            \return time */
        Time_t getTime();

        /** Configuration. */
        const Configuration& config() const;


        /*
         *  Database Schema
         */

        /** Access set of all active tokens.
            \return set */
        afl::net::redis::StringSetKey allTokens();

        /** Access a token's metadata.
            \return token metadata */
        Token tokenById(String_t token);

        /** Allocate a user Id.
            \return new user Id */
        String_t allocateUserId();

        /** Access set of all live user Ids.
            \return set */
        afl::net::redis::StringSetKey allUsers();

        /** Access copyable default profile.
            When a user is created, this hash is copied into their profile.
            Unlike defaultProfile(), a change in defaultProfileCopy()'s content will not affect existing users.
            \return hash */
        afl::net::redis::HashKey defaultProfileCopy();

     private:
        afl::sys::Log m_log;
        afl::net::CommandHandler& m_db;
        server::common::IdGenerator& m_generator;
        PasswordEncrypter& m_encrypter;
        const Configuration m_config;
    };

} }

#endif
