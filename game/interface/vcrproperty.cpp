/**
  *  \file game/interface/vcrproperty.cpp
  *  \brief Enum game::interface::VcrProperty
  */

#include "game/interface/vcrproperty.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/interface/vcrsidefunction.hpp"
#include "game/turn.hpp"
#include "game/vcr/battle.hpp"
#include "interpreter/values.hpp"

using interpreter::makeIntegerValue;
using interpreter::makeOptionalIntegerValue;
using interpreter::makeStringValue;
using interpreter::makeSizeValue;
using game::vcr::Battle;

afl::data::Value*
game::interface::getVcrProperty(size_t battleNumber,
                                VcrProperty ivp,
                                afl::string::Translator& tx,
                                const afl::base::Ref<const Root>& root,
                                const afl::base::Ptr<game::vcr::Database>& battles,
                                const afl::base::Ref<const game::spec::ShipList>& shipList)
{
    // ex int/if/vcrif.h:getVcrProperty
    Battle*const battle = (battles.get() != 0 ? battles->getBattle(battleNumber) : 0);
    if (battle == 0) {
        return 0;
    }

    switch (ivp) {
     case ivpSeed:
        /* @q Seed:Int (Combat Property)
           Random number seed.
           Valid only for classic combat, EMPTY for others.
           Since PCC2 2.40.11, also valid for FLAK. */
        return makeOptionalIntegerValue(battle->getAuxiliaryInformation(Battle::aiSeed));

     case ivpMagic:
        /* @q Magic:Int (Combat Property)
           VCR algorithm identification value.
           Valid only for classic combat, EMPTY for others. */
        return makeOptionalIntegerValue(battle->getAuxiliaryInformation(Battle::aiMagic));

     case ivpType:
        /* @q Type$:Int (Combat Property)
           Unit type identification value.
           - 0: this is a ship/ship fight.
           - 1: this is a ship/planet fight, {Right (Combat Property)|Right} resp.
             {Unit (Combat Property)|Unit(1)} is a planet.
           Valid only for classic combat, EMPTY for others. */
        return makeOptionalIntegerValue(battle->getAuxiliaryInformation(Battle::aiType));

     case ivpAlgorithm: {
        /* @q Algorithm:Str (Combat Property)
           Name of VCR algorithm. */
        afl::string::NullTranslator tx;
        return makeStringValue(battle->getAlgorithmName(tx));
     }
     case ivpFlags:
        /* @q Capabilities:Int (Combat Property)
           VCR feature identification value.
           Valid only for classic combat, EMPTY for others. */
        return makeOptionalIntegerValue(battle->getAuxiliaryInformation(Battle::aiFlags));

     case ivpNumUnits:
        /* @q NumUnits:Int (Combat Property)
           Number of units participating in this fight.
           This is the number of elements in the {Unit (Combat Property)|Unit} array.
           @since PCC2 1.99.19 */
        return makeSizeValue(battle->getNumObjects());

     case ivpUnits:
        /* @q Unit:Obj() (Combat Property)
           Information about all participating units.
           Each object in this array has {int:index:group:combatparticipantproperty|Combat Participant Properties}.
           Indexes are 1 to {NumUnits (Combat Property)|NumUnits}.

           The properties of <tt>Units(1)</tt> and <tt>Units(2)</tt> are also available as Combat Properties
           %Left.XXX and %Right.XXX, mainly for classic 1:1 combat.

           @since PCC2 1.99.19 */
        return new VcrSideFunction(battleNumber, tx, root, battles, shipList);

     case ivpLocX:
     case ivpLocY: {
        /* @q Loc.X:Int (Combat Property), Loc.Y:Int (Combat Property)
           Location of the battle in the universe, if known.
           @since PCC2 2.40.11 */
        game::map::Point pt;
        if (battle->getPosition().get(pt)) {
            return makeIntegerValue(ivp == ivpLocX ? pt.getX() : pt.getY());
        }
        return 0;
     }

     case ivpAmbient:
        /* @q Ambient:Int (Combat Property)
           Ambient flags for combat.
           Valid for FLAK combat, although as of PCC2 2.40.10, not in use by the FLAK server.
           @since PCC2 2.40.11 */
        return makeOptionalIntegerValue(battle->getAuxiliaryInformation(Battle::aiAmbient));
    }
    return 0;
}
