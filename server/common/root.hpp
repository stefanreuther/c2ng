/**
  *  \file server/common/root.hpp
  *  \brief Class server::common::Root
  */
#ifndef C2NG_SERVER_COMMON_ROOT_HPP
#define C2NG_SERVER_COMMON_ROOT_HPP

#include "afl/net/commandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/subtree.hpp"

namespace server { namespace common {

    /** Root state for a service using the database.
        Contains access to database nodes that are shared between multiple services.
        All accesses happen through subtree or other objects given out by Root.

        <b>Usage Guidelines:</b>

        Root produces links (afl::net::redis::Subtree) to parts of the database.
        Data model objects should never keep a reference to a Root.
        Instead, when a function needs to refer to data outside its object, pass it a Root reference as parameter,
        to make these outside accesses explicit.

        Derived classes will provide additional database nodes,
        and additional configuration.

        \todo There is not yet a way to retrieve the database instance this has been given,
        to avoid giving users of the derived class full access to the database
        (and to avoid starting to mess with protected).
        Instead, derived classes can have their own m_db member.
        Revisit this decision if it proves cumbersome. */
    class Root {
     public:
        /** Constructor.
            \param db Database */
        explicit Root(afl::net::CommandHandler& db);

        /** Access root of user database.
            \return database node */
        afl::net::redis::Subtree userRoot();

        /** Access root of game database.
            \return database node */
        afl::net::redis::Subtree gameRoot();

        /** Access default user profile.
            \return default profile */
        afl::net::redis::HashKey defaultProfile();

     private:
        afl::net::CommandHandler& m_db;
    };

} }

#endif
