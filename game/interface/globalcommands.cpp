/**
  *  \file game/interface/globalcommands.cpp
  */

#include "game/interface/globalcommands.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/root.hpp"
#include "game/turnloader.hpp"

/* @q History.ShowTurn nr:Int (Global Command)
   Show turn from history database.

   The parameter specifies the turn number to load.
   The special case "0" will load the current turn.
   PCC2 will load the specified turn's result file, if available.

   @since PCC2 2.40 */
void
game::interface::IFHistoryShowTurn(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    // Check parameters
    args.checkArgumentCount(1);
    int32_t turn;
    if (!interpreter::checkIntegerArg(turn, args.getNext(), 0, 10000)) {
        return;
    }

    // Do we have a game loaded?
    Root* r = session.getRoot().get();
    Game* g = session.getGame().get();
    if (g == 0 || r == 0) {
        throw Exception(Exception::eUser, Exception::eUser);
    }

    // Check turn number
    int currentTurn = g->currentTurn().universe().getTurnNumber();
    if (turn == 0) {
        turn = currentTurn;
    }
    if (turn <= 0 || turn > currentTurn) {
        throw interpreter::Error::rangeError();
    }

    // Load if required
    if (turn < currentTurn) {
        // If turn is not known at all, update metainformation
        if (g->previousTurns().getTurnStatus(turn) == HistoryTurn::Unknown) {
            g->previousTurns().initFromTurnScores(g->scores(), turn, 1);
            if (TurnLoader* tl = r->getTurnLoader().get()) {
                g->previousTurns().initFromTurnLoader(*tl, *r, g->getViewpointPlayer(), turn, 1);
            }
        }

        // If turn is loadable, load
        HistoryTurn* ht = g->previousTurns().get(turn);
        if (ht != 0 && ht->isLoadable() && r->getTurnLoader().get() != 0) {
            // FIXME: code duplication to turnlistdialog.cpp
            try {
                afl::base::Ptr<Turn> t = new Turn();
                int player = g->getViewpointPlayer();
                r->getTurnLoader()->loadHistoryTurn(*t, *g, player, turn, *r);
                t->universe().postprocess(game::PlayerSet_t(player), game::PlayerSet_t(player), game::map::Object::ReadOnly,
                                          r->hostVersion(), r->hostConfiguration(),
                                          session.translator(), session.log());
                ht->handleLoadSucceeded(t);
            }
            catch (std::exception& e) {
                ht->handleLoadFailed();
            }
        }

        if (ht == 0 || ht->getStatus() != HistoryTurn::Loaded) {
            throw interpreter::Error("Turn not available");
        }
    }

    // Do it
    g->setViewpointTurnNumber(turn);
}
