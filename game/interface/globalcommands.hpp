/**
  *  \file game/interface/globalcommands.hpp
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALCOMMANDS_HPP
#define C2NG_GAME_INTERFACE_GLOBALCOMMANDS_HPP

#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    bool checkPlayerSetArg(PlayerSet_t& result, afl::data::Value* value);

    void IFAddConfig(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFAddFCode(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFAddPref(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFAuthPlayer(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFCCSelectionExec(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFCreateConfigOption(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFCreatePrefOption(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFNewCircle(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFNewRectangle(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFNewRectangleRaw(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFNewLine(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFNewLineRaw(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFNewMarker(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFHistoryShowTurn(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFSaveGame(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFSendMessage(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);

} }


#endif
