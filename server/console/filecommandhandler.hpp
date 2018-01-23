/**
  *  \file server/console/filecommandhandler.hpp
  *  \brief Class server::console::FileCommandHandler
  */
#ifndef C2NG_SERVER_CONSOLE_FILECOMMANDHANDLER_HPP
#define C2NG_SERVER_CONSOLE_FILECOMMANDHANDLER_HPP

#include "server/console/commandhandler.hpp"
#include "afl/io/filesystem.hpp"

namespace server { namespace console {

    /** File commands. */
    class FileCommandHandler : public CommandHandler {
     public:
        explicit FileCommandHandler(afl::io::FileSystem& fs);
        virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);

     private:
        afl::io::FileSystem& m_fileSystem;
    };

} }

#endif
