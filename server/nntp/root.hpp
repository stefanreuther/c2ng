/**
  *  \file server/nntp/root.hpp
  *  \brief Class server::nntp::Root
  */
#ifndef C2NG_SERVER_NNTP_ROOT_HPP
#define C2NG_SERVER_NNTP_ROOT_HPP

#include "afl/sys/log.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace nntp {

    /** A NNTP server's root state.
        Contains global configuration and state objects.
        Root is shared between all connections. */
    class Root {
     public:
        /** Constructor.
            \param talk Talk connection
            \param baseUrl Base URL for links */
        Root(afl::net::CommandHandler& talk, const String_t& baseUrl);

        /** Access logger.
            \return logger */
        afl::sys::Log& log();

        /** Allocate an Id number.
            Returns a new number on every call.
            This is used to assign Ids to connections, for logging.
            \return Id */
        uint32_t allocateId();

        /** Access talk.
            \return talk connection */
        afl::net::CommandHandler& talk();

        /** Configure reconnection.
            Execute before every command that accesses the Talk service. */
        void configureReconnect();

        /** Get base URL.
            \return base URL */
        const String_t& getBaseUrl() const;

     private:
        afl::net::CommandHandler& m_talk;
        String_t m_baseUrl;
        afl::sys::Log m_log;
        uint32_t m_idCounter;
    };

} }

#endif
