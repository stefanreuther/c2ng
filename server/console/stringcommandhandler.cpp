/**
  *  \file server/console/stringcommandhandler.cpp
  *  \brief Class server::console::StringCommandHandler
  */

#include "server/console/stringcommandhandler.hpp"
#include "server/types.hpp"

bool
server::console::StringCommandHandler::call(const String_t& cmd, interpreter::Arguments args, Parser& /*parser*/, std::auto_ptr<afl::data::Value>& result)
{
    if (cmd == "str") {
        /* @q str ARG (Global Console Command)
           Convert the argument into a string and return that.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(toString(args.getNext())));
        return true;
    } else if (cmd == "str_eq") {
        /* @q str_eq A:Str B:Str (Global Console Command)
           Compare two strings.
           Returns nonzero (true) of both are equal.
           @since PCC2 1.99.21, PCC2 2.40.3 */
        args.checkArgumentCount(2);
        String_t a = toString(args.getNext());
        String_t b = toString(args.getNext());
        result.reset(makeIntegerValue(a == b));
        return true;
    } else if (cmd == "str_empty") {
        /* @q str_empty STR:Str... (Global Console Command)
           Returns nonzero (true) if all arguments are empty.
           @since PCC2 1.99.19, PCC2 2.40.3 */
        bool eq = true;
        while (args.getNumArgs() > 0) {
            if (!toString(args.getNext()).empty()) {
                eq = false;
                break;
            }
        }
        // @change PCC2 returns boolean (and that is the only boolean used in c2console).
        // Returning an integer is more consistent with the rest.
        result.reset(makeIntegerValue(eq));
        return true;
    } else {
        return false;
    }
}
