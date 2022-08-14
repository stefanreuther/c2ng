/**
  *  \file server/interface/hostkeyclient.hpp
  *  \brief Class server::interface::HostKeyClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTKEYCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTKEYCLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/hostkey.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Client for host key store.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostKeyClient : public HostKey {
     public:
        /** Constructor.
            \param commandHandler Server connection. Lifetime must exceed that of the HostKeyClient. */
        explicit HostKeyClient(afl::net::CommandHandler& commandHandler);

        /** Destructor. */
        ~HostKeyClient();

        // HostKey:
        virtual void listKeys(Infos_t& out);
        virtual String_t getKey(String_t keyId);

        /** Unpack a serialized Info structure.
            \param p Value received from server */
        static Info unpackInfo(const Value_t* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };


} }

#endif
