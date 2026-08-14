/**
  *  \file server/console/clientpool.hpp
  *  \brief Class server::console::ClientPool
  */
#ifndef C2NG_SERVER_CONSOLE_CLIENTPOOL_HPP
#define C2NG_SERVER_CONSOLE_CLIENTPOOL_HPP

#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/net/resp/client.hpp"

namespace server { namespace console {

    /** Pool of RESP clients.
        Stores a set of afl::net::resp::Client objects and metadata for it.
        Handles configuration and connect/reconnect.

        To use,
        - call create() to add clients, find() to retrieve previously-added clients
        - call handleConfiguration() to assimilate configuration options
        - call get() to retrieve a client, connecting it if required
        - call reset() to force-disconnect a client */
    class ClientPool {
     public:
        /** Type for an index to identify a client. */
        typedef size_t Index_t;

        /** Constructor.
            Makes a ClientPool that initially has no clients. */
        explicit ClientPool(afl::net::NetworkStack& stack);

        /** Destructor. */
        ~ClientPool();

        /** Create a client.
            @param name Name
            @param defaultPort Default port
            @return handle to access this client */
        Index_t create(String_t name, uint16_t defaultPort);

        /** Find client by name.
            @param name Name
            @return handle to this client
            @throw AssertionFailedException if name does not exist */
        Index_t find(String_t name) const;

        /** Get client by index.
            Auto-creates (and thus, connects) this client if that hasn't been done before.
            Connection failures are propagated as exceptions (typically, FileProblemException)-
            @param index Handle
            @return client
            @throw AssertionFailedException if index is invalid */
        afl::net::resp::Client& get(Index_t index);

        /** Reset a client by index.
            The next call to get() will reconnect.
            If the index is invalid, this is a no-op.
            @param index Handle */
        void reset(Index_t index);

        /** Handle a configuration option.
            @param key Name of configuration option
            @param value Value
            @retval true Option recognized
            @retval false Option not recognized. If given on the command line, this is a hard error; in a config file, this is ignored.
            @see server::ConfigurationHandler::handleConfiguration */
        bool handleConfiguration(const String_t& key, const String_t& value);

     private:
        struct Entry;

        afl::net::NetworkStack& m_networkStack;
        afl::container::PtrVector<Entry> m_entries;
    };

} }

#endif
