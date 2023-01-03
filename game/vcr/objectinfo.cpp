/**
  *  \file game/vcr/objectinfo.cpp
  *  \brief VCR Object Information
  */

#include <cmath>
#include "game/vcr/objectinfo.hpp"
#include "afl/string/format.hpp"
#include "game/vcr/object.hpp"
#include "util/math.hpp"

using afl::string::Format;
using util::roundToInt;

void
game::vcr::describePlanet(PlanetInfo& result, const Object& in, const game::config::HostConfiguration& config)
{
    // ex PlanetInformation::init
    // Clear output
    result = PlanetInfo();

    // Is it a planet?
    if (!in.isPlanet()) {
        return;
    }

    // Unarmed?
    int mass = in.getMass() - 100;
    if (mass <= 0 && in.getNumBeams() > 0) {
        // Mass indicates unarmed but unit has beams -> return, keeping isValid=false
        return;
    }

    // Derived information
    /* Fighters  = Round(Sqrt(PD)) + BaseFighters
       BeamCount = Round(Sqrt((PD+BD)/3))
       BeamType  = Round(Sqrt(PD/2)), at least BeamTech */
    // Validate beam count, should always follow mass
    int numBeams = roundToInt(std::sqrt(mass / 3.0));
    if (numBeams <= 10 && in.getNumBeams() != numBeams) {
        return;
    }

    // Assume no starbase, and compute effective beam type and fighter count.
    // If they do not match, there is a base.
    int beamType = roundToInt(std::sqrt(mass / 2.0));
    if (beamType > 10) {
        beamType = 10;
    }
    int numFighters = roundToInt(std::sqrt(double(mass)));
    int actualFighterCount = in.getNumFighters();

    /* FIXME: Nu uses different formulas.
         numFighters = roundToInt(std::sqrt(mass - 0.75))    // or ERND?
         numBays     = trunc(std::sqrt(mass)) */

    result.mass = in.getMass();
    if (in.getBeamType() != beamType || actualFighterCount != in.getNumBays() || in.getNumBays() != numFighters) {
        // It has a base
        result.hasBase = true;
        result.baseBeamTech = Range_t();

        /* We're pretty certain that there is a base. Try to figure out
           the PD/SBD split. PCC 1.x tried to compute the values directly,
           but didn't catch all cases. So we just use brute force now.
           It takes about 40k cycles maximum. */
        int maxSBD = config[game::config::HostConfiguration::MaximumDefenseOnBase](in.getOwner());
        int maxSBFighters = config[game::config::HostConfiguration::MaximumFightersOnBase](in.getOwner());
        for (int sbd = 0; sbd <= mass && sbd <= maxSBD; ++sbd) {
            // Compute derived information
            int pd = mass - sbd;
            int bt = roundToInt(std::sqrt(pd / 2.0));
            if (bt > 10) {
                bt = 10;
            }
            int ftr = roundToInt(std::sqrt(double(pd)));

            // Is this possible?
            // - Must not exceed reported type/count
            // - Consider that bases sometimes add 5 bays, sometimes not
            if (bt <= in.getBeamType()
                && ftr <= actualFighterCount
                && (ftr == in.getNumBays() || ftr + 5 == in.getNumBays())
                && actualFighterCount - ftr <= maxSBFighters)
            {
                result.baseBeamTech.include(in.getBeamType());
                if (bt == in.getBeamType()) {
                    // We got the same type from the formula as is used in the VCR.
                    // This means the VCR may be a result of the formula, and tech may be anything lower.
                    result.baseBeamTech.include(1);
                }
                result.defense.include(pd);
                result.baseDefense.include(sbd);
                result.numBaseFighters.include(actualFighterCount - ftr);
            }
        }
        result.maxBaseFighters = maxSBFighters;
        result.maxBaseDefense = maxSBD;
        result.isValid = true;
    } else {
        // No hint that this might be a starbase
        result.hasBase = false;
        result.defense = Range_t::fromValue(mass);
        result.baseDefense = Range_t::fromValue(0);
        result.numBaseFighters = Range_t::fromValue(0);
        result.baseBeamTech = Range_t();
        result.maxBaseFighters = 0;
        result.maxBaseDefense = 0;
        result.isValid = true;
    }
}

void
game::vcr::describeShip(ShipInfo& result, const Object& in, const game::spec::ShipList& shipList,
                        const game::spec::Hull* pAssumedHull,
                        bool withESB,
                        const game::config::HostConfiguration& config,
                        afl::string::Translator& tx,
                        util::NumberFormatter fmt)
{
    // ex showShipInfo (part)

    // Primary weapon
    const game::spec::Beam* b = shipList.beams().get(in.getBeamType());
    result.primary.first = (b != 0 && in.getNumBeams() != 0
                            ? String_t(Format("%d " UTF_TIMES " %s", in.getNumBeams(), b->getName(shipList.componentNamer())))
                            : tx("none"));
    result.primary.second = (pAssumedHull == 0
                             ? String_t()
                             : pAssumedHull->getMaxBeams() == 0
                             ? tx("none")
                             : String_t(Format(tx("%d beam%!1{s%}"), pAssumedHull->getMaxBeams())));

    // Secondary weapon
    const game::spec::TorpedoLauncher* tl = shipList.launchers().get(in.getBeamType());
    result.secondary.first = (in.getNumBays() != 0
                              ? String_t(Format(tx("%d fighter bay%!1{s%}"), in.getNumBays()))
                              : tl != 0 && in.getNumLaunchers() != 0
                              ? String_t(Format("%d " UTF_TIMES " %s", in.getNumLaunchers(), tl->getName(shipList.componentNamer())))
                              : tx("none"));
    result.secondary.second = (pAssumedHull == 0
                               ? String_t()
                               : pAssumedHull->getNumBays() != 0
                               ? String_t(Format(tx("%d fighter bay%!1{s%}"), pAssumedHull->getNumBays()))
                               : pAssumedHull->getMaxLaunchers() != 0
                               ? String_t(Format(tx("%d launcher%!1{s%}"), pAssumedHull->getMaxLaunchers()))
                               : tx("none"));

    // Ammo + Cargo
    result.ammo.first = (in.getNumBays() != 0
                         ? String_t(Format(tx("%d fighter%!1{s%}"), fmt.formatNumber(in.getNumFighters())))
                         : in.getNumLaunchers() != 0
                         ? String_t(Format(tx("%d torpedo%!1{es%}"), fmt.formatNumber(in.getNumTorpedoes())))
                         : String_t());
    result.ammo.second = (pAssumedHull != 0
                          ? String_t(Format(tx("%d kt cargo"), fmt.formatNumber(pAssumedHull->getMaxCargo())))
                          : String_t());

    // Crew
    result.crew.first = fmt.formatNumber(in.getCrew());
    result.crew.second = (pAssumedHull != 0
                          ? fmt.formatNumber(pAssumedHull->getMaxCrew())
                          : String_t());

    // Experience
    result.experienceLevel.first = (config[game::config::HostConfiguration::NumExperienceLevels]() != 0
                                    || in.getExperienceLevel() != 0)
        ? config.getExperienceLevelName(in.getExperienceLevel(), tx)
        : String_t();
    result.experienceLevel.second = String_t();

    // Tech level
    result.techLevel.first = String_t();
    result.techLevel.second = (pAssumedHull != 0
                               ? fmt.formatNumber(pAssumedHull->getTechLevel())
                               : String_t());

    // Mass
    result.mass.first = String_t(Format(tx("%d kt"), fmt.formatNumber(in.getMass())));
    result.mass.second = (pAssumedHull != 0
                          ? String_t(Format(tx("%d kt"), fmt.formatNumber(pAssumedHull->getMass())))
                          : String_t());

    // Shield
    result.shield.first = String_t(Format("%d%%", in.getShield()));
    result.shield.second = String_t();

    // Damage
    result.damage.first = String_t(Format("%d%%", in.getDamage()));
    result.damage.second = (config.getPlayerRaceNumber(in.getOwner()) == 2 ? "150%" : "99%");

    // Fuel
    result.fuel.first = String_t();
    result.fuel.second = (pAssumedHull != 0
                          ? String_t(Format(tx("%d kt"), fmt.formatNumber(pAssumedHull->getMaxFuel())))
                          : String_t());

    // Engines
    const game::spec::Engine* e = shipList.engines().get(in.getGuessedEngine(shipList.engines(), pAssumedHull, withESB, config));
    result.engine.first = (e != 0
                           ? e->getName(shipList.componentNamer())
                           : tx("unknown"));
    result.engine.second = (pAssumedHull != 0
                            ? String_t(Format(tx("%d engine%!1{s%}"), pAssumedHull->getNumEngines()))
                            : String_t());
}
