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

    void IFAddConfig(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFAddFCode(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFAddPref(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFAuthPlayer(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFCCHistoryShowTurn(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFCCSelectionExec(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFCreateConfigOption(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFCreatePrefOption(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFExport(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFNewCannedMarker(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFNewCircle(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFNewRectangle(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFNewRectangleRaw(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFNewLine(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFNewLineRaw(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFNewMarker(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFHistoryLoadTurn(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFSaveGame(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFSendMessage(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);

} }


#endif
