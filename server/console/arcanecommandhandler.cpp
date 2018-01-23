/**
  *  \file server/console/arcanecommandhandler.cpp
  *  \brief Class server::console::ArcaneCommandHandler
  */

#include <stdexcept>
#include "afl/string/format.hpp"
#include "server/console/arcanecommandhandler.hpp"
#include "server/console/environment.hpp"
#include "server/types.hpp"

server::console::ArcaneCommandHandler::ArcaneCommandHandler(Environment& env, CommandHandler& recurse)
    : CommandHandler(),
      m_environment(env),
      m_recurse(recurse)
{ }

server::console::ArcaneCommandHandler::~ArcaneCommandHandler()
{ }

bool
server::console::ArcaneCommandHandler::call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result)
{
    if (cmd == "checkenv") {
        /* @q checkenv VAR:Env... (Global Console Command)
           Check presence of environment variables.
           Takes a list of environment variable names, and fails if at least one of them is not set.
           @since PCC2 1.99.18, PCC2 2.40.3*/
        while (args.getNumArgs() > 0) {
            String_t name = toString(args.getNext());
            if (m_environment.get(name) == 0) {
                throw std::runtime_error(afl::string::Format("Required environment variable '${%s}' is missing", name));
            }
        }
        return true;
    } else if (cmd == "ifset") {
        /* @q ifset VAR:Env COMMAND ARGS... (Global Console Command)
           Execute COMMAND if variable is set.
           Note that this takes the command as a word list ("ifset X echo Hi!"),
           not a (brace-quoted) string like the regular {if} does.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        args.checkArgumentCountAtLeast(2);
        String_t name = toString(args.getNext());
        String_t recursiveCommand = toString(args.getNext());
        if (m_environment.get(name) != 0) {
            if (!m_recurse.call(recursiveCommand, args, parser, result)) {
                throw std::runtime_error(afl::string::Format("Unknown command '%s'", recursiveCommand));
            }
        }
        return true;
    } else {
        return false;
    }
}
