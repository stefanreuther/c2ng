/**
  *  \file server/console/filecommandhandler.cpp
  *  \brief Class server::console::FileCommandHandler
  */

#include "server/console/filecommandhandler.hpp"
#include "server/types.hpp"
#include "afl/io/stream.hpp"

server::console::FileCommandHandler::FileCommandHandler(afl::io::FileSystem& fs)
    : CommandHandler(),
      m_fileSystem(fs)
{ }

bool
server::console::FileCommandHandler::call(const String_t& cmd, interpreter::Arguments args, Parser& /*parser*/, std::auto_ptr<afl::data::Value>& result)
{
    if (cmd == "file_exists") {
        /* @q file_exists FILENAME:Str... (Global Console Command)
           Check existence of the given files.
           Returns true (nonzero) if all of them exist, zero if one doesn't.
           @since PCC2 1.99.18, PCC2 2.40.3 */
        int all = 1;
        while (args.getNumArgs() > 0) {
            if (m_fileSystem.openFileNT(toString(args.getNext()), afl::io::FileSystem::OpenRead).get() == 0) {
                all = 0;
                break;
            }
        }
        result.reset(makeIntegerValue(all));
        return true;
    } else {
        return false;
    }
}
