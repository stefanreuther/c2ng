/**
  *  \file game/vcr/battle.cpp
  */

#include "game/vcr/battle.hpp"
#include "afl/string/format.hpp"
#include "game/vcr/object.hpp"

// /** Describe a battle. The idea is to say "<name> vs <name>" in 1:1 fights,
//     and "<race> vs <race>" in fleet battles with two participating races.
//     \param e the battle */
String_t
game::vcr::Battle::getDescription(const game::PlayerList& players, afl::string::Translator& tx)
{
    // ex client/dialogs/combatdiagram.cc:getBattleName
    int leftRace = 0;
    Object* leftSlot = 0;
    int rightRace = 0;
    Object* rightSlot = 0;

    for (size_t i = 0, n = getNumObjects(); i < n; ++i) {
        if (Object* p = getObject(i, false)) {
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
                return tx.translateString("Multiple races");
            }
        }
    }

    if (leftRace == 0 || rightRace == 0) {
        return tx.translateString("Unknown");
    } else {
        String_t leftName  = (leftSlot  != 0 ? leftSlot->getName()  : players.getPlayerName(leftRace, Player::ShortName));
        String_t rightName = (rightSlot != 0 ? rightSlot->getName() : players.getPlayerName(rightRace, Player::ShortName));
        return afl::string::Format(tx.translateString("%s vs. %s").c_str(), leftName, rightName);
    }
}
