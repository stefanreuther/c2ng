/**
  *  \file game/vcr/classic/utils.cpp
  */

#include "game/vcr/classic/utils.hpp"
#include "game/vcr/classic/database.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"

using afl::string::Format;

game::vcr::classic::Database*
game::vcr::classic::getDatabase(Session& s)
{
    if (Game* g = s.getGame().get()) {
        return dynamic_cast<Database*>(g->currentTurn().getBattles().get());
    } else {
        return 0;
    }
}

// FIXME: this function is badly designed (game/ should not know about resource Ids)
// String_t
// game::vcr::classic::getImageResourceId(const Object& object, Side side, const game::spec::ShipList& shipList)
// {
//     if (object.isPlanet()) {
//         return "vcr.planet";
//     } else {
//         return afl::string::Format("%s.%d",
//                                    (side==LeftSide ? "vcr.lship" : "vcr.rship"),
//                                    object.getGuessedShipPicture(shipList.hulls()));
//     }
// }

String_t
game::vcr::classic::formatBattleResult(BattleResult_t result,
                                       const String_t& leftName, TeamSettings::Relation leftRelation,
                                       const String_t& rightName, TeamSettings::Relation rightRelation,
                                       const String_t& annotation,
                                       afl::string::Translator& tx)
{
    // FIXME: do we need this method, and do we need this signature?
    int me;
    if (leftRelation == TeamSettings::ThisPlayer) {
        me = 0;
    } else if (rightRelation == TeamSettings::ThisPlayer) {
        me = 1;
    } else {
        me = 2;
    }

    // Build annotation
    String_t insert;
    if (!annotation.empty()) {
        insert = Format(" (%s)", annotation);
    }

    String_t s;
    if (result.empty()) {
        s = tx("unknown. Wait while computing...");
    } else if (result == Invalid) {
        s = tx("Battle cannot be played!");
    } else if (result == Timeout) {
        s = tx("Battle timed out (too long).");
    } else if (result == Stalemate) {
        s = tx("Stalemate.");
    } else if (result == LeftDestroyed) {
        if (me == 0) {
            s = Format(tx("We were destroyed%s.").c_str(), insert);
        } else if (me == 1) {
            s = Format(tx("We won%s.").c_str(), insert);
        } else {
            s = Format(tx("%s won%s.").c_str(), rightName, insert);
        }
    } else if (result == RightDestroyed) {
        if (me == 1) {
            s = Format(tx("We were destroyed%s.").c_str(), insert);
        } else if (me == 0) {
            s = Format(tx("We won%s.").c_str(), insert);
        } else {
            s = Format(tx("%s won%s.").c_str(), leftName, insert);
        }
    } else if (result == LeftCaptured) {
        if (me == 0) {
            s = Format(tx("They have captured our ship%s.").c_str(), insert);
        } else if (me == 1) {
            s = Format(tx("We captured their ship%s.").c_str(), insert);
        } else {
            s = Format(tx("%s was captured.").c_str(), leftName);
        }
    } else if (result == RightCaptured) {
        if (me == 1) {
            s = Format(tx("They have captured our ship%s.").c_str(), insert);
        } else if (me == 0) {
            s = Format(tx("We captured their ship%s.").c_str(), insert);
        } else {
            s = Format(tx("%s was captured.").c_str(), rightName);
        }
    } else if (result == BattleResult_t(LeftDestroyed) + RightDestroyed) {
        s = tx("Both were destroyed.");
    } else {
        s = tx("Both are disabled.");
    }
    return s;
}
