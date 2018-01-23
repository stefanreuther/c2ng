/**
  *  \file server/dbexport/dbexporter.hpp
  *  \brief Function server::dbexport::exportDatabase
  */
#ifndef C2NG_SERVER_DBEXPORT_DBEXPORTER_HPP
#define C2NG_SERVER_DBEXPORT_DBEXPORTER_HPP

#include "afl/io/textwriter.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/sys/commandlineparser.hpp"

namespace server { namespace dbexport {

    /** Export database.
        \param out          Output receiver
        \param dbConnection Database connection
        \param commandLine  Command line, parsed for options and values to export. */
    void exportDatabase(afl::io::TextWriter& out,
                        afl::net::CommandHandler& dbConnection,
                        afl::sys::CommandLineParser& commandLine);

} }

#endif
