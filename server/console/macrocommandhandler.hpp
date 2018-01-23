/**
  *  \file server/console/macrocommandhandler.hpp
  *  \brief Class server::console::MacroCommandHandler
  */
#ifndef C2NG_SERVER_CONSOLE_MACROCOMMANDHANDLER_HPP
#define C2NG_SERVER_CONSOLE_MACROCOMMANDHANDLER_HPP

#include <map>
#include "server/console/commandhandler.hpp"

namespace server { namespace console {

    class Environment;

    /** Macro commands.
        This implements macros.
        The macro definitions are stored as members of the MacroCommandHandler object;
        thus, this object should be long-lived. */
    class MacroCommandHandler : public CommandHandler {
     public:
        /** Constructor.
            \param env Environment */
        explicit MacroCommandHandler(Environment& env);

        /** Destructor. */
        ~MacroCommandHandler();

        // CommandHandler:
        virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);

     private:
        std::map<String_t, String_t> m_macros;
        Environment& m_environment;
    };

} }

#endif
