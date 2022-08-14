/**
  *  \file server/interface/hosttoolserver.cpp
  *  \brief Class server::interface::HostToolServer
  */

#include <cstring>
#include <stdexcept>
#include "server/interface/hosttoolserver.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/errors.hpp"
#include "server/types.hpp"

namespace {
    bool parseUseFlag(interpreter::Arguments& args)
    {
        const String_t s = afl::string::strUCase(server::toString(args.getNext()));
        if (s == "USE") {
            return true;
        } else if (s == "SHOW") {
            return false;
        } else {
            throw std::runtime_error(server::INVALID_OPTION);
        }
    }
}

server::interface::HostToolServer::HostToolServer(HostTool& impl, HostTool::Area area)
    : ComposableCommandHandler(),
      m_implementation(impl),
      m_area(area)
{ }

server::interface::HostToolServer::~HostToolServer()
{ }

bool
server::interface::HostToolServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (isCommand(upcasedCommand, "ADD")) {
        /* @q HOSTADD id:Str path:FileName program:Str kind:Str (Host Command)
           Add host.

           Permissions: admin.

           @err 412 Invalid identifier
           @err 412 Invalid kind
           @err 412 Invalid executable

           @uses prog:host:prog:$HOST, prog:host:list */

        /* @q MASTERADD id:Str path:FileName program:Str kind:Str (Host Command)
           Add master.

           Permissions: admin.

           @err 412 Invalid identifier
           @err 412 Invalid kind
           @err 412 Invalid executable

           @uses prog:master:prog:$MASTER, prog:master:list */

        /* @q TOOLADD id:Str path:FileName program:Str kind:Str (Host Command)
           Add tool/add-on.
           %program can be the empty string if the tool does not have a primary program.

           Permissions: admin.

           @err 412 Invalid identifier
           @err 412 Invalid kind
           @err 412 Invalid executable

           @uses prog:tool:prog:$TOOL, prog:tool:list */

        /* @q SHIPLISTADD id:Str path:FileName program:Str kind:Str (Host Command)
           Add shiplist.

           Permissions: admin.

           @err 412 Invalid identifier
           @err 412 Invalid kind
           @err 412 Invalid executable

           %program should be the empty string as shiplists do not have a program.
           @uses prog:sl:prog:$SHIPLIST, prog:sl:list */
        args.checkArgumentCount(4);
        const String_t id   = toString(args.getNext());
        const String_t path = toString(args.getNext());
        const String_t prog = toString(args.getNext());
        const String_t kind = toString(args.getNext());
        m_implementation.add(id, path, prog, kind);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (isCommand(upcasedCommand, "SET")) {
        /* @q HOSTSET id:Str key:Str value:Str (Host Command), MASTERSET id:Str key:Str value:Str (Host Command), SHIPLISTSET id:Str key:Str value:Str (Host Command), TOOLSET id:Str key:Str value:Str (Host Command)
           Set host/master/shiplist/tool property.

           Permissions: admin.

           @err 412 Invalid identifier

           @uses prog:host:prog:$HOST, prog:tool:prog:$TOOL, prog:sl:prog:$SHIPLIST, prog:master:prog:$MASTER */
        args.checkArgumentCount(3);
        const String_t id  = toString(args.getNext());
        const String_t key = toString(args.getNext());
        const String_t val = toString(args.getNext());
        m_implementation.set(id, key, val);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (isCommand(upcasedCommand, "GET")) {
        /* @q HOSTGET id:Str key:Str (Host Command), MASTERGET id:Str key:Str (Host Command), SHIPLISTGET id:Str key:Str (Host Command), TOOLGET id:Str key:Str (Host Command)
           Get host/master/shiplist/tool property.

           Permissions: none.

           @retval Str property value
           @uses prog:host:prog:$HOST, prog:tool:prog:$TOOL, prog:sl:prog:$SHIPLIST, prog:master:prog:$MASTER */
        args.checkArgumentCount(2);
        const String_t id  = toString(args.getNext());
        const String_t key = toString(args.getNext());
        result.reset(makeStringValue(m_implementation.get(id, key)));
        return true;
    } else if (isCommand(upcasedCommand, "RM")) {
        /* @q HOSTRM id:Str (Host Command), MASTERRM id:Str (Host Command), SHIPLISTRM id:Str (Host Command), TOOLRM id:Str (Host Command)
           Remove a host/master/shiplist/tool.

           Permissions: admin.

           @uses prog:host:prog:$HOST, prog:tool:prog:$TOOL, prog:sl:prog:$SHIPLIST, prog:master:prog:$MASTER
           @uses prog:host:list, prog:tool:list, prog:sl:list, prog:master:list
           @retval Int (1=removed, 0=did not exist) */
        args.checkArgumentCount(1);
        const String_t id = toString(args.getNext());
        result.reset(makeIntegerValue(m_implementation.remove(id)));
        return true;
    } else if (isCommand(upcasedCommand, "LS")) {
        /* @q HOSTLS (Host Command), MASTERLS (Host Command), SHIPLISTLS (Host Command), TOOLLS (Host Command)
           List hosts/masters/shiplists/tools.

           Permissions: none.

           @retval HostToolInfo[] list of all requested components
           @uses prog:host:prog:$HOST, prog:tool:prog:$TOOL, prog:sl:prog:$SHIPLIST, prog:master:prog:$MASTER
           @uses prog:host:list, prog:tool:list, prog:sl:list, prog:master:list */
        /* @type HostToolInfo
           Description of a host/master/shiplist/tool.
           @key id:Str           (tool Id)
           @key description:Str  (user-friendly description)
           @key kind:Str         (tool kind)
           @key default:Int      (1 if this tool is the default) */
        // ex describeHost
        // @change c2host-classic does not verify argument count
        args.checkArgumentCount(0);
        std::vector<HostTool::Info> infos;
        m_implementation.getAll(infos);

        afl::base::Ref<afl::data::Vector> resultVector = afl::data::Vector::create();
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            resultVector->pushBackNew(packTool(infos[i]));
        }
        result.reset(new afl::data::VectorValue(resultVector));
        return true;
    } else if (isCommand(upcasedCommand, "CP")) {
        /* @q HOSTCP src:Str dest:Str (Host Command), MASTERCP src:Str dest:Str (Host Command), SHIPLISTCP src:Str dest:Str (Host Command), TOOLCP src:Str dest:Str (Host Command)
           Copy host/master/shiplist/tool.

           Permissions: admin.

           @err 404 Missing source (invalid %src)
           @err 412 Invalid identifier (invalid %dest)

           @uses prog:host:prog:$HOST, prog:tool:prog:$TOOL, prog:sl:prog:$SHIPLIST, prog:master:prog:$MASTER
           @uses prog:host:list, prog:tool:list, prog:sl:list, prog:master:list */
        args.checkArgumentCount(2);
        const String_t from = toString(args.getNext());
        const String_t to   = toString(args.getNext());
        m_implementation.copy(from, to);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (isCommand(upcasedCommand, "DEFAULT")) {
        /* @q HOSTDEFAULT id:Str (Host Command), MASTERDEFAULT id:Str (Host Command), SHIPLISTDEFAULT id:Str (Host Command)
           Set default host/master/shiplist.

           Permissions: admin.

           @err 404 Not found (%id does not exist)

           @uses prog:host:prog:$HOST, prog:tool:prog:$TOOL, prog:sl:prog:$SHIPLIST, prog:master:prog:$MASTER
           @uses prog:host:list, prog:tool:list, prog:sl:list, prog:master:list */
        // @change There is no "TOOLDEFAULT" normally.
        // We don't block it at the command parser level (and not at the client level).
        // If it is intended to be blocked, that must be done in the implementation;
        // letting it go through does not hurt either.
        // The setting is just ignored because a "default tool" does not make as much sense as a "default host".
        args.checkArgumentCount(1);
        m_implementation.setDefault(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (isCommand(upcasedCommand, "RATING")) {
        /* @q HOSTRATING id:Str {{SET n:Int | AUTO} {USE|SHOW}}|NONE|GET (Host Command), MASTERRATING id:Str {{SET n:Int | AUTO} {USE|SHOW}}|NONE|GET (Host Command), SHIPLISTRATING id:Str {{SET n:Int | AUTO} {USE|SHOW}}|NONE|GET (Host Command), TOOLRATING id:Str {{SET n:Int | AUTO} {USE|SHOW}}|NONE|GET (Host Command)
           Configure host/master/shiplist/tool rating.
           - "SET n" sets the rating to the given value.
           - "AUTO" resets the rating to the computed default value.<br/>
           SET or AUTO followed by "USE" actually uses the specified difficulty,
           followed by "SHOW" only shows it but internally uses the computed one.
           - "NONE" resets the difficulty.
           - "GET" does not modify the difficulty; instead, returns it.

           Permissions: admin.

           @err 404 Not found
           @err 400 Syntax error

           @retval Int new difficulty (unless NONE was used)
           @uses prog:host:prog:$HOST, prog:tool:prog:$TOOL, prog:sl:prog:$SHIPLIST, prog:master:prog:$MASTER */
        args.checkArgumentCountAtLeast(2);
        const String_t what = toString(args.getNext());
        const String_t op = afl::string::strUCase(toString(args.getNext()));
        if (op == "GET") {
            args.checkArgumentCount(0);
            result.reset(makeIntegerValue(m_implementation.getDifficulty(what)));
        } else if (op == "NONE") {
            args.checkArgumentCount(0);
            m_implementation.clearDifficulty(what);
            result.reset(makeStringValue("OK"));
        } else if (op == "AUTO") {
            args.checkArgumentCount(1);
            result.reset(makeIntegerValue(m_implementation.setDifficulty(what, afl::base::Nothing, parseUseFlag(args))));
        } else if (op == "SET") {
            args.checkArgumentCount(2);
            const int32_t value = toInteger(args.getNext());
            result.reset(makeIntegerValue(m_implementation.setDifficulty(what, value, parseUseFlag(args))));
        } else {
            throw std::runtime_error(INVALID_OPTION);
        }
        return true;
    } else {
        return false;
    }
}

bool
server::interface::HostToolServer::isCommand(const String_t& upcasedCommand, const char* suffix) const
{
    // Fetch prefix
    const char* prefix = HostTool::toString(m_area);
    if (prefix == 0) {
        // Cannot happen.
        return false;
    }

    const size_t suffixLength = std::strlen(suffix);
    const size_t prefixLength = std::strlen(prefix);
    return (prefixLength + suffixLength == upcasedCommand.size())
        && (upcasedCommand.compare(0, prefixLength, prefix, prefixLength) == 0)
        && (upcasedCommand.compare(prefixLength, suffixLength, suffix, suffixLength) == 0);
}

server::Value_t*
server::interface::HostToolServer::packTool(const HostTool::Info& tool)
{
    afl::base::Ref<afl::data::Hash> hash = afl::data::Hash::create();
    hash->setNew("id",          makeStringValue(tool.id));
    hash->setNew("description", makeStringValue(tool.description));
    hash->setNew("kind",        makeStringValue(tool.kind));
    hash->setNew("default",     makeIntegerValue(tool.isDefault));
    return new afl::data::HashValue(hash);
}
