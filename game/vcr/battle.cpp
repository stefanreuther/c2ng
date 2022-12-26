/**
  *  \file game/vcr/battle.cpp
  *  \brief Base class game::vcr::Battle
  */

#include "game/vcr/battle.hpp"
#include "afl/string/format.hpp"
#include "game/root.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/object.hpp"

String_t
game::vcr::Battle::getDescription(const game::PlayerList& players, afl::string::Translator& tx) const
{
    // ex client/dialogs/combatdiagram.cc:getBattleName
    int leftRace = 0;
    const Object* leftSlot = 0;
    int rightRace = 0;
    const Object* rightSlot = 0;

    for (size_t i = 0, n = getNumObjects(); i < n; ++i) {
        if (const Object* p = getObject(i, false)) {
            int pl = p->getOwner();
            if (leftRace == 0) {
                leftRace = pl;
                leftSlot = p;
            } else if (leftRace == pl) {
                leftSlot = 0;
            } else if (rightRace == 0) {
                rightRace = pl;
                rightSlot = p;
            } else if (rightRace == pl) {
                rightSlot = 0;
            } else {
                return tx("Multiple races");
            }
        }
    }

    if (leftRace == 0 || rightRace == 0) {
        return tx("Unknown");
    } else {
        String_t leftName  = (leftSlot  != 0 ? leftSlot->getName()  : players.getPlayerName(leftRace,  Player::ShortName, tx));
        String_t rightName = (rightSlot != 0 ? rightSlot->getName() : players.getPlayerName(rightRace, Player::ShortName, tx));
        return afl::string::Format(tx("%s vs. %s"), leftName, rightName);
    }
}

void
game::vcr::Battle::getBattleInfo(BattleInfo& out,
                                 const TeamSettings* teamSettings,
                                 const game::spec::ShipList& shipList,
                                 const Root& root,
                                 afl::string::Translator& tx) const
{
    const int me = teamSettings != 0 ? teamSettings->getViewpointPlayer() : 0;

    out.units.clear();
    for (size_t i = 0, n = getNumObjects(); i < n; ++i) {
        if (const game::vcr::Object* obj = getObject(i, false)) {
            out.units.push_back(obj->describe(teamSettings, &root, &shipList, tx));
        }
    }

    out.groups.clear();
    for (size_t i = 0, n = getNumGroups(); i < n; ++i) {
        out.groups.push_back(getGroupInfo(i, root.hostConfiguration()));
    }

    out.seed          = getAuxiliaryInformation(Battle::aiSeed);
    out.algorithmName = getAlgorithmName(tx);
    out.resultSummary = getResultSummary(me, root.hostConfiguration(), shipList, root.userConfiguration().getNumberFormatter(), tx);
    out.position      = getPosition();
}
