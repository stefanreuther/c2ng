/**
  *  \file server/console/dbexporter.hpp
  *  \brief Database Export
  */
#ifndef C2NG_SERVER_CONSOLE_DBEXPORTER_HPP
#define C2NG_SERVER_CONSOLE_DBEXPORTER_HPP

#include "afl/net/commandhandler.hpp"
#include "interpreter/arguments.hpp"

namespace server { namespace console {

    class Terminal;

    /** Export database.
        @param out          Output receiver
        @param dbConnection Database connection
        @param commandLine  Command line, parsed for options and values to export. */
    void exportDatabase(Terminal& out,
                        afl::net::CommandHandler& dbConnection,
                        interpreter::Arguments& commandLine);

} }

#endif
