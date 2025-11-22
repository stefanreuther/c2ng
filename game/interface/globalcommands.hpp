/**
  *  \file game/interface/globalcommands.hpp
  *  \brief Global Commands
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALCOMMANDS_HPP
#define C2NG_GAME_INTERFACE_GLOBALCOMMANDS_HPP

#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    /** Parse player-set argument.
        Expects a
        - one-dimensional script array (IndexableValue) containing player numbers;
        - data array (VectorValue) containing player numbers;
        - scalar with a player number.

        @param [out] result   Result
        @param [in]  value    User-provided parameter
        @return true if parameter was given, false if parameter was null
        @throw interpreter::Error on type error */
    bool checkPlayerSetArg(PlayerSet_t& result, afl::data::Value* value);

    /** Make player-set value.
        @param set Set
        @return newly-allocated list. Null if set is empty. */
    afl::data::Value* makePlayerSet(PlayerSet_t set);


    /*
     *  Global Commands, provided to the script world using SimpleProcedure
     */

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
