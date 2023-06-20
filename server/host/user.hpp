/**
  *  \file server/host/user.hpp
  *  \brief Class server::host::User
  */
#ifndef C2NG_SERVER_HOST_USER_HPP
#define C2NG_SERVER_HOST_USER_HPP

#include "server/common/user.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"

namespace server { namespace host {

    class Root;

    /** A user profile.
        This encapsulates the user profile access for c2host.
        It is based on the common User class. */
    class User : public server::common::User {
     public:
        /** Constructor.
            \param root Service root
            \param userId User Id ("1001") */
        User(Root& root, String_t userId);

        /** Destructor. */
        ~User();

        /** Check whether user can join games. */
        bool isJoinAllowed();

        /** Access set of owned games.
            \return set of owned games */
        afl::net::redis::IntegerSetKey ownedGames();

        /** Access user history.
            The user history contains events, with the newest event in front.
            \return user history */
        afl::net::redis::StringListKey history();

        /** Access game reference counts.
            This hash maps game numbers to reference counts
            (e.g. 2=subscribed to the game in two slots; 0=not subscribed but allowed to see the game)
            \return game reference counts */
        afl::net::redis::HashKey gameReferenceCounts();

        /** Access game reference count for one game.
            \param gameId game to query
            \return game reference count */
        afl::net::redis::IntegerField gameReferenceCount(int32_t gameId);

        /** Access key store.
            \return keystore subtree */
        afl::net::redis::Subtree keyStore();


        /*
         *  Profile Access
         */

        /** Access user's rank-level.
            \return rank level (0=recruit, etc.) */
        afl::net::redis::IntegerField rankLevel();

        /** Access user's skill points.
            \return skill points */
        afl::net::redis::IntegerField rankPoints();

        /** Access user's turn reliability.
            \return turn reliability */
        afl::net::redis::IntegerField turnReliability();

        /** Access number of turns played.
            \return number of turns played */
        afl::net::redis::IntegerField numTurnsPlayed();

        /** Access number of turns missed.
            \return number of turns played */
        afl::net::redis::IntegerField numTurnsMissed();
    };

} }

#endif
