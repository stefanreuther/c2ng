/**
  *  \file server/console/consoleapplication.hpp
  *  \brief Class server::console::ConsoleApplication
  */
#ifndef C2NG_SERVER_CONSOLE_CONSOLEAPPLICATION_HPP
#define C2NG_SERVER_CONSOLE_CONSOLEAPPLICATION_HPP

#include <memory>
#include "afl/container/ptrvector.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/net/tunnel/tunnelablenetworkstack.hpp"
#include "server/configurationhandler.hpp"
#include "server/console/colorterminal.hpp"
#include "server/console/commandhandler.hpp"
#include "server/console/context.hpp"
#include "server/console/dumbterminal.hpp"
#include "server/console/environment.hpp"
#include "server/console/macrocommandhandler.hpp"
#include "server/types.hpp"
#include "util/application.hpp"

namespace server { namespace console {

    class Terminal;
    class ContextFactory;

    /** c2console main application. */
    class ConsoleApplication : public util::Application,
                               private ConfigurationHandler,
                               private CommandHandler
    {
     public:
        typedef std::auto_ptr<afl::data::Value> ValuePtr_t;

        /** Constructor.
            \param env Environment
            \param fs File system
            \param net Network stack */
        ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net);

        /** Destructor. */
        ~ConsoleApplication();

        /** Application enty point. */
        virtual void appMain();

        /** Get a ContextFactory, given a name.
            \param name Name
            \return ContextFactory if one exists; null otherwise */
        ContextFactory* getContextFactoryByName(String_t name);

        /** Enter a new context.
            \param ctx Newly-allocated context, must not be null */
        void pushNewContext(Context* ctx);

     private:
        /** Display help and exit. */
        void help();

        /** Evaluate a text stream, possibly interactively.
            Repeatedly reads commands from the input stream,
            printing prompts using the given terminal, and executes them.
            \param in Input stream
            \param term Output terminal */
        void evaluateInteractive(Terminal& term, afl::io::TextReader& in);


        // ConfigurationHandler:
        virtual bool handleConfiguration(const String_t& key, const String_t& value);

        // CommandHandler
        virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);


        /** Network stack instance. */
        afl::net::tunnel::TunnelableNetworkStack m_networkStack;

        /** Available Contexts.
            (Must be before m_contextStack to satisfy the guarantee that Context objects don't outlive their ContextFactory.) */
        afl::container::PtrVector<ContextFactory> m_availableContexts;

        /** Environment variables. */
        Environment m_environment;

        /** Active contexts. */
        ContextStack_t m_contextStack;

        /** Macros.
            This CommandHandler must be long-lived because it stores state. */
        MacroCommandHandler m_macros;
    };

} }

#endif
