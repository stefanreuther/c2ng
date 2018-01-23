/**
  *  \file server/console/routercontextfactory.hpp
  */
#ifndef C2NG_SERVER_CONSOLE_ROUTERCONTEXTFACTORY_HPP
#define C2NG_SERVER_CONSOLE_ROUTERCONTEXTFACTORY_HPP

#include "server/console/contextfactory.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/net/name.hpp"

namespace server { namespace console {

    /** Context for talking with c2router.
        This protocol differs from the other protocols.
        In particular, it follows an one-request-per-connection paradigm.

        This implements the very basic command/response scheme, in the same way as c2console-classic and c2scon.pl.
        It does not yet attempt to bring the commands into a sensible form for scripted post-processing.
        Those would be
        - convert "200" router replies into a list of strings
        - convert "200" session replies into objects (?)
        - convert "201" replies into a number (session number)
        - convert other replies into errors

        A minimal version of that transformation is in server::interface::SessionRouterClient
        which is used for machine/machine communication. */
    class RouterContextFactory : public ContextFactory {
     public:
        RouterContextFactory(String_t name, afl::net::NetworkStack& stack);
        ~RouterContextFactory();

        // ContextFactory:
        virtual String_t getCommandName();
        virtual Context* create();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);

     private:
        class Impl;

        String_t m_name;
        afl::net::Name m_address;
        afl::net::NetworkStack& m_networkStack;
    };

} }

#endif
