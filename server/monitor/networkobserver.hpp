/**
  *  \file server/monitor/networkobserver.hpp
  *  \brief Class server::monitor::NetworkObserver
  */
#ifndef C2NG_SERVER_MONITOR_NETWORKOBSERVER_HPP
#define C2NG_SERVER_MONITOR_NETWORKOBSERVER_HPP

#include "server/monitor/networkobserver.hpp"
#include "server/monitor/statusobserver.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/net/name.hpp"

namespace server { namespace monitor {

    /** Observer for a network service. */
    class NetworkObserver : public StatusObserver {
     public:
        /** Flavor of service to observer. */
        enum Flavor {
            /** Web server (but not c2monitor).
                Reports Running if answers a "GET /" with a "200 OK". */
            Web,

            /** Router server.
                Reports Running if it successfully answers a LIST command. */
            Router,

            /** Redis server.
                Reports Running if it successfully answers to a PING command.
                (Redis behaves a little different than the PlanetsCentral servers.) */
            Redis,

            /** Generic PlanetsCentral server.
                Reports Running if it successfully answers to a PING command. */
            Service
        };

        /** Constructor.
            \param name User-friendly name
            \param identifier Identifier (prefix for configuration)
            \param flavor Flavor of service.
            \param net NetworkStack instance
            \param defaultAddress Default address if none configured */
        NetworkObserver(String_t name, String_t identifier, Flavor flavor, afl::net::NetworkStack& net, afl::net::Name defaultAddress);

        /** Destructor. */
        ~NetworkObserver();

        // Observer:
        virtual String_t getName();
        virtual String_t getId();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);
        virtual Status checkStatus();

     private:
        String_t m_name;
        String_t m_identifier;
        Flavor m_flavor;
        afl::net::NetworkStack& m_networkStack;
        afl::net::Name m_address;
    };

} }

#endif
