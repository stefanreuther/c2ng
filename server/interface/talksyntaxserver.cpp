/**
  *  \file server/interface/talksyntaxserver.cpp
  *  \brief Class server::interface::TalkSyntaxServer
  */

#include <vector>
#include <stdexcept>
#include "server/interface/talksyntaxserver.hpp"
#include "server/interface/talksyntax.hpp"
#include "interpreter/arguments.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"

// Constructor.
server::interface::TalkSyntaxServer::TalkSyntaxServer(TalkSyntax& impl)
    : m_implementation(impl)
{ }

// Destructor.
server::interface::TalkSyntaxServer::~TalkSyntaxServer()
{ }

bool
server::interface::TalkSyntaxServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "SYNTAXGET") {
        /* @q SYNTAXGET key:Str (Talk Command)
           Get syntax coloring key.
           This queries the syntax coloring database that is usually used for rendering code.

           Valid keys:
           - <tt>ini.SECTION.NAME.info</tt>: help text for %NAME in section %SECTION.
           - <tt>ini.SECTION.NAME.link</tt>: link URL for %NAME in section %SECTION.

           Permissions: none.

           @err 404 Not found (key not found)
           @retval Str result */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(m_implementation.get(toString(args.getNext()))));
        return true;
    } else if (upcasedCommand == "SYNTAXMGET") {
        /* @q SYNTAXMGET key:Str... (Talk Command)
           Get syntax coloring keys.

           Permissions: none.

           @see SYNTAXGET
           @retval StrList results. If a key cannot be found, the result is reported as null; no error is generated. */
        std::vector<String_t> a;
        while (args.getNumArgs() > 0) {
            a.push_back(toString(args.getNext()));
        }
        result.reset(new afl::data::VectorValue(m_implementation.mget(a)));
        return true;
    } else {
        return false;
    }
}
