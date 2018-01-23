/**
  *  \file server/console/commandhandler.hpp
  *  \brief Interface server::console::CommandHandler
  */
#ifndef C2NG_SERVER_CONSOLE_COMMANDHANDLER_HPP
#define C2NG_SERVER_CONSOLE_COMMANDHANDLER_HPP

#include <memory>
#include "afl/base/deletable.hpp"
#include "afl/data/value.hpp"
#include "interpreter/arguments.hpp"

namespace server { namespace console {

    class Parser;

    /** Interface for a console command handler. */
    class CommandHandler : public afl::base::Deletable {
     public:
        /** Execute a command.
            \param cmd    [in] Command verb
            \param args   [in] Remaining arguments
            \param parser [in] Invoking parser instance (could be used for recursive invocation)
            \param result [out] Result. Pass in as null.
            \retval true Command was executed; result has been set.
            \retval false Command was not recognized; result not changed */
        virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result) = 0;
    };

} }

#endif
