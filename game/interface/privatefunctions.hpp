/**
  *  \file game/interface/privatefunctions.hpp
  *  \brief Class game::interface::PrivateFunctions
  */
#ifndef C2NG_GAME_INTERFACE_PRIVATEFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_PRIVATEFUNCTIONS_HPP

#include "game/browser/session.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "util/requestsender.hpp"

namespace game { namespace interface {

    /** Private functions.
        These functions are NOT exported to scripts, they are only used internally.
        This means their interface is NOT stable.

        These functions are used during startup, to use the interpreter for sequencing.
        To invoke them, create a BytecodeObject, call the appropriate add() function,
        and run the BytecodeObject in a process.

        These functions shall be called from the thread that owns the Session parameter. */
    class PrivateFunctions {
     public:
        /** Create code to take over a browser's current directory's Root into a game session.
            @param session        Session
            @param bco            BytecodeObject
            @param gameSender     RequestSender to access session
            @param browserSender  RequestSender to access browser */
        static void addTakeRoot(Session& session, interpreter::BytecodeObject& bco, util::RequestSender<game::Session> gameSender, util::RequestSender<game::browser::Session> browserSender);

        /** Create code to call Session::setGame() with a new Game.
            @param session Session
            @param bco     BytecodeObject */
        static void addMakeGame(Session& session, interpreter::BytecodeObject& bco);

        /** Create code to call Session::setShipList() with a new ShipList.
            @param session Session
            @param bco     BytecodeObject */
        static void addMakeShipList(Session& session, interpreter::BytecodeObject& bco);

        /** Create code to call SpecificationLoader::loadShipList() on the session's Root/ShipList.
            @param session Session
            @param bco     BytecodeObject */
        static void addLoadShipList(Session& session, interpreter::BytecodeObject& bco);

        /** Create code to call TurnLoader::loadCurrentTurn() on the session's Root/Game.
            @param session Session
            @param bco     BytecodeObject
            @param player  Player number */
        static void addLoadCurrentTurn(Session& session, interpreter::BytecodeObject& bco, int player);

        /** Create code to postprocess the current turn in session's Root/Game.
            @param session Session
            @param bco     BytecodeObject
            @param player  Player */
        static void addPostprocessCurrentTurn(Session& session, interpreter::BytecodeObject& bco, int player);

     private:
        static void IFTakeRoot(Session& session, interpreter::Process& proc, interpreter::Arguments& args);
        static void IFMakeGame(Session& session, interpreter::Process& proc, interpreter::Arguments& args);
        static void IFMakeShipList(Session& session, interpreter::Process& proc, interpreter::Arguments& args);
        static void IFLoadShipList(Session& session, interpreter::Process& proc, interpreter::Arguments& args);
        static void IFLoadCurrentTurn(Session& session, interpreter::Process& proc, interpreter::Arguments& args);
        static void IFPostprocessCurrentTurn(Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    };
} }

#endif
