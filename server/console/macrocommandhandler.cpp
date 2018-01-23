/**
  *  \file server/console/macrocommandhandler.cpp
  *  \brief Class server::console::MacroCommandHandler
  */

#include "server/console/macrocommandhandler.hpp"
#include "server/console/environment.hpp"
#include "server/console/parser.hpp"
#include "server/types.hpp"

server::console::MacroCommandHandler::MacroCommandHandler(Environment& env)
    : CommandHandler(),
      m_macros(),
      m_environment(env)
{ }

server::console::MacroCommandHandler::~MacroCommandHandler()
{ }

bool
server::console::MacroCommandHandler::call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result)
{
    std::map<String_t, String_t>::iterator it = m_macros.find(cmd);
    if (it != m_macros.end()) {
        // Macro invocation. Pass positional parameters.
        // @since PCC2 1.99.19
        afl::data::Segment posParams;
        while (args.getNumArgs() > 0) {
            posParams.pushBack(args.getNext());
        }

        Environment::SegmentPtr_t save(m_environment.pushPositionalParameters(posParams));

        // Call it.
        try {
            parser.evaluateString(it->second, result);
            m_environment.popPositionalParameters(save);
        }
        catch (...) {
            m_environment.popPositionalParameters(save);
            throw;
        }
        return true;
    } else if (cmd == "macro") {
        /* @q macro NAME BODY:Code (Global Console Command)
           Define a macro.
           The macro can later be invoked by using its name as a command verb.
           Within the macro body, positional arguments can be referred as $1, $2 etc.
           @since PCC2 1.99.19, PCC2 2.40.3 */
        args.checkArgumentCount(2);
        String_t macroName = toString(args.getNext());
        String_t macroText = toString(args.getNext());
        m_macros[macroName] = macroText;
        return true;
    } else {
        return false;
    }
}
