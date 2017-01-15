/**
  *  \file game/interface/vcrproperty.cpp
  */

#include "game/interface/vcrproperty.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/interface/vcrsidefunction.hpp"
#include "game/turn.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/database.hpp"
#include "interpreter/values.hpp"

using interpreter::makeIntegerValue;
using interpreter::makeStringValue;

// /** Get property of a VCR record.
//     \param vcr The record
//     \param ivp Property to get */
afl::data::Value*
game::interface::getVcrProperty(size_t battleNumber,
                                VcrProperty ivp,
                                Session& session,
                                afl::base::Ref<Root> root,     // for PlayerList
                                afl::base::Ref<Turn> turn,     // for Turn
                                afl::base::Ref<game::spec::ShipList> shipList)
{
    // ex int/if/vcrif.h:getVcrProperty
    game::vcr::Battle* battle;
    if (game::vcr::Database* db = turn->getBattles().get()) {
        battle = db->getBattle(battleNumber);
    } else {
        battle = 0;
    }
    game::vcr::classic::Battle* classic = dynamic_cast<game::vcr::classic::Battle*>(battle);

    switch (ivp) {
     case ivpSeed:
        /* @q Seed:Int (Combat Property)
           Random number seed.
           Valid only for classic combat, EMPTY for others. */
        if (classic != 0) {
            return makeIntegerValue(classic->getSeed());
        } else {
            return 0;
        }
     case ivpMagic:
        /* @q Magic:Int (Combat Property)
           VCR algorithm identification value.
           Valid only for classic combat, EMPTY for others. */
        if (classic != 0) {
            return makeIntegerValue(classic->getSignature());
        } else {
            return 0;
        }
     case ivpType:
        /* @q Type$:Int (Combat Property)
           Unit type identification value.
           - 0: this is a ship/ship fight.
           - 1: this is a ship/planet fight, {Right (Combat Property)|Right} resp.
                {Unit (Combat Property)|Unit(1)} is a planet.
           Valid only for classic combat, EMPTY for others. */
        if (classic != 0) {
            return makeIntegerValue(classic->right().isPlanet());
        } else {
            return 0;
        }
     case ivpAlgorithm:
        /* @q Algorithm:Str (Combat Property)
           Name of VCR algorithm. */
        if (battle != 0) {
            afl::string::NullTranslator tx;
            return makeStringValue(battle->getAlgorithmName(tx));
        } else {
            return 0;
        }
     case ivpFlags:
        /* @q Capabilities:Int (Combat Property)
           VCR feature identification value.
           Valid only for classic combat, EMPTY for others. */
        if (classic != 0) {
            return makeIntegerValue(classic->getCapabilities());
        } else {
            return 0;
        }
     case ivpNumUnits:
        /* @q NumUnits:Int (Combat Property)
           Number of units participating in this fight.
           This is the number of elements in the {Unit (Combat Property)|Unit} array.
           @since PCC2 1.99.19 */
        if (battle != 0) {
            return makeIntegerValue(battle->getNumObjects());
        } else {
            return 0;
        }
     case ivpUnits:
        /* @q Unit:Obj() (Combat Property)
           Information about all participating units.
           Each object in this array has {int:index:group:combatparticipantproperty|Combat Participant Properties}.
           Indexes are 1 to {NumUnits (Combat Property)|NumUnits}.

           The properties of <tt>Units(1)</tt> and <tt>Units(2)</tt> are also available as Combat Properties
           %Left.XXX and %Right.XXX, mainly for classic 1:1 combat.

           @since PCC2 1.99.19 */
        if (battle != 0) {
            return new VcrSideFunction(battleNumber, session, root, turn, shipList);
        } else {
            return 0;
        }
    }
    return 0;
}
