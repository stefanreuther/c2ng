/**
  *  \file game/map/shippredictor.cpp
  *
  *  FIXME: this is the PCC 2.0 version which has been improved until 2.0.3!
  */

#include <cmath>
#include "game/map/shippredictor.hpp"
#include "game/map/universe.hpp"
#include "game/map/ship.hpp"
#include "game/spec/hullfunction.hpp"
#include "game/map/planet.hpp"
#include "game/spec/hull.hpp"
#include "game/registrationkey.hpp"
#include "util/math.hpp"
#include "afl/bits/bits.hpp"
#include "game/map/minefieldmission.hpp"

using game::spec::HullFunction;
using game::spec::Cost;
using game::config::HostConfiguration;

namespace {
    int sgn(double d)
    {
        return d < 0 ? -1 : d > 0 ? +1 : 0;
    }

    const int AlchemyTri = 1;
    const int AlchemyDur = 2;
    const int AlchemyMol = 4;

    int getAlchemyFCodeValue(const String_t fc, const game::HostVersion& host, const game::RegistrationKey& key, bool enable)
    {
        // ex shipacc.pas:AlchemyFCValue
        int result = AlchemyTri + AlchemyDur + AlchemyMol;
        if (enable && key.getStatus() == game::RegistrationKey::Registered) {
            if (fc == "alt") {
                result = AlchemyTri;
            } else if (fc == "ald") {
                result = AlchemyDur;
            } else if (fc == "alm") {
                result = AlchemyMol;
            } else if (host.hasAlchemyExclusionFCodes()) {
                if (fc == "nat") {
                    result -= AlchemyTri;
                } else if (fc == "nad") {
                    result -= AlchemyDur;
                } else if (fc == "nam") {
                    result -= AlchemyMol;
                }
            }
        }
        return result;
    }


    // /** Perform refinery reaction.
    //     \param s   [in/out] Ship
    //     \param ore [in/out] Ore to consume
    //     \param sup [in/out] Supplies to consume */
    void doRefinery(game::map::ShipData& ship, game::IntegerProperty_t& ore, int& sup, const game::spec::Hull& hull)
    {
        // ex game/shippredict.cc:doRefinery
        // ex shipacc.pas:DoRefinery
        const int haveFuel = ship.neutronium.orElse(0);
        const int haveOre  = ore.orElse(0);
        int n = hull.getMaxFuel() - haveOre;
        if (n > haveOre) {
            n = haveOre;
        }
        if (n > sup) {
            n = sup;
        }
        ship.neutronium = haveFuel + n;
        sup -= n;
        ore = haveOre - n;
    }

    // /** Perform direct alchemy reaction (Alchemy + Refinery function on one ship).
    //     \param ship [in/out] Ship
    //     \param ratio [in] Supply conversion ratio */
    void doDirectRefinery(game::map::ShipData& ship, const int ratio, const game::spec::Hull& hull)
    {
        // ex game/shippredict.cc:doDirectRefinery
        // ex shipacc.pas:DoDirectRefinery
        const int haveFuel = ship.neutronium.orElse(0);
        const int haveSupplies = ship.supplies.orElse(0);
        const int free = hull.getMaxFuel() - haveFuel;
        int prod = std::min(haveSupplies / ratio, free);

        ship.supplies = haveSupplies - prod*ratio;
        ship.neutronium = haveFuel + prod;
    }

    // /** Perform alchemy reaction.
    //     Merlin converts 3 Supplies -> 1 Mineral.
    //     Friendly codes allow target mineral control.

    //     THost and NuHost actually convert 9 Supplies -> 3 Minerals;
    //     this is what the %rounder is used for. */
    void doAlchemy(const String_t& shipFCode, game::map::ShipData& ship, const game::HostVersion& host, const game::RegistrationKey& key)
    {
        // ex game/shippredict.cc:doAlchemy
        // Merlin converts 3 Sup -> 1 Min
        int t = 0, d = 0, m = 0;
        int haveSupplies = ship.supplies.orElse(0);
        int mins = haveSupplies / 3;
        int rounder = host.isAlchemyRounding() ? 3 : 1;
        switch (getAlchemyFCodeValue(shipFCode, host, key, true)) {
         case AlchemyTri:
            t = rounder*(mins/rounder);
            // FIXME: used_fc = true;
            break;

         case AlchemyDur:
            d = rounder*(mins/rounder);
            // FIXME: used_fc = true;
            break;

         case AlchemyMol:
            m = rounder*(mins/rounder);
            // FIXME: used_fc = true;
            break;

         case AlchemyDur + AlchemyMol:
            d = m = mins/2;
            // FIXME: used_fc = true;
            break;

         case AlchemyMol + AlchemyTri:
            t = m = mins/2;
            // FIXME: used_fc = true;
            break;

         case AlchemyTri + AlchemyDur:
            t = d = mins/2;
            // FIXME: used_fc = true;
            break;

         default:
            t = d = m = mins/3;
            // FIXME: used_fc = true;
            break;
        }

        ship.supplies   = haveSupplies - 3*(t+d+m);
        ship.tritanium  = ship.tritanium.orElse(0)  + t;
        ship.duranium   = ship.duranium.orElse(0)   + d;
        ship.molybdenum = ship.molybdenum.orElse(0) + m;
    }

    int getCloakFuel(int turns,
                     int realOwner,
                     const game::config::HostConfiguration& config,
                     const game::spec::Hull& hull)
    {
        const int cfb = config[config.CloakFuelBurn](realOwner);
        int fuel = hull.getMass() * cfb / 100;
        if (fuel < cfb) {
            fuel = cfb;
        }
        if (turns) {
            fuel *= turns;
        }
        return fuel;
    }

    void normalizePosition(game::map::ShipData& ship, const game::map::Configuration& config)
    {
        game::map::Point new_pos = config.getCanonicalLocation(game::map::Point(ship.x.orElse(0), ship.y.orElse(0)));
        ship.x = new_pos.getX();
        ship.y = new_pos.getY();
    }

    int getEngineLoad(const game::map::Universe& univ,
                      const game::map::ShipData& ship,
                      int towee_id,
                      const game::map::ShipData* towee_override,
                      bool tow_corr,
                      const game::spec::ShipList& shipList)
    {
        int mass = getShipMass(ship, shipList).orElse(0);
        if (ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
            int towee_mass = 0;
            int mission_towee_id = ship.missionTowParameter.orElse(0);
            if (towee_override != 0 && towee_id == mission_towee_id) {
                towee_mass = getShipMass(*towee_override, shipList).orElse(0);
            } else if (game::map::Ship* p = univ.ships().get(mission_towee_id)) {
                towee_mass = p->getMass(shipList).orElse(0);
            } else {
                // towee not accessible
            }
            if (tow_corr) {
                towee_mass = 10*(towee_mass/10);
            }
            mass += towee_mass;
        }
        return mass;
    }

    /** Compute distance, Tim-style.

        Tim's compiler disagrees with this one (and both differ from IEEE) when
        considering pythagoras terms. INT(SQR(3^2)) yields 2 with PDS BASIC.

        Instead of finding a formula which matches that behaviour, we just build
        a table of distances. Only distances <= 162 ly matter, for other
        distances it just matters that they're larger. Therefore we don't
        need to care about precision for large values, and our table has a
        bounded size.

        \param dx,dy Displacement
        \return Effective distance to use

        See http://phost.de/~stefan/fuelusage.html */
    int32_t timDistance(int dx, int dy)
    {
        // ex shipacc.pas:TimDist
        static const uint8_t TABLE[168][21] = {
            { 253,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            { 249,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            { 218,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            { 233,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            { 177,239,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            { 210,253,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            {  81,239,255,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            {  97,251,254,255,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            { 161,237,239,255,255,253,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            { 161,246,254,254,255,255,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            { 161,186,247,191,255,255,255,223,255,255,255,255,255,255,255,255,255,255,255,255,255 },
            {  66,237,125,255,239,255,255,255,255,254,255,255,255,255,255,255,255,255,255,255,255 },
            {  66,109,239,247,255,251,255,255,255,255,239,255,255,255,255,255,255,255,255,255,255 },
            {  65,181,123,255,254,255,254,255,255,255,255,255,251,255,255,255,255,255,255,255,255 },
            {  65,213,222,247,223,255,255,254,255,255,255,255,255,255,254,255,255,255,255,255,255 },
            {  65, 90,247,126,255,251,255,127,255,255,255,255,255,255,255,255,254,255,255,255,255 },
            {  65,106,187,247,247,127,255,255,255,254,255,255,255,255,255,255,255,255,253,255,255 },
            { 129,170,237,190,127,255,223,255,255,255,253,255,255,255,255,255,255,255,255,255,251 },
            { 129,178,118,239,251,239,255,247,255,255,255,251,255,255,255,255,255,255,255,255,255 },
            { 129,212,218,123,191,255,254,255,251,255,255,255,247,255,255,255,255,255,255,255,255 },
            { 130, 84,235,238,251,251,191,255,255,253,255,255,255,191,255,255,255,255,255,255,255 },
            { 129, 84,109,247,190,191,255,247,255,255,254,255,255,255,255,253,255,255,255,255,255 },
            { 130,164,181,189,247,251,247,255,253,255,127,255,255,255,255,255,239,255,255,255,255 },
            {   2,169,218,238,189,191,255,254,127,255,255,127,255,255,255,255,255,127,255,255,255 },
            {   2,169, 90,119,239,251,251,223,255,223,255,255,127,255,255,255,255,255,255,239,255 },
            {   2,169,106,219,123,223,191,255,251,255,247,255,255,255,254,255,255,255,255,255,255 },
            {   1, 73,173,237,222,251,253,251,127,255,255,251,255,255,255,253,255,255,255,255,255 },
            {   2, 73,213,118,247,222,223,127,255,239,255,255,254,255,255,255,251,255,255,255,255 },
            {   2, 73,213,186,189,247,253,253,239,255,251,255,255,254,255,255,255,247,255,255,255 },
            {   1, 81, 89,219,238,189,223,223,255,253,255,254,255,127,255,255,255,255,223,255,255 },
            {   2, 81,106,109,119,239,253,253,251,191,255,191,255,255,127,255,255,255,255,127,255 },
            {   1,146,170,181,221,123,223,223,191,255,247,255,239,255,255,191,255,255,255,255,255 },
            {   1,146,170,214,238,222,251,254,253,247,255,254,255,247,255,255,127,255,255,255,255 },
            {   1,146,210,218,182,247,222,239,223,255,254,191,255,255,251,255,255,127,255,255,255 },
            {   1,146, 84,107,219,189,247,253,254,253,239,255,239,255,255,253,255,255,255,254,255 },
            {   1,162, 84,173,237,238,190,239,239,191,255,251,255,251,255,255,254,255,255,255,253 },
            {   1, 34, 85,181,118,123,239,253,254,254,251,127,255,255,254,255,127,255,255,255,255 },
            {   1, 34,165,214,186,221,123,223,239,239,127,255,239,255,191,255,255,191,255,255,255 },
            {   1, 34,169, 90,219,238,222,251,254,254,253,239,255,251,255,239,255,255,191,255,255 },
            {   1, 34,169,106,109,187,247,222,239,247,223,255,254,127,255,255,247,255,255,191,255 },
            {   1, 36,169,170,181,221,189,247,125,255,254,253,191,255,223,255,255,251,255,255,191 },
            {   2, 68, 74,181,214,238,238,190,239,247,239,191,255,247,255,247,255,255,253,255,255 },
            {   1, 68, 74,213,218,182,123,239,253,126,255,254,247,255,254,255,253,255,255,254,255 },
            {   1, 68, 82, 85,107,219,221,123,239,247,247,239,255,254,191,255,127,255,255,127,255 },
            {   1, 68, 82,105,173,237,238,222,251,126,127,255,253,223,255,247,255,191,255,255,191 },
            {   2, 68,146,170,181,118,187,247,222,247,247,247,223,255,251,255,253,255,239,255,255 },
            {   1, 68,148,170,214,182,221,189,247,126,127,127,255,251,127,255,127,255,255,247,255 },
            {   2,132,164,170,106,219,110,239,189,239,247,247,239,127,255,239,255,223,255,255,251 },
            {   1,132,164, 82,107,109,183,123,239,125,191,127,255,254,239,255,253,255,247,255,255 },
            {   2,136,164, 84,173,181,219,221,123,239,247,251,247,223,255,253,127,255,255,253,255 },
            {   1,136, 36, 85,181,182,237,246,222,251,190,191,127,255,253,191,255,223,255,127,255 },
            {   2,136, 36, 85,213,218,182,187,247,222,247,251,251,247,191,255,247,255,251,255,223 },
            {   1,136, 40,165, 90,107,219,221,189,247,126,191,191,255,254,247,255,254,255,254,255 },
            {   1,136, 72,169,106,173,237,118,239,189,239,251,251,251,239,255,254,223,255,191,255 },
            {   2,136, 72,169,170,181,110,187,123,239,125,191,191,191,255,253,223,255,247,255,239 },
            {   2,  8, 73,170,172,218,182,221,221,123,239,251,251,251,247,223,255,251,255,254,255 },
            {   1,  8, 73, 74,213, 90,219,110,247,222,251,190,223,191,127,255,251,127,255,191,255 },
            {   2,  8, 73, 82, 85,107,109,183,187,247,222,247,251,253,251,247,127,255,239,255,239 },
            {   2,  8,145, 82, 85,173,181,219,221,189,247,190,223,223,191,255,254,239,255,253,255 },
            {   1,  8,145, 82,105,181,182,109,119,239,189,239,251,253,253,251,239,255,253,127,255 },
            {   2, 16,145,148,170,214,218,182,187,123,239,125,223,223,223,191,255,253,191,255,239 },
            {   2, 16,145,148,170, 90,107,219,221,221,123,239,251,253,253,253,247,191,255,247,255 },
            {   2, 16,146,164,170,106,173,237,118,247,238,123,191,223,223,223,127,255,247,255,254 },
            {   1, 16,146,164, 82,171,181,109,187,187,247,222,247,253,254,253,251,239,255,254,223 },
            {   2, 16, 18,165, 84,181,214,182,221,222,189,247,190,223,239,223,191,255,254,223,255 },
            {   1, 16, 34, 41, 85,213, 90,219,110,119,239,189,247,251,254,254,253,251,223,255,251 },
            {   2, 16, 34, 41, 85, 85,107,109,183,187,123,239,125,223,239,239,223,127,255,251,127 },
            {   1, 16, 34, 73,165,106,173,109,219,237,222,123,239,251,253,254,254,253,247,127,255 },
            {   2, 16, 34, 73,169,170,181,182,237,118,247,238,123,191,239,239,239,223,255,254,239 },
            {   1, 16, 34, 73,170,170,214,218,182,187,187,247,222,247,253,254,254,254,251,239,255 },
            {   1, 16, 34, 82,170,172, 90,107,187,221,238,189,247,190,223,239,239,239,191,255,253 },
            {   1, 32, 36, 82, 74, 85,107,109,221,118,119,239,190,247,253,126,255,254,254,247,191 },
            {   2, 32, 68,146, 82, 85,173,181,109,187,187,123,239,125,223,239,247,239,223,127,255 },
            {   1, 32, 68,146, 82, 85,181,214,182,221,238,222,123,239,251,126,127,255,254,253,239 },
            {   2, 32, 68,146, 84,170,213, 90,219,110,119,247,238,123,223,239,247,247,239,223,255 },
            {   1, 32, 68,162,148,170, 90,107,235,182,187,187,247,222,247,253,126,127,255,254,251 },
            {   1, 32, 68,164,148,170,106,173,109,219,237,238,189,247,190,239,247,247,247,239,191 },
            {   1, 32, 68, 36,165,170,170,181,182,237,118,119,239,189,247,253,126,127,127,255,253 },
            {   1, 32, 68, 36,165, 82,173,214,218,118,187,187,123,239,125,223,247,247,247,247,223 },
            {   1, 32,132, 36,169, 84,213, 90,107,187,237,238,222,123,239,251,126,191,191,127,255 },
            {   1, 32,136, 36, 41, 85, 85,107,109,219,118,119,247,222,123,223,239,247,251,247,239 },
            {   1, 32,136, 68, 73, 85, 85,173,181,109,187,219,187,247,222,251,125,191,191,127,255 },
            {   2, 32,136, 72, 74,165,106,181,214,182,221,238,238,189,247,190,239,247,251,251,247 },
            {   2, 64,136, 72, 82,169,170, 86, 91,219,110,119,119,239,190,247,253,190,191,191,127 },
            {   1, 64,136, 72, 82,170,170, 90,107,219,182,219,189,123,239,189,223,247,251,251,251 },
            {   1, 64,136, 72, 82,170,180,106,173,109,219,237,238,222,123,239,251,126,191,191,191 },
            {   2, 64,  8, 73,146, 74, 85,171,181,182,237,118,119,247,238,123,223,247,251,251,251 },
            {   1, 64,  8,137,148, 82, 85,181,214,218,118,187,221,187,247,222,251,126,191,223,223 },
            {   1, 64,  8,137,148, 84, 85,213, 90,219,182,221,238,238,189,247,190,239,251,251,253 },
            {   1, 64, 16,145,164, 84,169, 86,107,109,219,118,119,119,239,189,247,125,191,223,223 },
            {   1, 64, 16,145,164,148,170, 90,173,181,109,187,219,221,123,239,189,239,247,251,253 },
            {   2, 64, 16,145, 36,165,170,170,213,214,182,219,238,238,222,123,239,251,190,223,223 },
            {   1, 64, 16,145, 36,165,170,170,214,218,218,109,119,119,247,222,123,223,247,251,253 },
            {   1, 64, 16, 17, 41,169, 82,181, 90,107,219,182,219,221,189,247,222,251,126,223,223 },
            {   1, 64, 16, 18, 73, 41, 85, 85,107,173,109,219,237,238,238,189,247,190,239,251,253 },
            {   2, 64, 16, 34, 73, 42, 85, 85,173,182,182,237,118,183,119,239,189,247,125,191,223 },
            {   2, 64, 16, 34, 73, 74, 85, 85,181,214,214,110,187,221,221,123,239,189,239,247,253 },
            {   1,128, 16, 34, 73, 74,165,170,213, 90,219,182,221,238,238,222,123,239,251,190,223 },
            {   2,128, 32, 34, 73, 82,169,170, 90,107,109,219,110,183,123,247,222,123,223,247,251 },
            {   2,128, 32, 34,146, 82,170,170,106,181,181,109,183,219,221,189,247,222,251,190,223 },
            {   1,128, 32, 34,146,146,170,212,170,181,182,182,219,238,238,238,189,247,190,239,251 },
            {   1,128, 32, 34,146,148, 74, 85,173,214,218,182,109,119,187,119,239,189,247,125,223 },
            {   1,128, 32, 36,146,148, 82, 85,213, 90,107,219,182,219,221,221,123,239,189,239,251 },
            {   2,128, 32, 68,146,164, 84, 85, 85,107,173,109,219,237,238,238,222,123,239,123,191 },
            {   1,128, 32, 68,146,164, 84,169, 86,173,181,181,221,118,187,123,247,222,123,223,247 },
            {   1,128, 32, 68, 34, 37,149,170,106,181,214,182,109,187,221,221,189,247,222,251,190 },
            {   1,128, 32, 68, 36, 41,165,170,170,214, 90,219,182,221,238,238,238,189,247,222,247 },
            {   1,128, 32, 68, 36, 41,165,170,170, 90,107,109,219,110,183,187,119,239,189,247,125 },
            {   2,128, 32, 68, 36, 73,169, 82,181,106,173,181,109,183,219,221,221,123,239,189,239 },
            {   2,128, 64, 68, 36, 73, 41, 85, 85,171,181,182,118,219,238,238,238,222,123,239,125 },
            {   1,128, 64,132, 36, 74, 42, 85, 85,181,214,218,182,109,119,187,123,247,222,123,223 },
            {   2,  0, 65,136, 68, 82, 74, 85, 90,213,106,107,219,182,219,221,221,189,247,222,251 },
            {   2,  0, 65,136, 68, 82, 82,165,170, 86,107,173,109,187,237,110,239,238,189,247,222 },
            {   1,  0, 65,136, 72,146, 82,169,170, 90,173,181,181,221,118,187,187,123,239,189,247 },
            {   1,  0, 65,136, 72,146, 82,170,170,170,181,214,182,109,187,221,221,221,123,239,189 },
            {   2,  0, 65,136, 72,146,148,170,212,170, 86, 91,219,182,221,238,118,239,222,123,239 },
            {   1,  0, 65,136, 72,148,148, 74, 85,181, 90,107,109,219,110,183,187,187,247,222,123 },
            {   2,  0, 65,136,136,164,164, 82, 85,213,106,173,173,237,182,219,221,221,189,247,222 },
            {   1,  0, 65,  8,137, 36,165, 84, 85, 85,173,181,182,109,219,237,118,247,238,189,247 },
            {   1,  0, 65,  8,137, 36,165, 84,169, 90,181,214,218,182,237,182,187,187,123,239,189 },
            {   2,  0,129, 16,145, 36, 41,149,170,170,213, 90,107,219,118,219,221,221,221,123,239 },
            {   2,  0,129, 16,145, 36, 41,165,170,170, 90,107,109,109,187,237,110,119,239,222,123 },
            {   1,  0,129, 16,145, 72, 73,169,170,170,106,173,181,109,219,118,187,187,187,247,222 },
            {   2,  0,129, 16, 17, 73, 74,169, 82,213,170,181,214,182,109,187,221,221,221,189,247 },
            {   2,  0,129, 16, 17, 73, 74, 42, 85, 85,173,214, 90,219,182,221,110,119,247,238,189 },
            {   2,  0,129, 16, 18, 73, 82, 74, 85, 85,213, 90,107,107,219,110,183,187,187,123,239 },
            {   1,  0,129, 16, 18, 73, 82, 74, 85, 90, 85,107,173,109,221,182,219,221,221,221,123 },
            {   1,  0,130, 16, 34, 73,146, 82,165,170, 86,173,181,182,109,219,237,118,119,239,222 },
            {   1,  0,130, 16, 34,145,148, 82,169,170,106,181,214,218,182,237,118,187,187,187,247 },
            {   2,  0,130, 32, 34,145,148, 84,170,170,170,214, 90, 91,219,118,219,221,238,221,189 },
            {   1,  0,130, 32, 34,146,164,148,170, 84,171, 90,107,109,107,183,237,110,119,247,238 },
            {   1,  0,  2, 33, 34,146,164,164, 74, 85,181,106,173,181,109,219,118,187,187,187,123 },
            {   2,  0,  2, 33, 34,146, 36,165, 82, 85, 85,171,181,214,182,109,187,221,237,238,221 },
            {   2,  0,  2, 33, 34,146, 36,165, 84, 85, 85,181,214,218,218,182,219,110,119,119,239 },
            {   1,  0,  2, 33, 68, 34, 41, 41, 85,169, 90,213, 90,107,107,219,109,183,187,187,187 },
            {   1,  0,  2, 33, 68, 34, 73, 41,149,170,170, 86,107,173,109,219,182,219,237,238,222 },
            {   1,  0,  2, 33, 68, 34, 73, 73,165,170,170, 90,173,181,182,109,219,237,118,119,247 },
            {   2,  0,  2, 33, 68, 36, 73, 74,169,170,170,170,181,214,218,182,237,118,187,187,187 },
            {   1,  0,  2, 65, 68, 36, 73, 74,169, 82, 85,171,214, 90, 91,219,110,219,221,238,238 },
            {   1,  0,  2, 65, 68, 36, 73, 82, 42, 85, 85,181, 90,107,109,219,182,237,110,119,119 },
            {   1,  0,  2, 65, 68, 36,146, 82, 74, 85, 85,213,106,173,181,109,219,118,187,187,187 },
            {   1,  0,  2, 65,132, 68,146,146, 74, 85,106, 85,173,181,214,182,109,183,221,237,238 },
            {   2,  0,  2, 66,136, 68,146,148, 82,165,170, 90,181,214,218,218,182,219,110,119,119 },
            {   1,  0,  2, 66,136, 68,146,148, 84,170,170,170,213, 90,107,219,186,109,183,187,219 },
            {   2,  0,  4, 66,136, 72,146,164, 84,170,170,170, 86,107,173,109,219,182,219,237,238 },
            {   2,  0,  4, 66,136, 72,162,164,148,170, 84,173,106,173,181,182,109,219,237,118,119 },
            {   1,  0,  4, 66,136, 72, 36, 37,165, 74, 85,213,170,181,214,214,182,221,118,187,219 },
            {   1,  0,  4, 66,136,136, 36, 41,165, 82, 85, 85,173,214, 90,219,218,110,187,221,238 },
            {   2,  0,  4,130,136,136, 36, 41,169, 84, 85, 85,181, 90,107,109,219,182,221,110,119 },
            {   2,  0,  4,130,  8,137, 36, 73, 41, 85,170,106, 85,107,173,181,109,219,110,187,219 },
            {   1,  0,  4,130,  8,137, 36, 73, 41,149,170,170, 86,173,181,182,182,109,183,221,237 },
            {   1,  0,  4,130, 16,145, 68, 74, 74,165,170,170,106,181,214,218,214,118,219,110,119 },
            {   1,  0,  4,130, 16,145, 72, 82, 74,169,170,170,170,213, 90,107,219,182,109,183,187 },
            {   1,  0,  4,130, 16,145, 72, 82, 82,169, 82, 85,171, 90,107,173,109,219,182,219,237 },
            {   2,  0,  4,130, 16, 17, 73,146, 82, 42, 85, 85,181,106,173,181,181,109,187,237,118 },
            {   1,  0,  4,132, 16, 17, 73,146,146, 74, 85, 85, 85,171,181,214,214,182,221,118,187 },
            {   1,  0,  4,132, 16, 17, 73,146,148, 82, 85,170, 85,181,214, 90,219,182,109,187,221 },
            {   1,  0,  4,132, 16, 18,137,148,148, 82,165,170,106,213, 90,107,109,219,182,221,110 },
            {   1,  0,  4,  4, 17, 34,145,164,164, 84,170,170,170, 86,107,173,173,109,219,110,187 },
            {   1,  0,  4,  4, 17, 34,145, 36,165, 84,170,170,170, 90,173,181,182,182,237,182,221 },
            {   1,  0,  4,  4, 33, 34,145, 36, 37,149,170, 84,173,170,181,214,218,182,109,219,110 },
            {   1,  0,  8,  4, 33, 34,146, 36, 41,165, 74, 85, 85,171,214, 90,107,219,182,109,183 },
            {   1,  0,  8,  4, 33, 34,146, 36, 41,165, 82, 85, 85,181, 90,107,109,109,219,118,219 },
            {   1,  0,  8,  4, 33, 34, 18, 73, 73,169, 84, 85, 85,213,106,173,181,181,109,187,237 },
            {   2,  0,  8,  4, 33, 34, 34, 73, 74, 41, 85,170,170, 85,173,181,214,182,182,219,118 },
            {   2,  0,  8,  4, 33, 68, 34, 73, 74, 74,149,170,170, 90,181,214, 90,219,182,109,187 },
            {   2,  0,  8,  4, 33, 68, 34, 73, 82, 74,165,170,170,170,213, 90,107,107,219,182,221 },
            {   2,  0,  8,  8, 33, 68, 36, 73,146, 82,169, 42,181,170, 86,171,173,173,109,219,110 },
        };

        dx = std::abs(dx);
        dy = std::abs(dy);
        if (dy == 0) {
            if (dx == 0) {
                return 0;
            }
            dy = dx;
            dx = 0;
        }
        if (dx >= 168 || dy >= 168) {
            // lo-fi version
            return int(util::getDistanceFromDX(dx, dy));
        }

        int value = dy-1;
        const uint8_t* table_ptr = &TABLE[dy-1][0];
        while (dx >= 8) {
            value += int(afl::bits::bitPop8(*table_ptr));
            ++table_ptr;
            dx -= 8;
        }

        for (int bit = 0; bit <= dx; ++bit) {
            if ((*table_ptr & (1 << bit)) != 0) {
                ++value;
            }
        }
        return value;
    }

    int computeFuelUsage(const game::map::Universe& univ,
                         const game::map::ShipData& ship,
                         int towee_id,
                         const game::map::ShipData* towee_override,
                         bool grav_acc, double dist,
                         const game::spec::ShipList& shipList,
                         const game::config::HostConfiguration& config,
                         const game::HostVersion& host)
    {
        // ex shipacc.pas:ComputeFuelUsage
        int warp = ship.warpFactor.orElse(0);
        if (warp <= 0) {
            return 0;
        }

        int way = warp*warp;
        if (grav_acc) {
            way *= 2;
        }

        const game::spec::Engine* pEngine = shipList.engines().get(ship.engineType.orElse(0));
        if (!pEngine) {
            return 0;
        }
        int32_t ff;
        if (!pEngine->getFuelFactor(warp, ff)) {
            return 0;
        }

        bool isTHost = host.getKind() != game::HostVersion::PHost;
        int load = getEngineLoad(univ, ship, towee_id, towee_override, isTHost, shipList);

        if (isTHost) {
            // THost formula
            int32_t move = std::min(timDistance(ship.waypointDX.orElse(0), ship.waypointDY.orElse(0)), way);
            return int((ff * (load / 10) * move) / (10000L * way));
        } else if (!config[config.UseAccurateFuelModel]()) {
            // PHost, standard formula
            return int(util::divideAndRoundToEven(load, 10, 0) * ff * long(dist) / (10000L * way));
        } else {
            // PHost, "accurate" formula
            // This is a cheat. Let's say the formula tells us we need 1.4 kt
            // fuel. If the ship has 5 kt, that'll yield a net usage of 1 kt
            // (5 - 1.4 = 3.6; rounded = 4 kt remaining). If the ship has just
            // one kt, PHost will see that the ship needs more than that, and go
            // into failure mode.
            double res = load * (1 - std::exp(-(ff * dist) / (way*100000.0)));
            int res_int = util::roundToInt(res);
            int ship_fuel = ship.neutronium.orElse(0);
            if (host.hasAccurateFuelModelBug()) {
                if (res_int == ship_fuel && res > ship_fuel) {
                    ++res_int;
                }
            }
            return res_int;
        }
    }

    int computeTurnFuel(const game::map::Ship& ship,
                        const game::config::HostConfiguration& config,
                        const game::spec::ShipList& shipList,
                        int eta)
    {
        // FIXME: do we have this elsewhere?
        // ex shipacc.pas:TurnFuelUsage (sort-of)
        const game::spec::Hull* hull = shipList.hulls().get(ship.getHull().orElse(0));
        int fuel = (hull != 0
                    ? (int32_t(config[config.FuelUsagePerTurnFor100KT](ship.getRealOwner().orElse(0))) * hull->getMass() + 99) / 100
                    : 0);
        if (eta != 0) {
            fuel *= eta;
        }
        return fuel;
    }
}

const int game::map::ShipPredictor::MOVEMENT_TIME_LIMIT;

// /** Constructor. Create a predictor for a single ship.
//     \param univ universe containing the ship
//     \param id   ship Id */
game::map::ShipPredictor::ShipPredictor(const Universe& univ, Id_t id,
                                        const UnitScoreDefinitionList& scoreDefinitions,
                                        const game::spec::ShipList& shipList,
                                        const game::config::HostConfiguration& config,
                                        const HostVersion& hostVersion,
                                        const RegistrationKey& key)
    : m_scoreDefinitions(scoreDefinitions),
      m_shipList(shipList),
      m_hostConfiguration(config),
      m_hostVersion(hostVersion),
      m_key(key),
      id(id), ship(), valid(false), towee_override(0),
      univ(univ), movement_fuel_used(0), cloak_fuel_used(0), turns_computed(0)
{
    // ex GShipTurnPredictor::GShipTurnPredictor
    init();
}

// /** Constructor. Create a predictor for a ship towing another ship.
//     \param univ   universe containing the ship
//     \param id     ship Id
//     \param towee_override predictor for towed ship */
game::map::ShipPredictor::ShipPredictor(const Universe& univ, Id_t id, ShipPredictor& towee,
                                        const UnitScoreDefinitionList& scoreDefinitions,
                                        const game::spec::ShipList& shipList,
                                        const game::config::HostConfiguration& config,
                                        const HostVersion& hostVersion,
                                        const RegistrationKey& key)
    : m_scoreDefinitions(scoreDefinitions),
      m_shipList(shipList),
      m_hostConfiguration(config),
      m_hostVersion(hostVersion),
      m_key(key),
      id(id), ship(), valid(false), towee_override(&towee),
      univ(univ), movement_fuel_used(0), cloak_fuel_used(0), turns_computed(0)
{
    // ex GShipTurnPredictor::GShipTurnPredictor
    init();
}

// /** Get total fuel used for movement. */
int32_t
game::map::ShipPredictor::getMovementFuelUsed() const
{
    // ex GShipTurnPredictor::getMovementFuelUsed
    return movement_fuel_used;
}

// /** Get total amount of fuel used for cloaking. */
int32_t
game::map::ShipPredictor::getCloakFuelUsed() const
{
    // ex GShipTurnPredictor::getCloakFuelUsed
    return cloak_fuel_used;
}

// /** Get number of turns computed. */
int
game::map::ShipPredictor::getNumTurns() const
{
    // ex GShipTurnPredictor::getNumTurns
    return turns_computed;
}

// /** Check whether computation was stopped because the turn limit was exceeded. */
bool
game::map::ShipPredictor::isAtTurnLimit() const
{
    // ex GShipTurnPredictor::isAtTurnLimit
    return turns_computed >= MOVEMENT_TIME_LIMIT;
}

// /** Compute one turn. This will compute all changes for the ship. It
//     will also update the towed ship predictor, if any. */
void
game::map::ShipPredictor::computeTurn()
{
    // ex GShipTurnPredictor::computeTurn
    // ex shipacc.pas:ComputeTurn

    // is this actually a predictable ship?
    if (!valid) {
        return;
    }

    // where are we?
    int pid = univ.getPlanetAt(Point(ship.x.orElse(0), ship.y.orElse(0)));
    const Ship* real_ship = univ.ships().get(id);
    const game::spec::Hull* pHull = m_shipList.hulls().get(ship.hullType.orElse(0));
    if (real_ship == 0 || pHull == 0) {
        ++turns_computed; // FIXME: needed to make the computeMovement() loop exit eventually. Give this function a success return instead?
        return;
    }

    // Mine laying
    // This is a hack: checkLayMission() takes a Ship, but we only have a ShipData.
    // However, with turns_computed=0, both still have the same content.
    MinefieldMission md;
    if (turns_computed == 0 && md.checkLayMission(*real_ship, univ, m_hostVersion, m_key, m_hostConfiguration, m_scoreDefinitions, m_shipList)) {
        int torps = ship.ammo.orElse(0);
        ship.ammo = torps - std::min(torps, md.getNumTorpedoes());
   //      ship.ammo -= std::min(int(ship.ammo), md.num_torps);
   // FIXME:     if (md.used_fcode) {
   //          used_properties |= UsedFCode;
   //      }
   // FIXME:     if (md.used_mission) {
   //          used_properties |= UsedMission;
   //      }
   //  }
    }

    // Special Missions I (Super Refit, Self Repair, Hiss, Rob) would go here
    const String_t shipFCode = ship.friendlyCode.orElse("");
    bool is_mkt_fc = (shipFCode == "mkt");
    if ((is_mkt_fc
         || (m_hostVersion.isPHost()
             && ship.mission.orElse(0) == m_hostConfiguration[m_hostConfiguration.ExtMissionsStartAt]() + game::spec::Mission::pmsn_BuildTorpsFromCargo))
        && ship.numLaunchers.orElse(0) > 0
        && ship.neutronium.orElse(0) > 0)
    {
        if (const game::spec::TorpedoLauncher* tl = m_shipList.launchers().get(ship.launcherType.orElse(0))) {
            // Determine available resources
            Cost cargo;
            cargo.set(Cost::Tritanium,  ship.tritanium.orElse(0));
            cargo.set(Cost::Duranium,   ship.duranium.orElse(0));
            cargo.set(Cost::Molybdenum, ship.molybdenum.orElse(0));
            cargo.set(Cost::Money,      ship.money.orElse(0));
            cargo.set(Cost::Supplies,   ship.supplies.orElse(0));

            // Determine number of torpedoes to build by dividing that by the cost
            int n = cargo.getMaxAmount(pHull->getMaxCargo(), tl->torpedoCost());

            // Determine new cargo
            cargo -= tl->torpedoCost() * n;

            // Add
            ship.ammo       = ship.ammo.orElse(0) + n;
            ship.tritanium  = cargo.get(Cost::Tritanium);
            ship.duranium   = cargo.get(Cost::Duranium);
            ship.molybdenum = cargo.get(Cost::Molybdenum);
            ship.money      = cargo.get(Cost::Money);
            ship.supplies   = cargo.get(Cost::Supplies);

            // FIXME: if (n > 0) {
            //          if (is_mkt_fc) {
            //              used_properties |= UsedFCode;
            //          } else {
            //              used_properties |= UsedMission;
            //          }
            //      }
        }
    }

    // Alchemy
    if (shipFCode != "NAL") {
        if (real_ship->hasSpecialFunction(HullFunction::MerlinAlchemy, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
            if (m_hostVersion.hasAlchemyCombinations()
                && m_hostConfiguration[HostConfiguration::AllowAdvancedRefinery]() != 0
                && real_ship->hasSpecialFunction(HullFunction::AriesRefinery, m_scoreDefinitions, m_shipList, m_hostConfiguration))
            {
                // Alchemy + AdvancedRefinery -> 3:1 direct refinery
                doDirectRefinery(ship, 3, *pHull);
            } else if (m_hostVersion.hasAlchemyCombinations()
                       && real_ship->hasSpecialFunction(HullFunction::NeutronicRefinery, m_scoreDefinitions, m_shipList, m_hostConfiguration))
            {
                // Alchemy + Refinery -> 4:1 direct refinery
                doDirectRefinery(ship, 4, *pHull);
            } else {
                // Regular Alchemy
                doAlchemy(shipFCode, ship, m_hostVersion, m_key);
            }
        } else if (real_ship->hasSpecialFunction(HullFunction::AriesRefinery, m_scoreDefinitions, m_shipList, m_hostConfiguration)
                   && m_hostConfiguration[HostConfiguration::AllowAdvancedRefinery]() != 0
                   && (m_hostVersion.hasAlchemyCombinations()
                       || !real_ship->hasSpecialFunction(HullFunction::NeutronicRefinery, m_scoreDefinitions, m_shipList, m_hostConfiguration)))
        {
            // Aries converts 1 Min -> 1 Neu
            // If host does not have hasAlchemyCombinations(), the lesser Refinery ability takes precedence!
            int which = getAlchemyFCodeValue(shipFCode, m_hostVersion, m_key, m_hostVersion.hasRefineryFCodes());
            int supplies = 0x7FFF;  // we assume that no ship has more cargo
            if ((which & AlchemyTri) != 0) {
                doRefinery(ship, ship.tritanium,  supplies, *pHull);
            }
            if ((which & AlchemyDur) != 0) {
                doRefinery(ship, ship.duranium,   supplies, *pHull);
            }
            if ((which & AlchemyMol) != 0) {
                doRefinery(ship, ship.molybdenum, supplies, *pHull);
            }
        } else if (real_ship->hasSpecialFunction(HullFunction::NeutronicRefinery, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
            // Neutronic refinery converts 1 Sup + 1 Min -> 1 Neu
            int which = getAlchemyFCodeValue(shipFCode, m_hostVersion, m_key, m_hostVersion.hasRefineryFCodes());
            int supplies = ship.supplies.orElse(0);
            if ((which & AlchemyTri) != 0) {
                doRefinery(ship, ship.tritanium,  supplies, *pHull);
            }
            if ((which & AlchemyDur) != 0) {
                doRefinery(ship, ship.duranium,   supplies, *pHull);
            }
            if ((which & AlchemyMol) != 0) {
                doRefinery(ship, ship.molybdenum, supplies, *pHull);
            }
            ship.supplies = supplies;
        } else {
            // Not an alchemy ship
        }
    }

    // Starbase Missions I (Fix, [Recycle, Load Torps])
    if (const Planet* p = univ.planets().get(pid)) {
        // @change PCC2 required Playable but I don't see why ReadOnly shouldn't do.
        if (p->isPlayable(Object::ReadOnly)
            && p->hasFullBaseData()
            && p->getBaseShipyardAction().orElse(0) == FixShipyardAction
            && p->getBaseShipyardId().orElse(0) == id)
        {
            // we're at a base which is fixing us
            ship.damage = 0;
            ship.crew = pHull->getMaxCrew();
        }
    }

    // Supply Repair
    int shipDamage = ship.damage.orElse(0);
    int shipSupplies = ship.supplies.orElse(0);
    if (shipDamage > 0 && shipSupplies > 0) {
        int max = shipSupplies / 5;
        if (max > shipDamage) {
            max = shipDamage;
        }
        ship.damage = shipDamage - max;
        ship.supplies = shipSupplies - 5*max;
    }

    // Cloak Fuel Burn
    bool canCloak = real_ship->hasSpecialFunction(HullFunction::Cloak, m_scoreDefinitions, m_shipList, m_hostConfiguration);
    bool canAdvancedCloak = real_ship->hasSpecialFunction(HullFunction::AdvancedCloak, m_scoreDefinitions, m_shipList, m_hostConfiguration);
    if ((canCloak || canAdvancedCloak) && m_shipList.missions().isMissionCloaking(ship.mission.orElse(0), ship.owner.orElse(0), m_hostConfiguration, m_hostVersion)) {
        // ex shipacc.pas:CloakFuel (sort-of)
        int neededFuel = canAdvancedCloak ? 0 : getCloakFuel(0, real_ship->getRealOwner().orElse(0), m_hostConfiguration, *pHull);
        int haveFuel = ship.neutronium.orElse(0);
        if (haveFuel <= neededFuel
            || (ship.damage.orElse(0) >= m_hostConfiguration[HostConfiguration::DamageLevelForCloakFail]()
                && !real_ship->hasSpecialFunction(HullFunction::HardenedCloak, m_scoreDefinitions, m_shipList, m_hostConfiguration)))
        {
            // We cancel only cloak missions here. Other missions are NOT canceled, see below.
            ship.mission = 0;
        } else {
            ship.neutronium = haveFuel - neededFuel;
            cloak_fuel_used += neededFuel;
        }
    }

    /* This used to check for fuelless ships at this place, and set
       them to mission zero. However, since this is mainly used for
       fuel predictions, this is not a good idea. We want to know how
       much we need, even if you have too little. This mainly affects
       the Tow mission. */

    /* damage speed limit */
    shipDamage = ship.damage.orElse(0);
    if (shipDamage > 0 && !real_ship->hasSpecialFunction(HullFunction::HardenedEngines, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
        int limit = (m_hostConfiguration.getPlayerRaceNumber(ship.owner.orElse(0)) == 2
                     ? (m_hostVersion.getKind() == HostVersion::PHost
                        ? 15 - shipDamage/10
                        : 14 - shipDamage/10)
                     : 10 - shipDamage/10);
        if (limit < 0) {
            limit = 0;
        }
        if (ship.warpFactor.orElse(0) > limit) {
            ship.warpFactor = limit;
        }
    }

    /* Actual movement here */
    int32_t dist2 = (int32_t(ship.waypointDX.orElse(0) * ship.waypointDX.orElse(0))
                     + int32_t(ship.waypointDY.orElse(0) * ship.waypointDY.orElse(0)));
    if (real_ship->hasSpecialFunction(HullFunction::Hyperdrive, m_scoreDefinitions, m_shipList, m_hostConfiguration)
        && shipFCode == "HYP"
        && ship.warpFactor.orElse(0) > 0
        && dist2 >= 20*20)   // FIXME: minimum distance not in PHost?
    {
        // It's hyperjumping
        ship.neutronium = ship.neutronium.orElse(0) - 50;      // FIXME: do not produce negative values!!!1
        movement_fuel_used += 50;

        // If it's jumping, it can't tow. Advance time in towee's world anyway.
        if (ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
            ship.mission = 0;
        }
        if (towee_override) {
            towee_override->computeTurn();
        }

        // Now move that bugger.
        if (m_hostVersion.isExactHyperjumpDistance2(dist2)) {
            ship.x = ship.x.orElse(0) + ship.waypointDX.orElse(0);
            ship.y = ship.y.orElse(0) + ship.waypointDY.orElse(0);
        } else {
            // non-exact jump
            double dist = 350 / std::sqrt(double(dist2));
            int mx = int(dist * std::abs(ship.waypointDX.orElse(0)) + 0.4999999);
            int my = int(dist * std::abs(ship.waypointDY.orElse(0)) + 0.4999999);
            if (ship.waypointDX.orElse(0) < 0) {
                mx = -mx;
            }
            if (ship.waypointDY.orElse(0) < 0) {
                my = -my;
            }
            ship.x = ship.x.orElse(0) + mx;
            ship.y = ship.y.orElse(0) + my;
        }
        ship.waypointDX = ship.waypointDY = 0;
        ship.warpFactor = 0;
        normalizePosition(ship, univ.config());
        // FIXME: IF pconf<>NIL THEN s.FC := '???';
        // // FIXME: gravity wells?
    } else if (dist2 > 0 && ship.warpFactor.orElse(0) > 0) {
        // Normal movement
        // First, compute new position in mx,my
        double dist = std::sqrt(double(dist2));
        int way = ship.warpFactor.orElse(0) * ship.warpFactor.orElse(0);
        if (real_ship->hasSpecialFunction(HullFunction::Gravitonic, m_scoreDefinitions, m_shipList, m_hostConfiguration)) {
            way *= 2;
        }

        int mx = ship.waypointDX.orElse(0), my = ship.waypointDY.orElse(0);
        if (dist > way) {
            int dx, dy;
            if (m_hostVersion.getKind() != HostVersion::PHost) {
                // THost movement formulas, from Donovan's
                if (std::abs(mx) > std::abs(my)) {
                    dx = int(double(way) * std::abs(mx) / dist + 0.5);
                    dy = int(double(dx) * std::abs(my) / std::abs(mx) + 0.5);
                } else {
                    dy = int(double(way) * std::abs(my) / dist + 0.5);
                    dx = int(double(dy) * std::abs(mx) / std::abs(my) + 0.5);
                }
                if (mx < 0) {
                    dx = -dx;
                }
                if (my < 0) {
                    dy = -dy;
                }
            } else {
                // PHost. From docs and source.
                double head = util::getHeadingRad(mx, my);
                double fx = std::sin(head) * way;
                double fy = std::cos(head) * way;
                dx = int(fx);
                dy = int(fy);
                if (dx != fx) {
                    dx += sgn(fx);
                }
                if (dy != fy) {
                    dy += sgn(fy);
                }
                if (mx == 0) {
                    dx = 0;
                }
                if (my == 0) {
                    dy = 0;
                }
            }

            // we now have the dx,dy we want to move
            mx = dx;
            my = dy;
            dist = way;         // FIXME: use distFromDX(dx,dy) instead?
        }

        // Advance time in towee. Must be here because we need to know its "post-movement" mass.
        if (towee_override) {
            if (ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
                // we assume the tow succeeds. FIXME: be more clever?
                if (towee_override->ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
                    towee_override->ship.mission = 0;
                }
                towee_override->ship.warpFactor = 0;
            }
            towee_override->computeTurn();
        }

        // Compute fuel usage.
        int fuel = computeFuelUsage(univ, ship,
                                    towee_override ? towee_override->id : 0,
                                    towee_override ? &towee_override->ship : 0,
                                    real_ship->hasSpecialFunction(HullFunction::Gravitonic, m_scoreDefinitions, m_shipList, m_hostConfiguration),
                                    dist,
                                    m_shipList, m_hostConfiguration, m_hostVersion);
        ship.neutronium = ship.neutronium.orElse(0) - fuel;
        movement_fuel_used += fuel;

        // We still have the position offset in mx,my. Move it.
        ship.x = ship.x.orElse(0) + mx;
        ship.y = ship.y.orElse(0) + my;
        ship.waypointDX = ship.waypointDX.orElse(0) - mx;
        ship.waypointDY = ship.waypointDY.orElse(0) - my;
        normalizePosition(ship, univ.config());

        // Warp wells
        int wp_x = ship.x.orElse(0) + ship.waypointDX.orElse(0);
        int wp_y = ship.y.orElse(0) + ship.waypointDY.orElse(0);

        if (ship.warpFactor.orElse(0) > 1 && univ.getPlanetAt(Point(ship.x.orElse(0), ship.y.orElse(0))) == 0) {
            Id_t gpid = univ.getGravityPlanetAt(Point(ship.x.orElse(0), ship.y.orElse(0)), m_hostConfiguration, m_hostVersion);
            if (const Planet* p = univ.planets().get(gpid)) {
                // Okay, there is a planet. Move the ship.
                Point ppos;
                if (p->getPosition(ppos)) {
                    ship.x = ppos.getX();
                    ship.y = ppos.getY();

                    // Now adjust the waypoint. If the waypoint was inside
                    // the warp well of the same planet, assume the end of
                    // this movement order. Otherwise, when users set a
                    // waypoint at the edge of the warp well, the ship
                    // would try to get there for ever. This is consistent
                    // with the CCScript `MoveTo' command and with what
                    // most people expect.
                    if (univ.getPlanetAt(Point(wp_x, wp_y), true, m_hostConfiguration, m_hostVersion) == gpid) {
                        ship.waypointDX = 0;
                        ship.waypointDY = 0;
                    } else {
                        Point new_wp = univ.config().getSimpleNearestAlias(Point(wp_x, wp_y), Point(ship.x.orElse(0), ship.y.orElse(0)));
                        ship.waypointDX = new_wp.getX() - ship.x.orElse(0);
                        ship.waypointDY = new_wp.getY() - ship.y.orElse(0);
                    }
                }
            }
        }

        // Update towee position
        if (towee_override) {
            if (ship.mission.orElse(0) == game::spec::Mission::msn_Tow) {
                towee_override->ship.x = ship.x;
                towee_override->ship.y = ship.y;
                towee_override->ship.waypointDX = 0;
                towee_override->ship.waypointDY = 0;
            }
        }
    } else {
        // No sensible movement order for this ship. Advance towee's time anyway.
        if (towee_override) {
            towee_override->computeTurn();
        }
    }

    // Turn fuel usage
    int fuel = computeTurnFuel(*real_ship, m_hostConfiguration, m_shipList, 1);
    if (ship.neutronium.orElse(0) >= 0) {
        ship.neutronium = std::max(0, ship.neutronium.orElse(0) - fuel);
    } else {
        // already dropped below 0. Keep it this way to let them see they consume too much.
    }

    // Turn is over.
    ++turns_computed;
}

// /** Compute this ship's movement, until movement is over or time runs out. */
void
game::map::ShipPredictor::computeMovement()
{
    // ex GShipTurnPredictor::computeMovement
    // ex shipacc.pas:ComputeMovement
    if (valid) {
        const int final_turn = turns_computed + MOVEMENT_TIME_LIMIT;
        while ((ship.waypointDX.orElse(0) || ship.waypointDY.orElse(0)) && turns_computed < final_turn) {
            computeTurn();
            if (ship.neutronium.orElse(0) < 0) {
                ship.neutronium = 0;
            }
        }
    }
}

// /** Override this ship's position. */
void
game::map::ShipPredictor::setPosition(Point pt)
{
    // ex GShipTurnPredictor::setPosition
    pt = univ.config().getSimpleCanonicalLocation(pt);
    ship.x = pt.getX();
    ship.y = pt.getY();
}

// /** Override this ship's waypoint. */
void
game::map::ShipPredictor::setWaypoint(Point pt)
{
    // ex GShipTurnPredictor::setWaypoint
    pt = univ.config().getSimpleNearestAlias(pt, Point(ship.x.orElse(0), ship.y.orElse(0)));
    ship.waypointDX = pt.getX() - ship.x.orElse(0);
    ship.waypointDY = pt.getY() - ship.y.orElse(0);
}

// /** Override this ship's speed. */
void
game::map::ShipPredictor::setWarpFactor(int warp)
{
    // ex GShipTurnPredictor::setSpeed
    ship.warpFactor = warp;
}

// /** Override this ship's engine. */
void
game::map::ShipPredictor::setEngineType(int engine)
{
    // ex GShipTurnPredictor::setEngine
    ship.engineType = engine;
}

// /** Override this ship's mission. */
void
game::map::ShipPredictor::setMission(int m, int i, int t)
{
    // ex GShipTurnPredictor::setMission
    ship.mission                   = m;
    ship.missionInterceptParameter = i;
    ship.missionTowParameter       = t;
}

// /** Override this ship's friendly code. */
void
game::map::ShipPredictor::setFriendlyCode(String_t s)
{
    // ex GShipTurnPredictor::setFCode
    ship.friendlyCode = s;
}

// /** Override this ship's amount of fuel. */
void
game::map::ShipPredictor::setFuel(int fuel)
{
    // ex GShipTurnPredictor::setFuel
    ship.neutronium = fuel;
}

// /** Check whether this ship has reached its waypoint. */
bool
game::map::ShipPredictor::isAtWaypoint() const
{
    // ex GShipTurnPredictor::isAtWaypoint
    return ship.waypointDX.orElse(0) == 0 && ship.waypointDY.orElse(0) == 0;
}

// /** Get this ship's current position. */
game::map::Point
game::map::ShipPredictor::getPosition() const
{
    // ex GShipTurnPredictor::getPosition
    return Point(ship.x.orElse(0), ship.y.orElse(0));
}

// /** Get this ship's current amount of fuel. */
int
game::map::ShipPredictor::getFuel() const
{
    // ex GShipTurnPredictor::getFuel
    return ship.neutronium.orElse(0);
}

// /** Get cargo amount. */
int
game::map::ShipPredictor::getCargo(Element::Type el) const
{
    // ex GShipTurnPredictor::getCargo
    switch (el) {
     case Element::Neutronium:
        return ship.neutronium.orElse(0);
     case Element::Tritanium:
        return ship.tritanium.orElse(0);
     case Element::Duranium:
        return ship.duranium.orElse(0);
     case Element::Molybdenum:
        return ship.molybdenum.orElse(0);
     case Element::Fighters:
        return ship.numBays.orElse(0) > 0 ? ship.ammo.orElse(0) : 0;
     case Element::Colonists:
        return ship.colonists.orElse(0);
     case Element::Supplies:
        return ship.supplies.orElse(0);
     case Element::Money:
        return ship.money.orElse(0);
     default:
        int tt;
        if (Element::isTorpedoType(el, tt) && tt == ship.launcherType.orElse(0) && ship.numLaunchers.orElse(0) > 0) {
            // We know it has this type of torpedoes
            return ship.ammo.orElse(0);
        } else {
            return 0;
        }
    }
}

// /** Get this ship's current speed. */
int
game::map::ShipPredictor::getWarpFactor() const
{
    // ex GShipTurnPredictor::getSpeed
    return ship.warpFactor.orElse(0);
}

// /** Get this ship's real owner. */
int
game::map::ShipPredictor::getRealOwner() const
{
    // ex GShipTurnPredictor::getRealOwner
    return ship.owner.orElse(0);
}

// /** Get this ship's friendly code. */
String_t
game::map::ShipPredictor::getFriendlyCode() const
{
    // ex GShipTurnPredictor::getFCode
    return ship.friendlyCode.orElse(0);
}

// /** Get the universe used for predicting. */
const game::map::Universe&
game::map::ShipPredictor::getUniverse() const
{
    // ex GShipTurnPredictor::getUniverse
    return univ;
}


void
game::map::ShipPredictor::init()
{
    // ex GShipTurnPredictor::init
    const Ship* p = univ.ships().get(id);
    if (p != 0 && p->hasFullShipData()) {
        p->getCurrentShipData(this->ship);
        ship.owner = p->getRealOwner();
        valid = true;
    } else {
        valid = false;
    }
}
