/**
  *  \file server/console/integercommandhandler.hpp
  *  \brief Class server::console::IntegerCommandHandler
  */
#ifndef C2NG_SERVER_CONSOLE_INTEGERCOMMANDHANDLER_HPP
#define C2NG_SERVER_CONSOLE_INTEGERCOMMANDHANDLER_HPP

#include "server/console/commandhandler.hpp"

namespace server { namespace console {

    /** Integer commands. */
    class IntegerCommandHandler : public CommandHandler {
     public:
        virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);
    };

} }

#endif
