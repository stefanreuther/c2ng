/**
  *  \file server/console/contextfactory.hpp
  *  \brief Interface server::console::ContextFactory
  */
#ifndef C2NG_SERVER_CONSOLE_CONTEXTFACTORY_HPP
#define C2NG_SERVER_CONSOLE_CONTEXTFACTORY_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace server { namespace console {

    class Context;

    /** Context factory.
        This creates (short-lived) Context objects when requested by the user.
        A ContextFactory instance contains long-lived state of a context. */
    class ContextFactory : public afl::base::Deletable {
     public:
        /** Get name of command used to invoke this context.
            \return name */
        virtual String_t getCommandName() = 0;

        /** Create the associated context.
            \return newly allocated Context object. This object will be destroyed before the ContextFactory object. */
        virtual Context* create() = 0;

        /** Handle configuration option.
            \param key Key
            \param value Value
            \return true if option was processed */
        virtual bool handleConfiguration(const String_t& key, const String_t& value) = 0;
    };

} }

#endif
