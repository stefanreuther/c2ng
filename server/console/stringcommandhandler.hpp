/**
  *  \file server/console/stringcommandhandler.hpp
  *  \brief Class server::console::StringCommandHandler
  */
#ifndef C2NG_SERVER_CONSOLE_STRINGCOMMANDHANDLER_HPP
#define C2NG_SERVER_CONSOLE_STRINGCOMMANDHANDLER_HPP

#include "server/console/commandhandler.hpp"

namespace server { namespace console {

    /** String commands. */
    class StringCommandHandler : public CommandHandler {
     public:
        virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);
    };

} }

#endif
