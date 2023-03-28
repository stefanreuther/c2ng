/**
  *  \file game/interface/consolecommands.hpp
  *  \brief Console Commands
  */
#ifndef C2NG_GAME_INTERFACE_CONSOLECOMMANDS_HPP
#define C2NG_GAME_INTERFACE_CONSOLECOMMANDS_HPP

#include "afl/base/ref.hpp"
#include "afl/io/textreader.hpp"
#include "afl/io/textwriter.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    /** Register console commands.
        Call this function only when operating in a console application.
        For a GUI application, use client::si::registerCommands().

        Console commands bypass the regular logger and work directly on the program's standard input/output.

        @param session Session
        @param in      Standard input stream
        @param out     Standard output stream */
    void registerConsoleCommands(Session& session, afl::base::Ref<afl::io::TextReader> in, afl::base::Ref<afl::io::TextWriter> out);

} }

#endif
