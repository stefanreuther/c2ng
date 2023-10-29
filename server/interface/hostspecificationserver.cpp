/**
  *  \file server/interface/hostspecificationserver.cpp
  *  \brief Class server::interface::HostSpecificationServer
  */

#include "server/interface/hostspecificationserver.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"

namespace {
    /*
     *  Common part of parameter parsing
     */
    using server::interface::HostSpecification;

    struct Args {
        HostSpecification::Format format;
        afl::data::StringList_t keys;
    };

    void parseArgs(Args& out, interpreter::Arguments& args)
    {
        // Format
        afl::base::Optional<HostSpecification::Format> fmt = HostSpecification::parseFormat(server::toString(args.getNext()));
        if (!fmt.isValid()) {
            throw std::runtime_error(server::INVALID_OPTION);
        }
        out.format = *fmt.get();

        // Keys
        while (args.getNumArgs() > 0) {
            out.keys.push_back(server::toString(args.getNext()));
        }
    }
}

server::interface::HostSpecificationServer::HostSpecificationServer(HostSpecification& impl)
    : m_implementation(impl)
{ }

server::interface::HostSpecificationServer::~HostSpecificationServer()
{ }

bool
server::interface::HostSpecificationServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "SPECSHIPLIST") {
        /* @q SPECSHIPLIST sl:Str fmt:Str key:Str... (Host Command)
           Get ship list data.
           Retrieves data for one ship list.
           The %format parameter selects the output format (direct: regular return; json: stringified JSON).
           The given %key parameters select the data to be returned.
           At least one key needs to be given.

           Permissions: none.

           @err 404 Ship list not found
           @err 401 Invalid key */
        args.checkArgumentCountAtLeast(3);
        String_t sl = toString(args.getNext());
        Args a;
        parseArgs(a, args);
        result.reset(m_implementation.getShiplistData(sl, a.format, a.keys));
        return true;
    } else if (upcasedCommand == "SPECGAME") {
        /* @q SPECGAME gid:GID fmt:Str key:Str... (Host Command)
           Get ship list data for a hosted game.
           Retrieves data for one game.
           The %format parameter selects the output format (direct: regular return; json: stringified JSON).
           The given %key parameters select the data to be returned.
           At least one key needs to be given.

           Permissions: none.

           @err 404 Ship list not found
           @err 401 Invalid key */
        args.checkArgumentCountAtLeast(3);
        int32_t gid = toInteger(args.getNext());
        Args a;
        parseArgs(a, args);
        result.reset(m_implementation.getGameData(gid, a.format, a.keys));
        return true;
    } else {
        return false;
    }
}
