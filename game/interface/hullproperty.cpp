/**
  *  \file game/interface/hullproperty.cpp
  */

#include "game/interface/hullproperty.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "game/experiencelevelset.hpp"
#include "game/limits.hpp"

using interpreter::makeIntegerValue;
using interpreter::makeStringValue;

namespace {
    struct FunctionMap {
        char ch;
        int basicFunction : 8;
    };
    const FunctionMap functions[] = {
        {'C', game::spec::HullFunction::Cloak},
        {'C', game::spec::HullFunction::AdvancedCloak},
        {'C', game::spec::HullFunction::HardenedCloak},
        {'H', game::spec::HullFunction::Hyperdrive},
        {'G', game::spec::HullFunction::Gravitonic},
        {'B', game::spec::HullFunction::Bioscan},
        {'B', game::spec::HullFunction::FullBioscan},
        {'A', game::spec::HullFunction::MerlinAlchemy},
        {'A', game::spec::HullFunction::AriesRefinery},
        {'A', game::spec::HullFunction::NeutronicRefinery},
        {'\0', 0} // dummy element to simplify loop
    };

    String_t getSpecialFunctionsString(const game::spec::Hull& hull,
                                       const game::spec::ShipList& list,
                                       const game::config::HostConfiguration& config)
    {
        String_t result;
        const game::ExperienceLevelSet_t levels = game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS);
        const game::PlayerSet_t players = game::PlayerSet_t::allUpTo(game::MAX_PLAYERS) - 0;
        const game::spec::HullFunctionAssignmentList& hfs = hull.getHullFunctions(true);
        const game::spec::HullFunctionAssignmentList& ras = list.racialAbilities();

        afl::base::Memory<const FunctionMap> fs(functions);
        char last = '\0';
        game::PlayerSet_t playerSum;
        while (const FunctionMap* f = fs.eat()) {
            if (f->ch != last) {
                if (playerSum.contains(players)) {
                    result += last;
                }
                playerSum.clear();
            }
            last = f->ch;
            playerSum += hfs.getPlayersThatCan(f->basicFunction, list.modifiedHullFunctions(), list.basicHullFunctions(), config, hull, levels, true);
            playerSum += ras.getPlayersThatCan(f->basicFunction, list.modifiedHullFunctions(), list.basicHullFunctions(), config, hull, levels, false);
        }
        return result;
     }
}

afl::data::Value*
game::interface::getHullProperty(const game::spec::Hull& h, HullProperty isp, const game::spec::ShipList& list, const game::config::HostConfiguration& config)
{
    // ex int/if/hullif.h:getHullProperty
    switch (isp) {
     case ihpMaxBeams:
        /* @q Beam.Max:Int (Ship Property, Hull Property)
           Maximum number of beams on this ship. */
        return makeIntegerValue(h.getMaxBeams());
     case ihpMaxCargo:
        /* @q Cargo.Max:Int (Ship Property, Hull Property)
           Maximum cargo on this ship. */
        return makeIntegerValue(h.getMaxCargo());
     case ihpMaxFuel:
        /* @q Cargo.MaxFuel:Int (Ship Property, Hull Property)
           Maximum fuel on this ship. */
        return makeIntegerValue(h.getMaxFuel());
     case ihpMaxCrew:
        /* @q Crew.Normal:Int (Ship Property, Hull Property)
           Maximum crew on this ship. */
        return makeIntegerValue(h.getMaxCrew());
     case ihpNumEngines:
        /* @q Engine.Count:Int (Ship Property, Hull Property)
           Number of engines. */
        return makeIntegerValue(h.getNumEngines());
     case ihpSpecial:
        /* @q Special:Str (Hull Property)
           Special function summary.
           This is a string identifying the major special functions of this hull.
           The string will contain each letter if and only if the hull
           has the respective ability assigned for all players.
           - "C" (Cloak, including Advanced and Hardened Cloak)
           - "H" (Hyperdrive)
           - "G" (Gravitonic accelerator)
           - "B" (Bioscan, including Full Bioscan)
           - "A" (Alchemy, including Neutronic/Aries Refinery) */
        return makeStringValue(getSpecialFunctionsString(h, list, config));
     case ihpMaxTorpLaunchers:
        /* @q Torp.LMax:Int (Ship Property, Hull Property)
           Maximum number of torpedo launchers on this ship. */
        return makeIntegerValue(h.getMaxLaunchers());
     case ihpNumFighterBays:
        /* @q Fighter.Bays:Int (Hull Property)
           Number of fighter bays on this ship. */
        return makeIntegerValue(h.getNumBays());
     case ihpImage:
        /* @q Image:Int (Hull Property)
           Picture number used to display this ship in PCC.
           @assignable */
        return makeIntegerValue(h.getInternalPictureNumber());
     case ihpImage2:
        /* @q Image$:Int (Hull Property)
           Picture number used to display this ship in planets.exe. */
        return makeIntegerValue(h.getExternalPictureNumber());
    }
    return 0;
}

void
game::interface::setHullProperty(game::spec::Hull& h, HullProperty isp, const afl::data::Value* value, game::spec::ShipList& list)
{
    // ex int/if/hullif.h:setHullProperty
    int32_t i;
    switch (isp) {
     case ihpImage:
        if (interpreter::checkIntegerArg(i, value)) {
            h.setInternalPictureNumber(i);
            list.sig_change.raise();
        }
        break;
     default:
        throw interpreter::Error::notAssignable();
    }
}
