/**
  *  \file game/vcr/battle.cpp
  *  \brief Base class game::vcr::Battle
  */

#include "game/vcr/battle.hpp"
#include "afl/string/format.hpp"
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
