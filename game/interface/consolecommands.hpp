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
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    void IFUIInput(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);

    /** Register console commands.
        Call this function only when operating in a console application.
        For a GUI application, use client::si::registerCommands(). */
    void registerConsoleCommands(Session& session, afl::base::Ref<afl::io::TextReader> in, afl::base::Ref<afl::io::TextWriter> out);

} }

#endif
