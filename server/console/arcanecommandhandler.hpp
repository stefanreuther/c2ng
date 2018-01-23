/**
  *  \file server/console/arcanecommandhandler.hpp
  *  \brief Class server::console::ArcaneCommandHandler
  */
#ifndef C2NG_SERVER_CONSOLE_ARCANECOMMANDHANDLER_HPP
#define C2NG_SERVER_CONSOLE_ARCANECOMMANDHANDLER_HPP

#include "server/console/commandhandler.hpp"

namespace server { namespace console {

    class Environment;

    /** Arcane commands.
        The commands in this CommandHandler replicate commands in c2console-classic.
        They are more arcane and considered deprecated. */
    class ArcaneCommandHandler : public CommandHandler {
     public:
        /** Constructor.
            \param env Environment
            \param CommandHandler Main command handler for recursive command invocation */
        ArcaneCommandHandler(Environment& env, CommandHandler& recurse);

        /** Destructor. */
        ~ArcaneCommandHandler();

        // CommandHandler:
        virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);

     private:
        Environment& m_environment;
        CommandHandler& m_recurse;
    };

} }

#endif
