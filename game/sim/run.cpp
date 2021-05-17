/**
  *  \file game/sim/run.cpp
  *  \brief Simulator Main Entry Point
  *
  *  As of 20200923, this is pretty close port of PCC2.
  */

#include <climits>
#include <cmath>
#include <memory>
#include "game/sim/run.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "game/battleorderrule.hpp"
#include "game/hostversion.hpp"
#include "game/sim/ability.hpp"
#include "game/sim/configuration.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/result.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/v3/structures.hpp"          // VCR capabilities
#include "game/vcr/classic/algorithm.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"
#include "game/vcr/classic/types.hpp"
#include "game/vcr/flak/algorithm.hpp"
#include "game/vcr/flak/configuration.hpp"
#include "game/vcr/flak/database.hpp"
#include "game/vcr/flak/gameenvironment.hpp"
#include "game/vcr/flak/nullvisualizer.hpp"
#include "game/vcr/flak/setup.hpp"
#include "game/vcr/object.hpp"
#include "game/vcr/statistic.hpp"
#include "util/math.hpp"

using afl::base::Ptr;
using afl::except::checkAssertion;
using game::BattleOrderRule;
using game::HostVersion;
using game::config::HostConfiguration;
using game::sim::Configuration;
using game::sim::Object;
using game::sim::Planet;
using game::sim::Result;
using game::sim::Setup;
using game::sim::Ship;
using game::spec::Cost;
using game::spec::Engine;
using game::spec::Hull;
using game::spec::ShipList;
using game::vcr::Statistic;
using util::RandomNumberGenerator;
using util::roundToInt;

namespace {

    template<typename T>
    T& mustExist(T* p)
    {
        checkAssertion(p, "unexpected null object");
        return *p;
    }

    void initializeStats(std::vector<Statistic>& stats, const Setup& setup)
    {
        // ex GSimState::initializeStats
        /* min_fighters_aboard needs some care: for ships, we have good reasons to initialize
           it to getAmmo() because that's the number of fighters initially on board.
           For planets, we'd need to duplicate the formulas here.
           Therefore, we just initialize it it INT_MAX, and filter it out in display.
           Also see runSimulation() documentation. */
        stats.clear();
        for (Setup::Slot_t i = 0; i < setup.getNumShips(); ++i) {
            const Ship& sh = mustExist(setup.getShip(i));

            // Sim ship to VCR object
            game::vcr::Object obj;
            obj.setNumFighters(sh.getAmmo());

            // VCR object to Statistic
            stats.push_back(Statistic());
            stats.back().init(obj, 0);
        }
        if (setup.hasPlanet()) {
            // Fake VCR object
            game::vcr::Object obj;
            obj.setNumFighters(INT_MAX);

            // VCR object to Statistic
            stats.push_back(Statistic());
            stats.back().init(obj, 0);
        }
    }

    Statistic* getStatistic(const afl::base::Memory<Statistic>& stats, const Setup& setup, const Object* obj)
    {
        Setup::Slot_t slot;
        if (setup.findIndex(obj, slot)) {
            return stats.at(slot);
        } else {
            return 0;
        }
    }


    struct GlobalModificators {
        // Commander level base: a Commander ship of level X gives each ship with a lower level a +1 boost.
        // This boost propagates to allies and remains for the turn even if the commander dies.
        game::PlayerArray<int> levelBase;

        // Number of shield generators: applies to only the player itself, immediately lost if shield generator ship dies.
        game::PlayerArray<int> numShieldGenerators;

        // Cloaked fighter bays: applies to only the player itself, only one per fight, immediately lost if ship dies.
        // We need the identity of the providing ship.
        game::PlayerArray<const Ship*> cloakedBaysHelper;
    };


    int plimit(int max, int scale, int d)
    {
        return max - (max * d) / scale;
    }


    int getSeed(const Configuration& opts, const Result& result, RandomNumberGenerator& rng)
    {
        // ex ccsim.pas:NewSeed (sort-of)
        if (opts.hasSeedControl()) {
            if (opts.getMode() == Configuration::VcrNuHost) {
                return (result.this_battle_index % 118) + 1;
            } else {
                return (result.this_battle_index % 110) + 1;
            }
        } else {
            if (opts.getMode() == Configuration::VcrHost) {
                return rng(110) + 1;
            } else if (opts.getMode() == Configuration::VcrNuHost) {
                return rng(118) + 1;
            } else {
                return rng();
            }
        }
    }

    int getDamageTech(int tech, int damage)
    {
        int max = (100 - damage) / 10;
        if (tech > max)
            tech = max;
        if (tech <= 0)
            tech = 1;
        return tech;
    }

    /** Bonus fighter table for Master at Arms compensation.
        First index is 0 for carrier/carrier, 1 for planet/carrier.
        Second index is number of effective bays of right carrier, plus 1.
        Third index is number of effective bays of left carrier, plus 1.
        (Effective bays has a range of -1 to 13).
        Value is average number of bonus fighters times 10, i.e. 64 means
        we're getting 6 bonus fighters, and with 40% probability, we get
        another bonus fighter. */
    const uint8_t master_bonus_fighters_X10[2][15][15] = {
         //-1   0   1   2   3   4   5   6   7   8   9  10  11  12  13
        {{  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },  // -1
         {  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2 },  // 0
         {  3,  3,  3,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7 },  // 1
         {  4,  7,  9, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19 },  // 2
         {  4,  7,  9, 21, 19, 24, 26, 26, 27, 27, 27, 27, 27, 27, 27 },  // 3
         {  4,  7,  9, 21, 26, 35, 38, 40, 40, 40, 40, 40, 40, 40, 40 },  // 4
         {  4,  7,  9, 25, 33, 40, 44, 48, 52, 53, 53, 53, 53, 53, 53 },  // 5
         {  4,  7,  9, 25, 34, 45, 50, 52, 56, 58, 61, 64, 64, 64, 64 },  // 6
         {  4,  7,  9, 25, 37, 50, 54, 58, 62, 64, 67, 72, 75, 75, 75 },  // 7
         {  4,  7,  9, 25, 37, 53, 60, 66, 69, 73, 75, 78, 82, 82, 82 },  // 8
         {  4,  7,  9, 25, 37, 53, 63, 69, 71, 76, 79, 82, 86, 89, 90 },  // 9
         {  4,  7,  9, 25, 37, 53, 63, 71, 74, 78, 82, 86, 94, 95, 96 },  // 10
         {  4,  7,  9, 25, 37, 53, 63, 71, 80, 82, 84, 89, 98, 99,100 },  // 11
         {  4,  7,  9, 25, 38, 53, 63, 71, 80, 85, 89, 93, 99,101,104 },  // 12
         {  4,  7,  9, 25, 38, 53, 63, 71, 80, 85, 89, 94, 99,102,106 }}, // 13

        {{  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
         {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
         {  2,  2,  2,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },
         {  2,  4,  5, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 },
         {  2,  4,  5, 11, 10, 12, 13, 13, 14, 14, 14, 14, 14, 14, 14 },
         {  2,  4,  5, 11, 13, 18, 19, 20, 20, 20, 20, 20, 20, 20, 20 },
         {  2,  4,  5, 13, 17, 20, 22, 24, 26, 27, 27, 27, 27, 27, 27 },
         {  2,  4,  5, 13, 17, 23, 25, 26, 28, 29, 31, 32, 32, 32, 32 },
         {  2,  4,  5, 13, 19, 25, 27, 29, 31, 32, 34, 36, 38, 38, 38 },
         {  2,  4,  5, 13, 19, 27, 30, 33, 35, 37, 38, 39, 41, 41, 41 },
         {  2,  4,  5, 13, 19, 27, 32, 35, 36, 38, 40, 41, 43, 45, 45 },
         {  2,  4,  5, 13, 19, 27, 32, 36, 37, 39, 41, 43, 47, 48, 48 },
         {  2,  4,  5, 13, 19, 27, 32, 36, 40, 41, 42, 45, 49, 50, 50 },
         {  2,  4,  5, 13, 19, 27, 32, 36, 40, 43, 45, 47, 50, 51, 52 },
         {  2,  4,  5, 13, 19, 27, 32, 36, 40, 43, 45, 47, 50, 51, 53 }}
    };

    /** Bonus bay table for Master at Arms compensation.
        First index is 0 for carrier/carrier, 1 for planet/carrier.
        Second index is number of effective bays of right carrier, plus 1.
        Third index is number of effective bays of left carrier, plus 1.
        (Effective bays has a range of -1 to 13).
        Value is average number of bonus bays times 100, i.e. 108 means
        we receive 1 bonus bay, plus another one with 8% probability. */
    const uint8_t master_bonus_bays_X100[2][15][15] = {
         //-1   0   1   2   3   4   5   6   7   8   9  10  11  12  13
        {{  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },  // -1
         {  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2 },  // 0
         {  2,  2,  2,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5 },  // 1
         {  3,  5,  7, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },  // 2
         {  3,  5,  7, 16, 14, 18, 20, 20, 20, 20, 20, 20, 20, 20, 20 },  // 3
         {  3,  5,  7, 16, 20, 27, 29, 30, 30, 30, 30, 30, 30, 30, 30 },  // 4
         {  3,  6,  7, 20, 26, 32, 35, 38, 41, 42, 42, 42, 42, 42, 42 },  // 5
         {  4,  7,  9, 24, 33, 43, 48, 50, 54, 56, 59, 62, 62, 62, 62 },  // 6
         {  5,  8, 10, 28, 42, 57, 61, 66, 71, 73, 76, 82, 85, 85, 85 },  // 7
         {  5,  9, 12, 33, 49, 70, 79, 87, 91, 96, 99,103,108,108,108 },  // 8
         {  6, 10, 13, 37, 55, 79, 94,103,106,114,118,123,129,133,135 },  // 9
         {  7, 12, 15, 42, 62, 89,106,120,125,131,138,145,158,160,162 },  // 10
         {  8, 13, 17, 47, 69, 99,118,133,150,154,158,167,184,186,188 },  // 11
         {  8, 15, 19, 52, 79,110,131,147,166,176,185,193,205,209,216 },  // 12
         {  9, 16, 21, 57, 87,121,144,162,182,194,203,214,226,232,241 }}, // 13

        {{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
         {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
         {  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 },
         {  1,  1,  2,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },
         {  1,  1,  2,  4,  4,  4,  5,  5,  5,  6,  6,  6,  6,  6,  6 },
         {  1,  1,  2,  4,  5,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8 },
         {  1,  2,  2,  6,  8,  9, 10, 11, 12, 12, 12, 12, 12, 12, 12 },
         {  1,  2,  3,  7, 10, 13, 14, 15, 16, 17, 17, 18, 18, 18, 18 },
         {  1,  2,  3,  8, 12, 17, 18, 20, 21, 22, 23, 24, 25, 25, 25 },
         {  2,  3,  4, 10, 14, 21, 23, 26, 27, 29, 29, 30, 32, 32, 32 },
         {  2,  3,  4, 11, 16, 24, 28, 31, 32, 34, 35, 37, 38, 40, 40 },
         {  2,  4,  5, 13, 19, 27, 32, 36, 37, 39, 41, 43, 47, 48, 48 },
         {  2,  4,  5, 14, 21, 30, 35, 40, 45, 46, 47, 50, 55, 55, 56 },
         {  2,  4,  6, 16, 24, 33, 39, 44, 50, 53, 55, 58, 61, 63, 64 },
         {  3,  5,  6, 17, 26, 36, 43, 49, 55, 58, 61, 64, 68, 70, 72 }}
    };

    /**************************** HOST Simulation ****************************/

    /* FIXME: these functions make up a host version.
       There should be a way to use the actual host version to simulate the version used in the game. */

    int getFCodeValueTHost(const Object& a)
    {
        HostVersion host(HostVersion::Host, MKVERSION(3,22,48));
        return BattleOrderRule(host).get(a);
    }

    bool sortByBattleOrderTHost(const Object* a, const Object* b)
    {
        int abo = getFCodeValueTHost(*a);
        int bbo = getFCodeValueTHost(*b);
        if (abo != bbo) {
            return abo < bbo;
        }
        return a->getId() < b->getId();
    }

    /* Predicate to sort by Id number, backwards. Used for Tim-Host intercept attack. */
    bool sortByIdBackwards(const Object* a, const Object* b)
    {
        return a->getId() > b->getId();
    }

    int getFCodeValuePHost(const Object& a)
    {
        HostVersion host(HostVersion::PHost, MKVERSION(4,0,0));
        return BattleOrderRule(host).get(a);
    }

    bool sortByBattleOrderPHost(const Object* a, const Object* b)
    {
        int abo = getFCodeValuePHost(*a);
        int bbo = getFCodeValuePHost(*b);
        if (abo != bbo) {
            return abo < bbo;
        }
        if (a->getId() != b->getId()) {
            return a->getId() < b->getId();
        }
        return dynamic_cast<const Ship*>(a) != 0;
    }

    /* Check whether friendly code is exempted from matching.
       This tries to emulate the simulated host.

       @change PCC/PCC2 consider only extra friendly codes (xtrfcode.txt) as exceptions,
       which can arguably be a pretty exact rule interpretation for the actual host.
       (hardcoded codes are only exempt in Host, extra codes only exist in PHost where they are exempt). */
    bool isFriendlyCodeExemptFromMatch(const String_t& friendlyCode, const Configuration& opts, const ShipList& list)
    {
        switch (opts.getMode()) {
         case Configuration::VcrHost:
         case Configuration::VcrNuHost:
            // Host exempts ATT/NUK (for planets) and mkt/NTP/lfm (for ships) from matching.
            // Those are checked by the caller.
            // Assume NuHost does the same.
            return false;

         case Configuration::VcrPHost2:
         case Configuration::VcrPHost3:
         case Configuration::VcrPHost4:
         case Configuration::VcrFLAK:
            // PHost and FLAK exempt all special friendly codes from matching.
            return list.friendlyCodes().isSpecial(friendlyCode, false);
        }
        return false;
    }

    /* Check whether two ships attack.
       Checks whether 'at' attacks 'op'.
       Checks only one direction! */
    bool isAttacking(const Ship& at, const Ship& op, const Configuration& opts, const ShipList& list, const HostConfiguration& config)
    {
        // ex ccsim.pas:IsAttacking, ccsim.pas:Attacking
        // ex isAttacking(const GSimShip& at, const GSimShip& op, const GSimOptions& opts)
        /* Conditions according to host.exe:
           no fight if players are the same                             (1)
           no fight if right ship offers alliance to left ship          (2)
           if CloakedShipsAttack=0:
             no fight if any ship cloaked                               (3)
           else
             no fight if both ships cloaked                             (4)
             no fight if PE of cloaked ship does not match other ship   (5)
           no fight if either ship already dead                         (6)
           no fight if either ship is fuelless                          (7)
           no fight if neither ship has beams nor bays                  (8)
           no fight if fcode match and fcode not lfm,NTP,mkt            (9)
           no fight if either ship has mission 0 and 10 crew            (10)

           otherwise, fight happens if there's a PE match or mission 4  (11)

           Deviations: (2) is handled symmetrically; should probably test
           it and if it's relevant, implement it. (8) is implemented as
           real freighter test. It remains to test whether (10) is
           relevant.

           Note that (5) implicitly follows from (11), so we don't check
           that explicitly. */

        /* Conditions according to PHost:
           no fight if either ship already dead                         (p1)  (=6)
           no fight if aggressor has no fuel                            (p2)  (=7)
           no fight if aggressor has neither PE nor mission 4           (p3)  (=11)
           no fight if players are the same                             (p4)  (=1)
           no fight if aggressor offers alliance to opponent            (p5)  (=2)
           no fight if opponent has no fuel                             (p6)  (=7)
           if CloakedShipsAttack=0:
             no fight if either ship is cloaked                         (p7)  (=3)
           else
             no fight if both ships cloaked                             (p8)  (=4)
             no fight if opponent is cloaked and has mismatching PE     (p9)
           no fight if FC match                                         (p11) (=9)

           These are almost the same rules as HOST. (5) follows from (11)
           = (p3), so it need not be implemented. (8) is not implemented
           in PHost up to 4.0j(!!), which is considered a bug. */

        /* deactivated units do not fight */
        if (((at.getFlags() | op.getFlags()) & Object::fl_Deactivated) != 0) {
            return false;
        }
        /* same owner does not fight */
        if (at.getOwner() == op.getOwner()) {
            return false;
        }
        /* zombies do not fight */
        if (at.getOwner() == 0 || op.getOwner() == 0) {
            return false;
        }
        /* friends do not fight */
        if (opts.hasHonorAlliances() && opts.allianceSettings().get(at.getOwner(), op.getOwner())) {
            return false;
        }
        /* passive or fuelless units do not attack */
        if (at.getAggressiveness() == Ship::agg_Passive || at.getAggressiveness() == Ship::agg_NoFuel) {
            return false;
        }
        /* we have a PE, so check whether opponent matches */
        if (at.getAggressiveness() != at.agg_Kill) {
            if (op.getOwner() != at.getAggressiveness() && !opts.enemySettings().get(at.getOwner(), op.getOwner())) {
                return false;
            }
        }
        /* check for cloaking */
        if ((at.getFlags() & Ship::fl_Cloaked) != 0) {
            if (!config[HostConfiguration::AllowCloakedShipsAttack]()) {
                return false;
            }
        }
        /* check whether enemy is cloaked */
        /* FIXME: PHost has silly exception here: you *can* attack a cloaked
           ship if they have you as their enemy */
        if ((op.getFlags() & Ship::fl_Cloaked) != 0) {
            return false;
        }

        /* check for fuel */
        if (op.getAggressiveness() == Ship::agg_NoFuel) {
            return false;
        }

        /* Now check friendly codes */
        const String_t afc = at.getFriendlyCode();
        if (afc == op.getFriendlyCode()) {
            if (afc != "mkt" && afc != "lfm" && afc != "NTP" && afc != "???" && !isFriendlyCodeExemptFromMatch(afc, opts, list)) {
                return false;
            }
        }

        return true;
    }

    /* Check whether a ship is immune from planet attacks. */
    bool isImmune(const Ship& sh, const Configuration& opts, const ShipList& list, const HostConfiguration& config)
    {
        // ex isImmune(const GSimShip& sh, const GSimOptions& opts)
        /* Conditions in Host 3.22.40:
           - immune if owned by Rebel and !PlanetsAttackRebels [handled by hasShipFunction]
           - immune if owned by Klingons and !PlanetsAttackKlingons [handled by hasShipFunction]
           - immune if owned by Birds, has no fuel, but has beams
           - immune if it is a SSD
           - immune if ship is cloaked */
        // FIXME: more hull functions?
        if (config.getPlayerRaceNumber(sh.getOwner()) == 3 && sh.getAggressiveness() == sh.agg_NoFuel && sh.getNumBeams() != 0) {
            return true;
        }
        if (sh.hasAbility(game::sim::PlanetImmunityAbility, opts, list, config)) {
            return true;
        }
        if (sh.getFlags() & Object::fl_Cloaked) {
            return true;
        }

        return false;
    }

    /* Check whether a ship/planet attack each other.
       Unlike the version which takes two GSimShip, this one tests both directions of aggression. */
    bool isAttacking(const Ship& left, const Planet& right, const Configuration& opts, const ShipList& list, const HostConfiguration& config)
    {
        // isAttacking(const GSimShip& left, const GSimPlanet& right, const GSimOptions& opts)
        /* Host 3.22.40:
           - if planet.fc="NUK" && AllowPlanetAttacks && planet.defense=0 && ship.owner=5
             then planet.defense <- 1

           no fight if ship is cloaked                           (1)
           fight if ship is aggressive:
             no alliance offer                                   (2)
             and mission kill or matching PE                     (3)
             and has fuel                                        (4)
             and not fcode match                                 (5)

           fight if planet is aggressive:
             no alliance offer                                   (6)
             and ship not immune                                 (7)
             and "ATT", AllowPlanetAttacks, ship has fuel        (8)
                 or "NUK", AllowPlanetAttacks, planet has defense */

        /* PHost differentiates between aggressor and opponent:

           if planet is aggressor:
             no fight if neither ATT nor NUK, or                 (p1) = (8)
               !AllowPlanetAttacks
             no fight if ship is immune                          (p2) = (7)
             no fight if ship is fuel-less and (fcode not NUK    (p3) = (7)
                or fuelless capital bird ship)

           if planet is opponent:
             no fight if out of fuel                             (p4) = (4)
             no fight if neither kill nor PE                     (p5) = (3)
             no fight if PE mismatch                             (p6) = (3)

           no fight if same owner                                (p7)
           no fight if aggressor offers alliance                 (p8) = (2)+(6)
           no fight if ship is cloaked                           (p9) = (7)+(1)
           no fight if fcode match                               (p10) = (5) */

        /* deactivated units don't fight */
        if (((left.getFlags() | right.getFlags()) & Object::fl_Deactivated) != 0) {
            return false;
        }
        /* same owner does not fight */
        if (left.getOwner() == right.getOwner()) {
            return false;
        }
        /* zombies do not fight */
        if (left.getOwner() == 0 || right.getOwner() == 0) {
            return false;
        }
        /* cloaked ships do not fight */
        if ((left.getFlags() & Object::fl_Cloaked) != 0) {
            return false;
        }
        /* same FCode does not fight */
        String_t fc = left.getFriendlyCode();
        if (fc == right.getFriendlyCode()) {
            if (fc != "ATT" && fc != "NUK" && fc != "???" && !isFriendlyCodeExemptFromMatch(fc, opts, list)) {
                return false;
            }
        }

        /* does the ship want to attack the planet? */
        bool ship_wants_attack;
        if (opts.hasHonorAlliances() && opts.allianceSettings().get(left.getOwner(), right.getOwner())) {
            ship_wants_attack = false;
        } else if (left.getAggressiveness() == left.agg_Kill) {
            ship_wants_attack = true;
        } else if (left.getAggressiveness() == left.agg_NoFuel || left.getAggressiveness() == left.agg_Passive) {
            ship_wants_attack = false;
        } else if (left.getAggressiveness() == right.getOwner() || opts.enemySettings().get(left.getOwner(), right.getOwner())) {
            ship_wants_attack = true;
        } else {
            ship_wants_attack = false;
        }

        /* does the planet want to attack the ship? */
        /* FIXME: handle the "1 dp" condition, and the anti-NUK-trap rule */
        bool planet_wants_attack;
        if (opts.hasHonorAlliances() && opts.allianceSettings().get(right.getOwner(), left.getOwner())) {
            planet_wants_attack = false;
        } else if (right.getFriendlyCode() == "ATT") {
            planet_wants_attack = (left.getAggressiveness() != left.agg_NoFuel);
        } else if (right.getFriendlyCode() == "NUK") {
            planet_wants_attack = true;
        } else {
            planet_wants_attack = false;
        }

        return ship_wants_attack
            || (planet_wants_attack && !isImmune(left, opts, list, config));
    }

    /** Check whether any two objects attack each other. Unlike that
        isAttacking() functions, this one can take any object combination
        in any order. */
    bool isAttackingAny(const Object& a, const Object& b, const Configuration& opts, const ShipList& list, const HostConfiguration& config)
    {
        // isAttackingAny(const GSimObject& a, const GSimObject& b, const GSimOptions& opts)
        const Ship* as   = dynamic_cast<const Ship*>(&a);
        const Ship* bs   = dynamic_cast<const Ship*>(&b);
        const Planet* ap = dynamic_cast<const Planet*>(&a);
        const Planet* bp = dynamic_cast<const Planet*>(&b);

        if (as && bs) {
            return isAttacking(*as, *bs, opts, list, config) || isAttacking(*bs, *as, opts, list, config);
        }
        if (as && bp) {
            return isAttacking(*as, *bp, opts, list, config);
        }
        if (ap && bs) {
            return isAttacking(*bs, *ap, opts, list, config);
        }
        return false;
    }

    /* Check whether ship is armed. */
    bool isArmed(const Ship& sh)
    {
        return sh.getNumBeams() != 0
            || sh.getNumLaunchers() != 0
            || sh.getNumBays() != 0;
    }

    /* Pack ship into VCR record. */
    void packShip(game::vcr::Object& obj, const Ship& sh, const Configuration& opts, const ShipList& list, const HostConfiguration& config)
    {
        // ex packShip(GVcrObject& obj, const GSimShip& sh, const GSimOptions& opts)
        obj.setIsPlanet(false);
        if (sh.getNumBays() != 0) {
            obj.setNumFighters(sh.getAmmo());
            obj.setNumTorpedoes(0);
            obj.setNumLaunchers(0);
            obj.setNumBays(sh.getNumBays());
            obj.setTorpedoType(0);
        } else if (sh.getNumLaunchers() != 0) {
            obj.setNumFighters(0);
            obj.setNumTorpedoes(sh.getAmmo());
            obj.setNumLaunchers(sh.getNumLaunchers());
            obj.setNumBays(0);
            obj.setTorpedoType(sh.getTorpedoType());
        } else {
            obj.setNumFighters(0);
            obj.setNumTorpedoes(0);
            obj.setNumLaunchers(0);
            obj.setNumBays(0);
            obj.setTorpedoType(0);
        }
        obj.setName(sh.getName());
        obj.setDamage(sh.getDamage());
        obj.setCrew(sh.getCrew());
        obj.setId(sh.getId());
        obj.setOwner(sh.getOwner());
        obj.setRace(config.getPlayerRaceNumber(sh.getOwner()));
        obj.setBeamKillRate(config.getPlayerRaceNumber(sh.getOwner()) == 5 ? 3 : 1);
        obj.setBeamType(sh.getBeamType());
        obj.setNumBeams(sh.getNumBeams());
        obj.setExperienceLevel(opts.isExperienceEnabled(config) ? sh.getExperienceLevel() : 0);
        obj.setShield(sh.getShield());

        if (sh.getHullType() != 0) {
            const Hull& h = mustExist(list.hulls().get(sh.getHullType()));
            obj.setMass(h.getMass());
            obj.setPicture(h.getInternalPictureNumber());
            obj.setHull(sh.getHullType());
        } else {
            obj.setMass(sh.getMass());
            obj.setPicture(200); // FIXME: hardcoded; this is the same value as used in PCC
            obj.setHull(0);
        }

        // Not set/left at defaults: beamChargeRate, torpMissRate, torpChargeRate, crewDefenseRate.
        // These are set in applyShipModificators.
    }

    /* Check whether friendly code states an ammunition limit.
       \return Maximum number of torpedoes/fighters to use, -1 if no limit */
    int getFCodeAmmoLimit(const String_t fc)
    {
        // ex getFCodeAmmoLimit(const string_t fc)
        // This originally called for making this configurable.
        // However, this is a simulator, so leave it enabled all the time - why not?
        if (fc.size() == 3 && fc[0] == 'N' && fc[1] == 'T') {
            if (fc[2] == 'P') {
                return 0;                               /* NTP */
            } else if (fc[2] == '0') {
                return 100;                             /* NT0 */
            } else if (fc[2] >= '1' && fc[2] <= '9') {
                return 10*(fc[2] - '0');                /* NT1..NT9 */
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }

    int getUnusedAmmo(int previous_ammo, int limit)
    {
        if (limit >= 0 && limit < previous_ammo) {
            return previous_ammo - limit;
        } else {
            return 0;
        }
    }

    /* Unpack ship from VCR to simulation data.

       Note: this routine must not be called more than once on a given vcr/sh pair.
       Otherwise, torps might get lost when NTx is used.

       This routine also performs post-simulation modificators. */
    void unpackShip(const game::vcr::Object& obj, Ship& sh, const GlobalModificators& mods)
    {
        // ex unpackShip(const GVcrObject& obj, GSimShip& sh, const GlobalModificators& mods)
        // Copy values
        sh.setShield(obj.getShield());
        sh.setDamage(obj.getDamage());
        sh.setCrew(obj.getCrew());

        // PHost, NuHost do fed bonus here - now in applyShipModificators for the next fight

        const int limit = getFCodeAmmoLimit(sh.getFriendlyCode());
        if (sh.getNumBays() != 0) {
            // Cloaked Fighter Bays Peer
            Ship* peer = const_cast<Ship*>(mods.cloakedBaysHelper.get(obj.getOwner()));

            // Previous ammo
            int previous_ammo = sh.getAmmo();
            if (peer != 0) {
                previous_ammo += peer->getAmmo();
            }

            // Unused ammo
            const int unused_ammo = getUnusedAmmo(previous_ammo, limit);

            int fighter_loss = sh.getAmmo() - (obj.getNumFighters() + unused_ammo);
            if (Ship* peer = const_cast<Ship*>(mods.cloakedBaysHelper.get(obj.getOwner()))) {
                // We have a peer that also contributes fighters, so it also suffices loss.
                // Above, fighter_loss has been computed as the loss of (ship, before) to (ship+peer, after),
                // so we need to add (peer, before) first.
                fighter_loss += peer->getAmmo();

                // Peer loss is proportional to the bay distribution, but peer cannot lose more than it has.
                int peer_loss = std::min(peer->getAmmo(), fighter_loss * peer->getNumBays() / (peer->getNumBays() + sh.getNumBays()));
                peer->setAmmo(peer->getAmmo() - peer_loss);
                fighter_loss -= peer_loss;
            }
            sh.setAmmo(sh.getAmmo() - fighter_loss);
        } else if (sh.getNumLaunchers()) {
            int unused_ammo = getUnusedAmmo(sh.getAmmo(), limit);
            sh.setAmmo(obj.getNumTorpedoes() + unused_ammo);
        }
    }

    /* Apply ship modificators to VCR.
       - Engine-Shield Bonus
       - Bonus Bays
       - Fed Shield / Mass Bonus
       - no shields for freighters
       - damage limits
       - NTP

       \param [in/out] obj            VCR unit containing the ship.
       \param [in]     againstPlanet  true if opponent is a planet
       \param [in]     sh             Original simulation ship data
       \param [in]     opts           Simulation options
       \param [in]     list           Ship list
       \param [in]     config         Host configuration
       \param [in]     mods           Modificators
       \param [in]     first          true if this is the first fight

       \pre all data belonging to the ship must be initialized. */
    void applyShipModificators(game::vcr::Object& obj, bool againstPlanet, const Ship& sh, const Configuration& opts, const ShipList& list,
                               const HostConfiguration& config, const GlobalModificators& mods, bool first)
    {
        // ex applyShipModificators, ccsim.pas:ApplyBonuses
        /* engine-shield bonus */
        int num_sg = mods.numShieldGenerators.get(obj.getOwner());
        int bonus = 50*num_sg;
        bool hosty = opts.getMode() == Configuration::VcrHost || opts.getMode() == Configuration::VcrNuHost;
        if (!againstPlanet || (!hosty && config[HostConfiguration::AllowESBonusAgainstPlanets]())) {
            bonus += opts.getEngineShieldBonus();
        }
        if (!hosty) {
            bonus += config.getExperienceBonus(HostConfiguration::EModEngineShieldBonusRate, obj.getExperienceLevel());
        }
        if (bonus != 0) {
            // FIXME: HOST uses mass = ERND(mass + bonus*rate/100)
            const Engine& e = mustExist(list.engines().get(sh.getEngineType()));
            obj.addMass(int32_t(bonus) * e.cost().get(Cost::Money) / 100);
        }

        /* bonus bays and scotty bonus */
        int bonus_fighters = 0;
        bonus = 0;
        if (config.getPlayerRaceNumber(sh.getOwner()) == 1 && opts.hasScottyBonus()) {
            /* only THost bonus bays; PHost bonus bays handled below */
            if (hosty) {
                bonus += 3;
            }
            obj.addMass(50);
            if (opts.getMode() == Configuration::VcrHost || !first) {
                // HOST gives shield bonus before every fight.
                // NuHost and PHost give bonus after every fight. We don't want to give a bonus
                // after the last fight to have realistic stats, thus we give it before the second.
                obj.setShield(obj.getShield() + 25);
            }
        }
        if (const Ship* sh = mods.cloakedBaysHelper.get(obj.getOwner())) {
            bonus += sh->getNumBays();
            bonus_fighters += sh->getAmmo();
        }

        /* Shield limit incl. shield generator */
        const int shield_limit = 100 + 50*num_sg;
        obj.setShield(std::max(0, std::min(obj.getShield() + num_sg*25, shield_limit - obj.getDamage())));

        if (!hosty) {
            bonus += config[HostConfiguration::ExtraFighterBays](sh.getOwner());
            bonus += config.getExperienceBonus(HostConfiguration::EModExtraFighterBays, obj.getExperienceLevel());
        }
        if (obj.getNumBays() != 0) {
            obj.addBays(bonus);
            obj.addFighters(bonus_fighters);
            // Note that we need "bonus" later on for damage limits
        }

        /* Freighters have no shields */
        if (!isArmed(sh)) {
            obj.setShield(0);
        }

        /* NTP & Co. */
        int limit = getFCodeAmmoLimit(sh.getFriendlyCode());
        if (limit >= 0) {
            if (obj.getNumFighters() > limit) {
                obj.setNumFighters(limit);
            }
            if (obj.getNumTorpedoes() > limit) {
                obj.setNumTorpedoes(limit);
            }
        }

        /* Damage limitations */
        if ((config.getPlayerRaceNumber(sh.getOwner()) != 1 || !opts.hasScottyBonus())
            && !sh.hasAbility(game::sim::FullWeaponryAbility, opts, list, config))
        {
            if (hosty) {
                int limit = 10 - obj.getDamage() / 10;
                if (config.getPlayerRaceNumber(sh.getOwner()) == 2) {
                    limit += 5;
                }
                if (limit < 0) {
                    limit = 0;
                }
                obj.setNumLaunchers(std::min(obj.getNumLaunchers(), limit));
                /* Bay bonus can be
                   - "+3" fed bonus (not here, this is the non-fed branch)
                   - "ExtraFighterBays" or "EModExtraFighterBays" (not here, this is the hosty branch)
                   - "Cloaked Fighter Bays"
                   Thus, this "+bonus" only includes cloaked fighter bays which is just what we want */
                obj.setNumBays         (std::min(obj.getNumBays(), limit + bonus));
                obj.setNumBeams        (std::min(obj.getNumBeams(), limit));
            } else {
                int limit = 100;
                if (config.getPlayerRaceNumber(sh.getOwner()) == 2) {
                    limit = 150;
                }
                if (sh.getHullType() == 0) {
                    obj.setNumLaunchers(std::min(obj.getNumLaunchers(), plimit(sh.getNumLaunchers(),    limit, sh.getDamage())));
                    obj.setNumBays     (std::min(obj.getNumBays(),      plimit(sh.getNumBays() + bonus, limit, sh.getDamage())));
                    obj.setNumBeams    (std::min(obj.getNumBeams(),     plimit(sh.getNumBeams(),        limit, sh.getDamage())));
                } else {
                    const Hull& h = mustExist(list.hulls().get(sh.getHullType()));
                    obj.setNumLaunchers(std::min(obj.getNumLaunchers(), plimit(h.getMaxLaunchers(),    limit, sh.getDamage())));
                    obj.setNumBays     (std::min(obj.getNumBays(),      plimit(h.getNumBays() + bonus, limit, sh.getDamage())));
                    obj.setNumBeams    (std::min(obj.getNumBeams(),     plimit(h.getMaxBeams(),        limit, sh.getDamage())));
                }
            }
        }

        /* simplifications */
        if (obj.getNumLaunchers() == 0) {
            obj.setNumTorpedoes(0);
            obj.setTorpedoType(0);
        }
        if (obj.getNumBays() == 0) {
            obj.setNumFighters(0);
        }
        if (obj.getNumBeams() == 0) {
            obj.setBeamType(0);
        }

        /* Finally, add level bonus */
        if (obj.getExperienceLevel() < mods.levelBase.get(sh.getOwner())) {
            obj.setExperienceLevel(obj.getExperienceLevel() + 1);
        }

        /* Special abilities */
        obj.setBeamKillRate   (sh.hasAbility(game::sim::TripleBeamKillAbility,      opts, list, config) ?   3 : 1);
        obj.setBeamChargeRate (sh.hasAbility(game::sim::DoubleBeamChargeAbility,    opts, list, config) ?   2 : 1);
        obj.setTorpChargeRate (sh.hasAbility(game::sim::DoubleTorpedoChargeAbility, opts, list, config) ?   2 : 1);
        obj.setCrewDefenseRate(sh.hasAbility(game::sim::SquadronAbility,            opts, list, config) ? 100 : 0);
    }

    /* Apply modificators that apply to an opponent. */
    void applyOpponentModificators(game::vcr::Object& obj, const Ship& opp, const Configuration& opts, const ShipList& shipList, const HostConfiguration& config)
    {
        /* "Elusive" ability (Nu). Documented as "Ship has 10% rate of being hit by torpedoes",
           but the combat code only has a hit rate on the opponent. So I assume it is implemented
           this way: */
        if (opp.hasAbility(game::sim::ElusiveAbility, opts, shipList, config)) {
            obj.setTorpMissRate(90);
        }
    }

    /* Apply planet modificators to VCR.
       - Commander level bonus
       - Special abilities
       \pre all data belonging to the planet must be initialized. */
    void applyPlanetModificators(game::vcr::Object& obj, const Planet& pl, const Configuration& opts, const ShipList& list, const HostConfiguration& config, const GlobalModificators& mods)
    {
        // ex applyPlanetModificators(GVcrObject& obj, const GSimPlanet& pl, const GSimOptions& opts, const GlobalModificators& mods)
        /* add level bonus */
        if (obj.getExperienceLevel() < mods.levelBase.get(obj.getOwner())) {
            obj.setExperienceLevel(obj.getExperienceLevel() + 1);
        }

        /* Special abilities */
        obj.setBeamKillRate  (pl.hasAbility(game::sim::TripleBeamKillAbility,      opts, list, config) ? 3 : 1);
        obj.setBeamChargeRate(pl.hasAbility(game::sim::DoubleBeamChargeAbility,    opts, list, config) ? 2 : 1);
        obj.setTorpChargeRate(pl.hasAbility(game::sim::DoubleTorpedoChargeAbility, opts, list, config) ? 2 : 1);
    }

    /* Apply Master at Arms bonus.
       Master at Arms is a proposal from Sirius (Jan Klingele) to balance combat in Tim-Host.
       It has, so far, not been implemented anywhere, but CCBSim has it (it used to be a proof-of-concept implementation of the proposal),
       so we offer it, too.
       The balancing approach attempts to fix the fighter intercept imbalance due to Tim's biased random number generator.
       It applies to fighter/fighter battles only.

       \param [in/out] left   Left unit
       \param [in/out] right  Right unit
       \param [in/out] result Result (bonus may increase series length)
       \param [in]     opts   Simulation options
       \param [in]     rng    Random number generator */
    void applyMasterBonus(game::vcr::Object& left, game::vcr::Object& right, Result& result, const Configuration& opts, RandomNumberGenerator& rng)
    {
        // ex applyMasterBonus, ccsim.pas:ApplyMasterBonus
        /* only for fighter/fighter battles */
        if (left.getNumBays() == 0 || right.getNumBays() == 0) {
            return;
        }

        /* Compute maximum number of bonus fighters. This part of the
           algorithm is *not* in my paper / Winword copy of Master at
           Arms, and unfortunately, I cannot find my electronic
           communication with the Master at Arms people as well. However,
           this part is in PCC 1.x's ccsim.pas, so it probably wound up
           there after a suggestion by Sirius. The idea is to avoid giving
           a bonus if ships have so few fighters that fighter intercept is
           not an issue. */
        int eleft  = left.getNumFighters() - 2 * right.getNumBeams();
        int eright = right.getNumFighters() - 2 * left.getNumBeams();
        if (left.getShield() >= 100) {
            eright -= left.getNumBeams();
        }
        if (right.getShield() >= 100) {
            eleft -= right.getNumBeams();
        }

        /* we now have eleft, eright = effective fighters aboard
           (the fighters which do harm to the enemy, i.e. are not
           destroyed immediately after launch */
        int max_ef = std::max(0, std::min(eleft, eright));

        /* maximum bonus fighters 14% of max_ef. This computes
           10 times the maximum bonus fighters, and rounds it. */
        int max_bonus = (max_ef * 14 + 5) / 10;

        /* The following is straight from Master at Arms:
           Compute effective bay count. We add one to offset C indexing. */
        eleft  = left.getNumBays() - (right.getNumBeams() + 2) / 5 + 1;
        eright = right.getNumBays() - (left.getNumBeams() + 2) / 5 + 1;
        eleft  = std::max(0, std::min(eleft,  14));
        eright = std::max(0, std::min(eright, 14));

        /* Now apply the bonus */
        int bonus_bays_100    = master_bonus_bays_X100[right.isPlanet()][eright][eleft];
        int bonus_fighters_10 = master_bonus_fighters_X10[right.isPlanet()][eright][eleft];
        if (bonus_fighters_10 > max_bonus) {
            bonus_fighters_10 = max_bonus;
        }

        right.addBays(bonus_bays_100 / 100);
        right.addFighters(bonus_fighters_10 / 10);

        if (opts.hasSeedControl()) {
            if (result.addSeries(2)) {
                /* give bay bonus */
                right.addBays(1);
                result.this_battle_weight *= bonus_bays_100 % 100;
            } else {
                /* no bonus */
                result.this_battle_weight *= 100 - (bonus_bays_100 % 100);
            }
            result.total_battle_weight *= 100;

            /* FIXME: this can generate battles of weight 0 (PCC 1.x has
               the same problem). Solution (a): filter these out before
               running them. Solution (b): avoid generating them. This is
               not easy because input to this routine is not deterministic
               in case random left-right is used. */
            if (result.addSeries(2)) {
                /* give fighter bonus */
                right.addFighters(1);
                result.this_battle_weight *= bonus_fighters_10 % 10;
            } else {
                /* no bonus */
                result.this_battle_weight *= 10 - (bonus_fighters_10 % 10);
            }
            result.total_battle_weight *= 10;
        } else {
            if (rng(100) < bonus_bays_100 % 100) {
                right.addBays(1);
            }
            if (rng(10) < bonus_fighters_10 % 10) {
                right.addFighters(1);
            }
        }
    }

    /* Pack planet into VCR. */
    void packPlanet(game::vcr::Object& obj, const Planet& pl, const Configuration& opts, const ShipList& list, const HostConfiguration& config)
    {
        // ex packPlanet(GVcrObject& obj, const GSimPlanet& pl, const GSimOptions& opts)
        const Configuration::VcrMode mode = opts.getMode();
        const bool has_base = pl.getBaseBeamTech() > 0;

        const int planet_defense = pl.getDefense();
        const int base_defense   = has_base ? pl.getBaseDefense() : 0;
        const int base_fighters  = has_base ? pl.getNumBaseFighters() : 0;

        obj.setIsPlanet(true);
        if (mode != Configuration::VcrHost && mode != Configuration::VcrNuHost) {
            /* PHost */
            const int32_t eff_p_defense = planet_defense * (int32_t(100) - pl.getDamage()) / 100;
            const int32_t eff_bp_defense = (planet_defense + base_defense) * (int32_t(100) - pl.getDamage()) / 100;
            const int weapon_limit = config[HostConfiguration::AllowAlternativeCombat]() ? 20 : 10;

            obj.setNumFighters(roundToInt(std::sqrt(double(eff_p_defense))) + base_fighters);
            obj.setNumTorpedoes(0);
            obj.setNumLaunchers(0);
            obj.setName(pl.getName());
            obj.setDamage(pl.getDamage());
            obj.setCrew(0);
            obj.setId(pl.getId());
            obj.setOwner(pl.getOwner());
            obj.setRace(config.getPlayerRaceNumber(pl.getOwner()));
            obj.setBeamKillRate(config.getPlayerRaceNumber(pl.getOwner()) == 5 ? 3 : 1);
            obj.setPicture(1);
            obj.setHull(0);
            obj.setBeamType(roundToInt(std::sqrt(eff_p_defense / 2.0)));

            if (has_base && getDamageTech(pl.getBaseBeamTech(), pl.getBaseDamage()) > obj.getBeamType()) {
                obj.setBeamType(getDamageTech(pl.getBaseBeamTech(), pl.getBaseDamage()));
            }
            if (obj.getBeamType() < 1) {
                obj.setBeamType(1);
            }
            if (obj.getBeamType() > list.beams().size()) {
                obj.setBeamType(list.beams().size());
            }
            obj.setNumBeams(roundToInt(std::sqrt(eff_bp_defense / 3.0)));
            if (obj.getNumBeams() > weapon_limit) {
                obj.setNumBeams(weapon_limit);
            }
            obj.setExperienceLevel(opts.isExperienceEnabled(config) ? pl.getExperienceLevel() : 0);
            obj.setNumBays(obj.getNumFighters() - base_fighters);
            if (has_base) {
                obj.addBays(5);
            }
            obj.setTorpedoType(0);
            obj.setMass(100 + eff_p_defense + base_defense * (int32_t(100) - pl.getDamage()) / 100);
            obj.setShield(pl.getShield());

            if (config[HostConfiguration::PlanetsHaveTubes]()) {
                obj.setTorpedoType(roundToInt(std::sqrt(eff_p_defense / 2.0)));
                if (has_base && getDamageTech(pl.getBaseTorpedoTech(), pl.getBaseDamage()) > obj.getTorpedoType()) {
                    obj.setTorpedoType(getDamageTech(pl.getBaseTorpedoTech(), pl.getBaseDamage()));
                }
                if (obj.getTorpedoType() > list.launchers().size()) {
                    obj.setTorpedoType(list.launchers().size());
                }
                obj.setNumLaunchers(roundToInt(std::sqrt(eff_bp_defense / 4.0)));
                if (obj.getNumLaunchers() > 20) {// FIXME: is this correct?
                    obj.setNumLaunchers(20);
                }

                /* planetary torps */
                int ppt = config[HostConfiguration::PlanetaryTorpsPerTube](obj.getOwner());
                ppt += config.getExperienceBonus(HostConfiguration::EModPlanetaryTorpsPerTube, obj.getExperienceLevel());

                obj.setNumTorpedoes(ppt * obj.getNumLaunchers());

                /* add base storage torps */
                if (config[HostConfiguration::UseBaseTorpsInCombat](pl.getOwner())) {
                    int32_t cost = 0;
                    for (int i = 1; i <= list.launchers().size(); ++i) {
                        cost += pl.getNumBaseTorpedoes(i) * mustExist(list.launchers().get(i)).torpedoCost().get(Cost::Money);
                    }
                    if (obj.getTorpedoType() > 0) {
                        int torp_cost = mustExist(list.launchers().get(obj.getTorpedoType())).torpedoCost().get(Cost::Money);
                        if (torp_cost > 0) {
                            obj.addTorpedoes(cost / torp_cost);
                        }
                    }
                }
                if (obj.getNumTorpedoes() > 255) {
                    obj.setNumTorpedoes(255);
                }
            }
        } else {
            /* Host */
            obj.setNumFighters(roundToInt(std::sqrt(double(planet_defense))) + base_fighters);
            obj.setNumTorpedoes(0);
            obj.setNumLaunchers(0);

            obj.setName(pl.getName());

            obj.setDamage(0);
            obj.setCrew(obj.getNumFighters());                /* HOST does that, so we do it too,
                                                                 in case we someday export the VCRs
                                                                 to vcr.exe */
            obj.setId(pl.getId());
            obj.setOwner(pl.getOwner());
            obj.setRace(config.getPlayerRaceNumber(pl.getOwner()));
            obj.setBeamKillRate(config.getPlayerRaceNumber(pl.getOwner()) == 5 ? 3 : 1);
            obj.setPicture(1);
            obj.setHull(0);

            obj.setBeamType(roundToInt(std::sqrt(planet_defense / 2.0)));
            if (has_base && pl.getBaseBeamTech() > obj.getBeamType()) {
                obj.setBeamType(pl.getBaseBeamTech());
            }
            if (obj.getBeamType() < 1) {
                obj.setBeamType(1);
            }
            if (obj.getBeamType() > list.beams().size()) {
                obj.setBeamType(list.beams().size());
            }

            obj.setNumBeams(roundToInt(std::sqrt((planet_defense + base_defense) / 3.0)));
            if (obj.getNumBeams() > 10) {
                obj.setNumBeams(10);
            }

            obj.setExperienceLevel(opts.isExperienceEnabled(config) ? pl.getExperienceLevel() : 0);

            obj.setNumBays(roundToInt(std::sqrt(double(planet_defense))));
            if (base_fighters != 0) {
                obj.addBays(5);
            }

            obj.setTorpedoType(0);
            obj.setMass(100 + planet_defense + base_defense);
            obj.setShield(pl.getShield());
            if (planet_defense == 0 && base_defense == 0) {
                obj.setShield(0);
            }
        }

        // Not set/left at defaults: beamChargeRate, torpMissRate, torpChargeRate, crewDefenseRate.
        // These are set in applyPlanetModificators.
    }

    /* Unpack planet from VCR. */
    void unpackPlanet(const game::vcr::Object& obj, Planet& pl, const game::vcr::Object& orig_obj, const Configuration& opts, const ShipList& list, const HostConfiguration& config)
    {
        // ex unpackPlanet(const GVcrObject& obj, GSimPlanet& pl, const GVcrObject& orig_obj, const GSimOptions& opts)
        int fighters_lost = orig_obj.getNumFighters() - obj.getNumFighters();
        int integrity_remaining = 100 - obj.getDamage();
        if (integrity_remaining < 0) {
            integrity_remaining = 0;
        }

        if (opts.getMode() != Configuration::VcrHost && opts.getMode() != Configuration::VcrNuHost) {
            pl.setShield(obj.getShield());
            pl.setDamage(obj.getDamage());

            if (pl.getBaseBeamTech() > 0) {
                /* remove fighters */
                int new_base_fighters = pl.getNumBaseFighters() - fighters_lost;
                if (new_base_fighters < 0) {
                    pl.setDefense(pl.getDefense() + new_base_fighters);
                    pl.setNumBaseFighters(0);
                } else {
                    pl.setNumBaseFighters(new_base_fighters);
                }

                /* remove torps */
                if (config[HostConfiguration::PlanetsHaveTubes]() && config[HostConfiguration::UseBaseTorpsInCombat](pl.getOwner())) {
                    int    torps_lost = orig_obj.getNumTorpedoes() - obj.getNumTorpedoes();
                    int32_t torp_cost = torps_lost * mustExist(list.launchers().get(obj.getTorpedoType())).torpedoCost().get(Cost::Money);
                    while (torp_cost > 0) {
                        bool did = false;
                        for (int i = 1; i <= list.launchers().size(); ++i) {
                            if (pl.getNumBaseTorpedoes(i) > 0) {
                                pl.setNumBaseTorpedoes(i, pl.getNumBaseTorpedoes(i) - 1);
                                torp_cost -= mustExist(list.launchers().get(i)).torpedoCost().get(Cost::Money);
                                did = true;
                            }
                        }
                        if (!did) {
                            break;
                        }
                    }
                }
            }
        } else {
            pl.setShield(obj.getShield());

            /* reduce defense */
            pl.setDefense(util::divideAndRoundToEven(integrity_remaining * pl.getDefense(), 100, 0));

            if (pl.getBaseBeamTech() > 0) {
                /* remove fighters */
                int new_base_fighters = pl.getNumBaseFighters() - fighters_lost;
                if (new_base_fighters < 0) {
                    pl.setDefense(pl.getDefense() + new_base_fighters);
                    pl.setNumBaseFighters(0);
                } else {
                    pl.setNumBaseFighters(new_base_fighters);
                }

                /* reduce equipment */
                pl.setBaseDefense(util::divideAndRoundToEven(integrity_remaining * pl.getBaseDefense(), 100, 0));
                pl.setBaseBeamTech(std::max(1, util::divideAndRoundToEven(integrity_remaining * pl.getBaseBeamTech(), 100, 0)));
                pl.setBaseTorpedoTech(std::max(1, util::divideAndRoundToEven(integrity_remaining * pl.getBaseTorpedoTech(), 100, 0)));

                /* add base damage */
                int new_damage = pl.getDamage() + obj.getDamage();
                if (new_damage > 100) {
                    pl.setBaseBeamTech(0);
                } else {
                    pl.setDamage(new_damage);
                }
            } else {
                /* remove fighters */
                pl.setDefense(pl.getDefense() - fighters_lost);
            }

            if (pl.getDefense() < 0) {
                pl.setDefense(0);
            }
        }
    }

    /* Handle a ship being killed.
       This implements the respawn logic for Squadrons.
       \return true iff ship respawns */
    bool handleShipKilled(Ship& sh, const Configuration& opts, const ShipList& list, const HostConfiguration& config)
    {
        // ex handleShipKilled(GSimShip& sh, const GSimOptions& opts)
        if (sh.hasAbility(game::sim::SquadronAbility, opts, list, config) && sh.getNumBeams() > 1) {
            sh.setNumBeams(sh.getNumBeams() - 1);
            sh.setDamage(0);
            sh.setShield(100);
            return true;
        } else {
            sh.setOwner(0);
            return false;
        }
    }

    /* Make ship/ship VCR. This routine also does left/right randomisation.
       \param [in/out]  db        Database (VCR will be appended here)
       \param [in/out]  leftShip  Left ship
       \param [in/out]  leftStat  Out-of-band statistic for left ship
       \param [in/out]  rightShip Right ship
       \param [in/out]  rightStat Out-of-band statistic for right ship
       \param [in]      opts      Simulation options
       \param [in]      list      Ship list
       \param [in]      config    Host configuration
       \param [in]      type      Battle algorithm
       \param [in]      mods      Global modificators
       \param [in/out]  result    Result meta-information (weight)
       \param [in/out]  rng       Random number generator
       \retval true  Call makeShipShipVcr with same parameters again (ship respawned)
       \retval false Do not call makeShipShipVcr again */
    bool makeShipShipVcr(game::vcr::classic::Database& db,
                         Ship& leftShip,
                         game::vcr::Statistic* leftStat,
                         Ship& rightShip,
                         game::vcr::Statistic* rightStat,
                         const Configuration& opts,
                         const game::vcr::classic::Type type,
                         const ShipList& list,
                         const HostConfiguration& config,
                         const GlobalModificators& mods,
                         Result& result,
                         RandomNumberGenerator& rng)
    {
        // ex ccsim.pas:MakeVCR

        /* fight? */
        if (!isAttacking(leftShip, rightShip, opts, list, config) && !isAttacking(rightShip, leftShip, opts, list, config)) {
            return false;
        }
        if (!(isArmed(leftShip) || isArmed(rightShip))) {
            return false;
        }

        /* swap them? */
        Ship *one, *two;
        game::vcr::Statistic *oneStat, *twoStat;
        bool swap_them = false;
        if (opts.hasRandomLeftRight()) {
            if (opts.hasSeedControl()) {
                swap_them = result.addSeries(2) == 0;
            } else {
                swap_them = (rng(2) == 0);
            }
        }
        if (swap_them) {
            one = &rightShip, two = &leftShip;
            oneStat = rightStat, twoStat = leftStat;
        } else {
            one = &leftShip, two = &rightShip;
            oneStat = leftStat, twoStat = rightStat;
        }

        /* set up fight */
        const bool first = db.getNumBattles() == 0;

        game::vcr::Object left, right;
        uint16_t seed = uint16_t(getSeed(opts, result, rng));
        packShip(left,  *one, opts, list, config);
        packShip(right, *two, opts, list, config);
        applyShipModificators(left,  false, *one, opts, list, config, mods, first);
        applyShipModificators(right, false, *two, opts, list, config, mods, first);
        applyOpponentModificators(left,  *two, opts, list, config);
        applyOpponentModificators(right, *one, opts, list, config);

        /* left/right balance */
        if (opts.getBalancingMode() == Configuration::Balance360k) {
            // Note: the 360 kt bonus is given *after* ESB (which is how it works in Host).
            // In combination with the Nu Sabre Shield Generator ability, this turns a tiny
            // Outrider (75 kt) into a whooping 585 kt monster using one shield generator,
            // even with no other ESB.
            if (right.getMass() > 140 && left.getNumBays() != 0) {
                if (opts.hasSeedControl()) {
                    if (result.addSeries(2)) {
                        /* give bonus */
                        right.addMass(360);
                        result.this_battle_weight *= 59;
                        result.total_battle_weight *= 100;
                    } else {
                        /* don't give bonus */
                        result.this_battle_weight *= 41;
                        result.total_battle_weight *= 100;
                    }
                } else {
                    if (rng(100) > 40) {
                        right.addMass(360);
                    }
                }
            } else {
                /* The total_battle_weight for a series of simulations
                   should depend only on the GSimOptions and the input
                   situation. Due to random left/right, it can happen that
                   for the same situation, we sometimes see the trigger
                   for Balance360k, and sometimes we don't.

                   Although GSimResultSummary can handle weights that
                   change in the middle, it's better to use the clean way.
                   A possible optimisation would apply this fix only if
                   the reverse situation actually can trigger Balance360k. */
                if (opts.hasSeedControl()) {
                    result.addSeries(2);
                    result.this_battle_weight *= 50;
                    result.total_battle_weight *= 100;
                }
            }
        } else if (opts.getBalancingMode() == Configuration::BalanceMasterAtArms) {
            /* Master-at-Arms bonus */
            applyMasterBonus(left, right, result, opts, rng);
        }

        /* run it */
        game::vcr::classic::Battle* vcr = db.addNewBattle(new game::vcr::classic::Battle(left, right, seed, 0 /* sig */, 0 /* planet temp */));
        uint16_t cap = type == game::vcr::classic::PHost4
            ? game::v3::structures::DeathRayCapability | game::v3::structures::ExperienceCapability | game::v3::structures::BeamCapability
            : 0;
        vcr->setType(type, cap);

        game::vcr::classic::NullVisualizer vis;
        std::auto_ptr<game::vcr::classic::Algorithm> player(vcr->createAlgorithm(vis, config, list));
        checkAssertion(player.get(), "create VCR player");
        checkAssertion(player->setCapabilities(cap), "VCR player refuses capabilities");
        checkAssertion(!player->checkBattle(left, right, seed), "VCR player refuses battle");

        player->playBattle(left, right, seed);
        player->doneBattle(left, right);

        // FIXME -> e.setResultFromPlayer(*player);

        /* copy back */
        unpackShip(left,  *one, mods);
        unpackShip(right, *two, mods);

        bool again = false;
        game::vcr::classic::BattleResult_t status = player->getResult();
        if (status == game::vcr::classic::LeftDestroyed) {
            // Left ship destroyed
            again = handleShipKilled(*one, opts, list, config);
        } else if (status == game::vcr::classic::RightDestroyed) {
            // Right ship destroyed
            again = handleShipKilled(*two, opts, list, config);
        } else if (status == game::vcr::classic::LeftCaptured) {
            // Left ship captured
            one->setOwner(two->getOwner());
            one->setCrew(10);
            one->setAggressiveness(Ship::agg_Passive);
        } else if (status == game::vcr::classic::RightCaptured) {
            // Right ship captured
            two->setOwner(one->getOwner());
            two->setCrew(10);
            two->setAggressiveness(Ship::agg_Passive);
        } else if (status == game::vcr::classic::Timeout) {
            // Timeout with both ships still operable
        } else {
            // results such as mutual capture or kill
            /* FIXME: it seems HOST allows mutual capture to swap ships */
            one->setOwner(0);
            two->setOwner(0);
        }

        if (oneStat != 0) {
            oneStat->merge(player->getStatistic(game::vcr::classic::LeftSide));
        }
        if (twoStat != 0) {
            twoStat->merge(player->getStatistic(game::vcr::classic::RightSide));
        }
        return again;
    }


    /* Make ship/planet VCR.
       \param [in/out]  db        Database (VCR will be appended here)
       \param [in/out]  leftShip  Left ship
       \param [in/out]  leftStat  Out-of-band statistic for left ship
       \param [in/out]  rightPlanet Right planet
       \param [in/out]  rightStat Out-of-band statistic for right planet
       \param [in]      opts      Simulation options
       \param [in]      list      Ship list
       \param [in]      config    Host configuration
       \param [in]      type      Battle algorithm
       \param [in]      mods      Global modificators
       \param [in/out]  result    Result meta-information (weight)
       \param [in/out]  rng       Random number generator
       \retval true  Call makeShipPlanetVcr with same parameters again (ship respawned)
       \retval false Do not call makeShipPlanetVcr again */
    bool makeShipPlanetVcr(game::vcr::classic::Database& db,
                           Ship& leftShip,
                           game::vcr::Statistic* leftStat,
                           Planet& rightPlanet,
                           game::vcr::Statistic* rightStat,
                           const Configuration& opts,
                           const game::vcr::classic::Type type,
                           const ShipList& list,
                           const HostConfiguration& config,
                           const GlobalModificators& mods,
                           Result& result,
                           RandomNumberGenerator& rng)
    {
        // ex makeShipPlanetVcr, ccsim.pas:MakePlanetVCR

        /* fight? */
        if (!isAttacking(leftShip, rightPlanet, opts, list, config)) {
            return false;
        }

        /* set up fight */
        const bool first = db.getNumBattles() == 0;
        uint16_t seed = uint16_t(getSeed(opts, result, rng));

        game::vcr::Object left;
        packShip(left, leftShip, opts, list, config);
        applyShipModificators(left, true, leftShip, opts, list, config, mods, first);

        game::vcr::Object right;
        packPlanet(right, rightPlanet, opts, list, config);
        if (opts.getBalancingMode() == Configuration::BalanceMasterAtArms) {
            applyMasterBonus(left, right, result, opts, rng);
        }

        applyPlanetModificators(right, rightPlanet, opts, list, config, mods);

        /* run it */
        game::vcr::Object origPlanet = right;
        game::vcr::classic::Battle* vcr = db.addNewBattle(new game::vcr::classic::Battle(left, right, seed, 0 /* sig */, 0 /* planet temp */));
        uint16_t cap = type == game::vcr::classic::PHost4
            ? game::v3::structures::DeathRayCapability | game::v3::structures::ExperienceCapability | game::v3::structures::BeamCapability
            : 0;
        vcr->setType(type, cap);

        game::vcr::classic::NullVisualizer vis;
        std::auto_ptr<game::vcr::classic::Algorithm> player(vcr->createAlgorithm(vis, config, list));
        checkAssertion(player.get(), "create VCR player");
        checkAssertion(player->setCapabilities(cap), "VCR player refuses capabilities");
        checkAssertion(!player->checkBattle(left, right, seed), "VCR player refuses battle");

        player->playBattle(left, right, seed);
        player->doneBattle(left, right);

        // FIXME -> e.setResultFromPlayer(*player);

        /* copy back */
        unpackShip(left, leftShip, mods);
        unpackPlanet(right, rightPlanet, origPlanet, opts, list, config);

        bool again = false;
        game::vcr::classic::BattleResult_t status = player->getResult();
        if (status == game::vcr::classic::LeftDestroyed) {
            // Ship destroyed
            again = handleShipKilled(leftShip, opts, list, config);
        } else if (status == game::vcr::classic::RightDestroyed) {
            // Planet destroyed
            rightPlanet.setOwner(0);
        } else if (status == game::vcr::classic::LeftCaptured) {
            // Ship captured
            leftShip.setOwner(rightPlanet.getOwner());
            leftShip.setCrew(10);
            leftShip.setAggressiveness(Ship::agg_Passive);
        } else if (status == game::vcr::classic::RightCaptured) {
            // Planet captured
            rightPlanet.setOwner(leftShip.getOwner());
            rightPlanet.setBaseBeamTech(0);
            rightPlanet.setFriendlyCode("???");
        } else if (status == game::vcr::classic::Timeout) {
            // Timeout with both units still operable
        } else {
            // Results such as mutual capture or kill
            leftShip.setOwner(0);
            rightPlanet.setOwner(0);
        }

        if (leftStat != 0) {
            leftStat->merge(player->getStatistic(game::vcr::classic::LeftSide));
        }
        if (rightStat != 0) {
            rightStat->merge(player->getStatistic(game::vcr::classic::RightSide));
        }
        return again;
    }

    /* Compute maximum experience levels of all Commander ships.
       Finds the most experienced commander ships of all players, and propagates them via alliances. */
    void computeMaximumExperienceLevels(const Setup& setup, const Configuration& opts, const ShipList& shipList, const HostConfiguration& config, game::PlayerArray<int>& result)
    {
        // ex computeMaximumExperienceLevels, ccsim.pas:ComputeMinimumLevels
        game::PlayerArray<int> tmp;
        tmp.setAll(0);
        result.setAll(0);
        if (opts.isExperienceEnabled(config)) {
            // Find maximum experience levels for all players
            for (Setup::Slot_t i = 0; i < setup.getNumShips(); ++i) {
                const Ship& sh = mustExist(setup.getShip(i));
                if ((sh.getFlags() & Ship::fl_Deactivated) == 0
                    && sh.hasAbility(game::sim::CommanderAbility, opts, shipList, config)
                    && sh.getExperienceLevel() > tmp.get(sh.getOwner()))
                {
                    tmp.set(sh.getOwner(), sh.getExperienceLevel());
                }
            }

            // Propagate levels, honoring alliances
            for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
                int level = 0;
                for (int ally = 1; ally <= game::MAX_PLAYERS; ++ally) {
                    if (ally == i || (opts.hasHonorAlliances() && opts.allianceSettings().get(ally, i))) {
                        if (level < tmp.get(ally)) {
                            level = tmp.get(ally);
                        }
                    }
                }
                result.set(i, level);
            }
        }
    }

    /* Compute helpers for one fight:
       - numShieldGenerators
       - cloakedBaysHelper */
    void computeHelpers(GlobalModificators& mods,
                        const std::vector<Object*>& battle_order,
                        const Object* ignore1,
                        const Object* ignore2,
                        const Configuration& opts,
                        const ShipList& list,
                        const HostConfiguration& config)
    {
        // ex computeHelpers
        const int MAX_SHIELD_GEN = 2;

        mods.numShieldGenerators.setAll(0);
        mods.cloakedBaysHelper.setAll(0);

        for (size_t i = 0; i < battle_order.size(); ++i) {
            const Object* obj = battle_order[i];
            if (obj != ignore1 && obj != ignore2) {
                if (const Ship* sh = dynamic_cast<const Ship*>(obj)) {
                    int owner = obj->getOwner();
                    if (owner != 0
                        && (sh->getFlags() & Object::fl_Deactivated) == 0)
                    {
                        // Shield Generator: count number of active ships
                        if (sh->hasAbility(game::sim::ShieldGeneratorAbility, opts, list, config)) {
                            if (int* pValue = mods.numShieldGenerators.at(owner)) {
                                if (*pValue < MAX_SHIELD_GEN) {
                                    ++*pValue;
                                }
                            }
                        }

                        // Cloaked Fighter Bays: Nu docs don't say how the fighter provider is chosen.
                        // For now, chose first in battle order.
                        if ((sh->getFlags() & Object::fl_Cloaked) != 0
                            && sh->getNumBays() != 0
                            && sh->hasAbility(game::sim::CloakedBaysAbility, opts, list, config))
                        {
                            if (mods.cloakedBaysHelper.get(owner) == 0) {
                                mods.cloakedBaysHelper.set(owner, sh);
                            }
                        }
                    }
                }
            }
        }
    }

    /* Intercept-attack main loop */
    bool doInterceptAttacks(Setup& setup,
                            const Configuration& opts,
                            Result& result,
                            afl::base::Memory<Statistic> stats,
                            const ShipList& list,
                            const HostConfiguration& config,
                            util::RandomNumberGenerator& rng,
                            game::vcr::classic::Type type,
                            game::vcr::classic::Database& db,
                            GlobalModificators& mods,
                            const std::vector<Object*>& battle_order)
    {
        for (std::vector<Object*>::size_type interceptor = 0; interceptor < battle_order.size(); ++interceptor) {
            if (Ship* iship = dynamic_cast<Ship*>(battle_order[interceptor])) {
                Ship* target;
                if (iship->getInterceptId() != 0 &&
                    (target = setup.findShipById(iship->getInterceptId())) != 0
                    && target != iship)
                {
                    bool loop = true;
                    while (loop) {
                        computeHelpers(mods, battle_order, target, iship, opts, list, config);
                        loop = makeShipShipVcr(db, *target, getStatistic(stats, setup, target),
                                               *iship, getStatistic(stats, setup, iship),
                                               opts, type, list, config, mods, result, rng);
                        if (db.getNumBattles() != 0 && opts.hasOnlyOneSimulation()) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    /* General battle order main loop */
    bool doCombatOrder(Setup& setup,
                       const Configuration& opts,
                       Result& result,
                       afl::base::Memory<Statistic> stats,
                       const ShipList& list,
                       const HostConfiguration& config,
                       util::RandomNumberGenerator& rng,
                       game::vcr::classic::Type type,
                       game::vcr::classic::Database& db,
                       GlobalModificators& mods,
                       const std::vector<Object*>& battle_order)
    {
        for (std::vector<Object*>::size_type right = 0; right < battle_order.size(); ++right) {
            for (std::vector<Object*>::size_type left = 0; left < battle_order.size(); ++left) {
                if (left != right) {
                    bool loop = true;
                    while (loop) {
                        loop = false;
                        computeHelpers(mods, battle_order, battle_order[left], battle_order[right], opts, list, config);
                        if (Ship* lship = dynamic_cast<Ship*>(battle_order[left])) {
                            if (Ship* rship = dynamic_cast<Ship*>(battle_order[right])) {
                                loop = makeShipShipVcr(db, *lship, getStatistic(stats, setup, lship),
                                                       *rship, getStatistic(stats, setup, rship),
                                                       opts, type, list, config, mods, result, rng);
                            } else if (Planet* rplan = dynamic_cast<Planet*>(battle_order[right])) {
                                loop = makeShipPlanetVcr(db, *lship, getStatistic(stats, setup, lship),
                                                         *rplan, getStatistic(stats, setup, rplan),
                                                         opts, type, list, config, mods, result, rng);
                            }
                        } else if (Planet* lplan = dynamic_cast<Planet*>(battle_order[left])) {
                            if (Ship* rship = dynamic_cast<Ship*>(battle_order[right])) {
                                loop = makeShipPlanetVcr(db, *rship, getStatistic(stats, setup, rship),
                                                         *lplan, getStatistic(stats, setup, lplan),
                                                         opts, type, list, config, mods, result, rng);
                            }
                        }
                        if (db.getNumBattles() != 0 && opts.hasOnlyOneSimulation()) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }


    /* Generate simulation according to HOST/NuHost rules. */
    void simulateHost(Setup& setup,
                      const Configuration& opts,
                      Result& result,
                      afl::base::Memory<Statistic> stats,
                      const ShipList& list,
                      const HostConfiguration& config,
                      util::RandomNumberGenerator& rng,
                      game::vcr::classic::Type type)
    {
        Ptr<game::vcr::classic::Database> db = new game::vcr::classic::Database();
        result.battles = db;

        /* compute Commander level limits */
        GlobalModificators mods;
        computeMaximumExperienceLevels(setup, opts, list, config, mods.levelBase);

        /* compute battle order */
        std::vector<Object*> battle_order;
        for (Setup::Slot_t i = 0; i < setup.getNumShips(); ++i) {
            if ((setup.getShip(i)->getFlags() & Object::fl_Deactivated) == 0) {
                battle_order.push_back(setup.getShip(i));
            }
        }

        /* simulate intercept-attack. */
        std::sort(battle_order.begin(), battle_order.end(), sortByIdBackwards);
        if (doInterceptAttacks(setup, opts, result, stats, list, config, rng, type, *db, mods, battle_order)) {
            return;
        }

        /* simulate. Outer loop selects right ship, inner loop selects left ship. */
        std::sort(battle_order.begin(), battle_order.end(), sortByBattleOrderTHost);
        if (doCombatOrder(setup, opts, result, stats, list, config, rng, type, *db, mods, battle_order)) {
            return;
        }

        /* simulate fight vs planet */
        if (setup.hasPlanet()) {
            for (std::vector<Object*>::size_type left = 0; left < battle_order.size(); ++left) {
                Ship* leftShip = dynamic_cast<Ship*>(battle_order[left]);
                if (leftShip != 0) {
                    bool loop = true;
                    while (loop) {
                        computeHelpers(mods, battle_order, leftShip, setup.getPlanet(), opts, list, config);
                        loop = makeShipPlanetVcr(*db, *leftShip, getStatistic(stats, setup, leftShip),
                                                 *setup.getPlanet(), getStatistic(stats, setup, setup.getPlanet()),
                                                 opts, type, list, config, mods, result, rng);
                        if (db->getNumBattles() != 0 && opts.hasOnlyOneSimulation()) {
                            return;
                        }
                    }
                }
            }
        }
    }

    /* Generate simulate according to PHost rules */
    void simulatePHost(Setup& setup,
                       const Configuration& opts,
                       Result& result,
                       afl::base::Memory<Statistic> stats,
                       const ShipList& list,
                       const HostConfiguration& config,
                       util::RandomNumberGenerator& rng,
                       game::vcr::classic::Type type)
    {
        Ptr<game::vcr::classic::Database> db = new game::vcr::classic::Database();
        result.battles = db;

        /* compute Commander level limits */
        GlobalModificators mods;
        computeMaximumExperienceLevels(setup, opts, list, config, mods.levelBase);

        /* prepare planet */
        if (Planet* p = setup.getPlanet()) {
            p->setShield(100);
            p->setDamage(0);
        }

        /* compute battle order */
        std::vector<Object*> battle_order;
        for (Setup::Slot_t i = 0; i < setup.getNumShips(); ++i) {
            if ((setup.getShip(i)->getFlags() & Object::fl_Deactivated) == 0) {
                battle_order.push_back(setup.getShip(i));
            }
        }
        if (setup.hasPlanet()) {
            battle_order.push_back(setup.getPlanet());
        }
        std::sort(battle_order.begin(), battle_order.end(), sortByBattleOrderPHost);

        /* simulate intercept-attack. */
        if (doInterceptAttacks(setup, opts, result, stats, list, config, rng, type, *db, mods, battle_order)) {
            goto out;
        }

        /* simulate. Outer loop picks aggressor, inner loop picks opponent */
        if (doCombatOrder(setup, opts, result, stats, list, config, rng, type, *db, mods, battle_order)) {
            goto out;
        }

     out:
        /* postprocess planet */
        if (Planet* p = setup.getPlanet()) {
            p->setDefense(p->getDefense() * (100 - p->getDamage()) / 100);
            if (p->getBaseBeamTech() > 0) {
                int base_damage = p->getBaseDamage() + p->getDamage();
                if (base_damage >= 100) {
                    p->setBaseBeamTech(0); // delete base
                } else {
                    p->setBaseDamage(base_damage);
                    p->setBaseDefense(p->getBaseDefense() * (100 - p->getDamage()) / 100);
                    p->setBaseBeamTech(getDamageTech(p->getBaseBeamTech(), p->getDamage()));
                    p->setBaseTorpedoTech(getDamageTech(p->getBaseTorpedoTech(), p->getDamage()));
                }
            }
        }
    }

    // FIXME: ShipInfo and PermutedSorter as well as the following functions
    // are almost copied verbatim from pdk-host.cc!
    // FIXME: merge with regular packing?

    /** Get damage-restricted tech level for a base. */
    int getBaseDamageTech(const Planet& pl, int have_tech)
    {
        return std::min(int((100 - pl.getBaseDamage()) / 10),
                        int(have_tech));
    }

    /** Compute number of beams on a planet. */
    int getNumPlanetBeams(const Planet& pl, const HostConfiguration& config)
    {
        // ex getPlanetBeamCount
        int defense = pl.getDefense();
        if (pl.hasBase()) {
            defense += pl.getBaseDefense();
        }
        defense = util::roundToInt(std::sqrt(defense / 3.0));
        return std::min(defense, int(config[HostConfiguration::AllowAlternativeCombat]() ? game::vcr::flak::FLAK_MAX_BEAMS : 10));
    }

    /** Compute beam type on a planet. */
    int getPlanetBeamType(const Planet& pl)
    {
        // ex getPlanetBeamTech
        int tech = util::roundToInt(std::sqrt(pl.getDefense() / 2.0));
        if (tech > 10) {
            return 10;
        } else if (pl.hasBase() && getBaseDamageTech(pl, pl.getBaseBeamTech()) > tech) {
            return getBaseDamageTech(pl, pl.getBaseBeamTech());
        } else {
            return tech;
        }
    }

    /** Compute number of planetary tubes. */
    int getNumPlanetLaunchers(const Planet& pl, const HostConfiguration& config)
    {
        // ex getPlanetTubeCount
        if (!config[HostConfiguration::PlanetsHaveTubes]()) {
            return 0;
        }
        int defense = pl.getDefense();
        if (pl.hasBase()) {
            defense += pl.getBaseDefense();
        }
        defense = util::roundToInt(std::sqrt(defense / 4.0));
        return std::min(defense, int(game::vcr::flak::FLAK_MAX_TORPS));
    }

    /** Compute torpedo type of a planet. */
    int getPlanetTorpedoType(const Planet& pl, const ShipList& shipList)
    {
        // ex getPlanetTorpType
        int tech = util::roundToInt(std::sqrt(pl.getDefense() / 2.0));
        if (tech > shipList.launchers().size()) {
            tech = shipList.launchers().size();
        }
        if (pl.hasBase() && getBaseDamageTech(pl, pl.getBaseTorpedoTech()) > tech) {
            return getBaseDamageTech(pl, pl.getBaseTorpedoTech());
        } else {
            return tech;
        }
    }

    /** Compute number of torpedoes on a planet. */
    int getNumPlanetTorpedoes(const Planet& pl, const ShipList& shipList, const HostConfiguration& config)
    {
        // ex getPlanetTorpCount
        int torps = getNumPlanetLaunchers(pl, config) * config[HostConfiguration::PlanetaryTorpsPerTube](pl.getOwner());
        if (pl.hasBase() && config[HostConfiguration::UseBaseTorpsInCombat](pl.getOwner())) {
            torps += pl.getNumBaseTorpedoesAsType(getPlanetTorpedoType(pl, shipList), shipList);
        }
        return torps;
    }

    /** Compute number of fighter bays on a planet. */
    int getNumPlanetBays(const Planet& pl)
    {
        // ex getPlanetBayCount
        int bays = util::roundToInt(std::sqrt((double) pl.getDefense()));
        if (pl.hasBase()) {
            bays += 5;
        }
        return bays;
    }

    /** Compute number of fighters on a planet. */
    int getNumPlanetFighters(const Planet& pl)
    {
        // ex getPlanetFighterCount
        int fighters = util::roundToInt(std::sqrt((double) pl.getDefense()));
        if (pl.hasBase()) {
            fighters += pl.getNumBaseFighters();
        }
        return fighters;
    }

    /** Compute combat mass of a planet. */
    int getPlanetCombatMass(const Planet& pl)
    {
        int mass = 100 + pl.getDefense();
        if (pl.hasBase()) {
            mass += pl.getBaseDefense();
        }
        return mass;
    }


    struct ShipInfo {
        /** FCBO value, plus 100. */
        int fcbo_plus_100;
        /** Ship data. */
        game::vcr::flak::Object data;
        /** Link to original unit. */
        const game::sim::Object* orig;

        ShipInfo& initFromShip(const Ship& sh, const ShipList& shipList,
                               const HostConfiguration& config,
                               const game::vcr::flak::Configuration& flakConfig);
        ShipInfo& initFromPlanet(const Planet& pl, const ShipList& shipList,
                                 const HostConfiguration& config,
                                 const game::vcr::flak::Configuration& flakConfig);

        /** True iff this is a planet. */
        bool  isPlanet() const
            { return data.isPlanet(); }
    };


    /** Initialize data for a ship. */
    ShipInfo& ShipInfo::initFromShip(const Ship& sh, const ShipList& shipList,
                                     const HostConfiguration& config,
                                     const game::vcr::flak::Configuration& flakConfig)
    {
        orig = &sh;

        const int level = sh.getExperienceLevel();

        data.setName(sh.getName());
        data.setDamage(sh.getDamage());
        data.setCrew(sh.getCrew());
        data.setId(sh.getId());
        data.setOwner(sh.getOwner());
        data.setHull(sh.getHullType());
        data.setExperienceLevel(sh.getExperienceLevel());
        data.setNumBeams(sh.getNumBeams());
        data.setBeamType(sh.getBeamType());
        data.setNumLaunchers(sh.getNumLaunchers());
        data.setNumTorpedoes(sh.getNumLaunchers() != 0 ? sh.getAmmo() : 0);
        data.setTorpedoType(sh.getTorpedoType());
        data.setNumBays(sh.getNumBays());
        data.setNumFighters(sh.getNumLaunchers() == 0 ? 0 : sh.getAmmo());
        data.setMass(sh.getMass());
        data.setShield(sh.getShield());

        /* NTP */
        if (sh.getFriendlyCode() == "NTP") {
            data.setNumFighters(0);
            data.setNumTorpedoes(0);
        }

        /* ESB */
        int esb = 0;
        if (config[HostConfiguration::AllowEngineShieldBonus]() != 0) {
            esb += config[HostConfiguration::EngineShieldBonusRate](data.getOwner());
        }
        if (level != 0) {
            esb += config[HostConfiguration::EModEngineShieldBonusRate](level);
        }
        if (esb) {
            if (const game::spec::Engine* e = shipList.engines().get(sh.getEngineType())) {
                data.addMass(e->cost().get(Cost::Money) * esb / 100);
            }
        }

        /* Fed crew bonus */
        if (config[HostConfiguration::AllowFedCombatBonus]() && config.getPlayerRaceNumber(data.getOwner()) == 1) {
            data.addMass(50);
        }

        /* extra bays */
        if (data.getNumBays() != 0) {
            data.addBays(config[HostConfiguration::ExtraFighterBays](data.getOwner()));
            data.addBays(config.getExperienceBonus(HostConfiguration::EModExtraFighterBays, level));
            if (data.getNumBays() > game::vcr::flak::FLAK_MAX_BAYS) {
                data.setNumBays(game::vcr::flak::FLAK_MAX_BAYS);
            }
        }

        data.setIsPlanet(false);

        fcbo_plus_100 = std::min(getFCodeValuePHost(sh) + 100, 1099);

        data.init(flakConfig);

        // Rating overrides
        if ((sh.getFlags() & Ship::fl_RatingOverride) != 0) {
            data.setRating(sh.getFlakRatingOverride());
            data.setCompensation(sh.getFlakCompensationOverride());
        }
        return *this;
    }

    /** Initialize data from planet. */
    ShipInfo& ShipInfo::initFromPlanet(const Planet& pl,
                                       const ShipList& shipList,
                                       const HostConfiguration& config,
                                       const game::vcr::flak::Configuration& flakConfig)
    {
        orig = &pl;

        const int level = pl.getExperienceLevel();

        data.setName(pl.getName());
        data.setDamage(0); /* planet starts with 0 damage in every turn */
        data.setCrew(0);
        data.setId(pl.getId());
        data.setOwner(pl.getOwner());
        data.setHull(0);
        data.setExperienceLevel(level);
        data.setNumBeams(getNumPlanetBeams(pl, config));
        data.setBeamType(getPlanetBeamType(pl));
        data.setNumLaunchers(getNumPlanetLaunchers(pl, config));
        data.setNumTorpedoes(getNumPlanetTorpedoes(pl, shipList, config));
        data.setTorpedoType(getPlanetTorpedoType(pl, shipList));
        data.setNumBays(getNumPlanetBays(pl));
        data.setNumFighters(getNumPlanetFighters(pl));
        data.setMass(getPlanetCombatMass(pl));
        data.setShield(100); /* planet starts with 100 shield in every turn */

        // Extra bays
        if (data.getNumBays() != 0) {
            if (level != 0) {
                data.addBays(config[HostConfiguration::EModExtraFighterBays](level));
            }
            if (data.getNumBays() > game::vcr::flak::FLAK_MAX_BAYS) {
                data.setNumBays(game::vcr::flak::FLAK_MAX_BAYS);
            }
        }

        data.setIsPlanet(true);
        fcbo_plus_100 = std::min(getFCodeValuePHost(pl) + 100, 1099);

        data.init(flakConfig);

        return *this;
    }

    bool isArmed(const game::vcr::Object& obj)
    {
        // ex FlakShip::isArmed
        return obj.getNumBeams() != 0
            || (obj.getNumLaunchers() != 0 && obj.getNumTorpedoes() != 0)
            || (obj.getNumBays() != 0 && obj.getNumFighters() != 0);
    }

    /** Check whether any ship from me can attack any ship from them. */
    bool canAttackThisFleet(const game::vcr::flak::Setup& battle, const game::vcr::flak::Setup::Fleet& me, const game::vcr::flak::Setup::Fleet& them,
                            const std::vector<ShipInfo>& info,
                            const Configuration& opts,
                            const ShipList& shipList, const HostConfiguration& config)
    {
        // shortcut
        if (&me == &them) {
            return false;
        }

        // check it
        for (size_t my_index = 0; my_index < me.numShips; ++my_index) {
            for (size_t their_index = 0; their_index < them.numShips; ++their_index) {
                const game::vcr::flak::Object& my_ship = battle.getShipByIndex(me.firstShipIndex + my_index);
                const game::vcr::flak::Object& their_ship = battle.getShipByIndex(them.firstShipIndex + their_index);
                if (isAttackingAny(*info[me.firstShipIndex + my_index].orig, *info[them.firstShipIndex + their_index].orig, opts, shipList, config)) {
                    if (isArmed(my_ship) || isArmed(their_ship)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /** Compute attack list for one fleet. */
    void computeAttackList(game::vcr::flak::Setup& battle, game::vcr::flak::Setup::Fleet& fleet, const std::vector<ShipInfo>& info, const Configuration& opts, const ShipList& shipList, const HostConfiguration& config,
                           const game::vcr::flak::Configuration& flakConfig,
                           util::RandomNumberGenerator& rng)
    {
        /* Fleet/fleet attack relations must be symmetrical. If any ship
           from a fleet can attack/be attacked by us, we must be allowed
           to attack all ships from that fleet. Otherwise, it could happen
           that the other fleet is still aggressive to us, but we do no
           longer have someone to attack. */
        /* FIXME: this code is really ugly. And it is almost a duplicate
           of the same thing in pdk-host.cc */
        for (size_t other_fleet_nr = 0; other_fleet_nr < battle.getNumFleets(); ++other_fleet_nr) {
            const game::vcr::flak::Setup::Fleet& other_fleet = battle.getFleetByIndex(other_fleet_nr);
            if (canAttackThisFleet(battle, fleet, other_fleet, info, opts, shipList, config)) {
                for (size_t other_index = 0; other_index < other_fleet.numShips; ++other_index) {
                    const size_t other_member = other_index + other_fleet.firstShipIndex;
                    const game::vcr::flak::Object& them = battle.getShipByIndex(other_member);
                    bool can_attack = false, match_pe = false, match_fc = false;
                    // for each fleet member
                    for (size_t fleet_index = 0; fleet_index < fleet.numShips; ++fleet_index) {
                        const size_t fleet_member = fleet_index + fleet.firstShipIndex;
                        const game::vcr::flak::Object& me = battle.getShipByIndex(fleet_member);
                        if (isAttackingAny(*info[fleet_member].orig, *info[other_member].orig, opts, shipList, config)) {
                            if (isArmed(me) || isArmed(them)) {
                                can_attack = true;
                                if (const Ship* sh = dynamic_cast<const Ship*>(info[fleet_member].orig)) {
                                    if (sh->getAggressiveness() == them.getOwner()) {
                                        match_pe = true;
                                    }
                                }
                            }
                        } else {
                            match_fc = true;
                        }
                    }
                    if (can_attack) {
                        // we can attack it regularily
                        int bonus = rng(static_cast<uint16_t>(flakConfig.RatingRandomBonus));
                        if (match_pe) {
                            bonus += flakConfig.RatingPEBonus;
                        }
                        if (!match_fc) {
                            bonus += flakConfig.RatingFullAttackBonus;
                        }
                        if (!bonus) {
                            bonus = 1;
                        }
                        battle.addAttackListEntry(other_member, static_cast<int16_t>(bonus));
                    } else {
                        // we cannot attack it, so give it priority 0
                        battle.addAttackListEntry(other_member, 0);
                    }
                }
            }
        }
    }


    /** Predicate for sort: sort a list of ShipInfo into their combat
        order. Groups players together, randomly permuted. Upon
        construction, this computes a permutation and uses it later
        on. */
    class PermutedSorter {
        int player_map[game::MAX_PLAYERS];
        int frob(int pid)
            {
                if (pid > 0 && pid <= game::MAX_PLAYERS) {
                    return player_map[pid-1];
                }
                return pid;
            }
     public:
        PermutedSorter(util::RandomNumberGenerator& rng);
        bool operator()(const ShipInfo& l, const ShipInfo& r)
            {
                int dif = frob(l.data.getOwner()) - frob(r.data.getOwner());
                if (dif) {
                    return dif < 0;
                }
                dif = l.fcbo_plus_100 - r.fcbo_plus_100;
                if (dif) {
                    return dif < 0;
                }
                dif = l.data.getId() - r.data.getId();
                if (dif) {
                    return dif < 0;
                }
                return !r.isPlanet();
            }
    };

    /** Constructor. Generates a permutation. */
    PermutedSorter::PermutedSorter(util::RandomNumberGenerator& rng)
    {
        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            player_map[i-1] = i;
        }
        for (int i = 1; i < game::MAX_PLAYERS; ++i) {
            int j = rng(static_cast<int16_t>(i+1));
            std::swap(player_map[i], player_map[j]);
        }
    }

    /** Update simulation from FLAK ship.
        \param fsh [in] Result of FLAK fight for a ship
        \param ssh [in/out] Ship from simulation setup to update */
    void unpackFlakShip(const game::vcr::flak::Object& fsh, Ship& ssh)
    {
        ssh.setShield(fsh.getShield());
        ssh.setDamage(fsh.getDamage());
        ssh.setCrew(fsh.getCrew());
        if (fsh.getNumLaunchers() != 0) {
            ssh.setAmmo(fsh.getNumTorpedoes());
        } else {
            ssh.setAmmo(fsh.getNumFighters());
        }
        if (fsh.getEndingStatus() < 0) {
            // died
            ssh.setAggressiveness(Ship::agg_Passive);
            ssh.setOwner(0);
        } else if (fsh.getEndingStatus() != 0 && fsh.getEndingStatus() != fsh.getOwner()) {
            // captured
            ssh.setOwner(fsh.getEndingStatus());
            ssh.setCrew(10);
            ssh.setAggressiveness(0);
        }
    }

    /** Update simulation from FLAK planet.
        \param [in]     fsh       Result of FLAK fight for a planet
        \param [in]     oldObj    Original planet
        \param [in/out] spl       Planet from simulation setup to update
        \param [in]     shipList  Ship list
        \param [in]     config    Host configuration */
    void unpackFlakPlanet(const game::vcr::flak::Object& fsh, const game::vcr::flak::Object& oldObj, Planet& spl, const ShipList& shipList, const game::config::HostConfiguration& config)
    {
        spl.setDamage(fsh.getDamage());
        spl.setShield(fsh.getShield());
        if (spl.hasBase() && spl.getDamage() >= 100) {
            spl.setBaseBeamTech(0); /* remove the base */
        }

        if (spl.hasBase()) {
            // Base fighters
            int fighters_lost = oldObj.getNumFighters() - fsh.getNumFighters();
            int new_sbf = spl.getNumBaseFighters() - fighters_lost;
            if (new_sbf < 0) {
                spl.setNumBaseFighters(0);
            } else {
                spl.setNumBaseFighters(new_sbf);
            }

            // Reduce tech
            int max_tech = (100 - spl.getDamage()) / 10;
            if (max_tech <= 0) {
                max_tech = 1;
            }
            if (spl.getBaseBeamTech() > max_tech) {
                spl.setBaseBeamTech(max_tech);
            }
            if (spl.getBaseTorpedoTech() > max_tech) {
                spl.setBaseTorpedoTech(max_tech);
            }
        }

        int torps_lost = oldObj.getNumTorpedoes() - fsh.getNumTorpedoes();
        if (torps_lost > 0 && spl.hasBase() && config[HostConfiguration::PlanetsHaveTubes]() && config[HostConfiguration::UseBaseTorpsInCombat](spl.getOwner())) {
            int32_t total_cost = torps_lost;
            if (const game::spec::TorpedoLauncher* tl = shipList.launchers().get(fsh.getTorpedoType())) {
                total_cost *= tl->torpedoCost().get(Cost::Money);
            }
            while (total_cost > 0) {
                bool did = 0;
                for (int i = 1; i <= shipList.launchers().size(); ++i) {
                    const game::spec::TorpedoLauncher* tl = shipList.launchers().get(i);
                    if (spl.getNumBaseTorpedoes(i) > 0 && tl != 0 && tl->torpedoCost().get(Cost::Money) <= total_cost) {
                        spl.setNumBaseTorpedoes(i, spl.getNumBaseTorpedoes(i) - 1);
                        total_cost -= tl->torpedoCost().get(Cost::Money);
                        did = true;
                    }
                }
                if (!did) {
                    total_cost = 0;
                }
            }
        }

        if (fsh.getEndingStatus() != 0 && fsh.getEndingStatus() != fsh.getOwner()) {
            if (fsh.getEndingStatus() < 0) {
                spl.setOwner(0);
            } else {
                spl.setOwner(fsh.getEndingStatus());
            }
            // spl.setAggressiveness(spl.agg_Passive);
            spl.setDefense(0);
            spl.setBaseBeamTech(0);
            spl.setFriendlyCode("\?\?\?");
            spl.setShield(0);
        }
    }

    void simulateFLAK(Setup& setup,
                      const Configuration& opts,
                      Result& result,
                      afl::base::Memory<Statistic> stats,
                      const ShipList& shipList,
                      const HostConfiguration& config,
                      const game::vcr::flak::Configuration& flakConfig,
                      util::RandomNumberGenerator& rng)
    {
        afl::base::Ptr<game::vcr::flak::Database> db(new game::vcr::flak::Database());
        result.battles = db;

        // Build list of ships
        std::vector<ShipInfo> ships;
        for (size_t i = 0, n = setup.getNumShips(); i < n; ++i) {
            ships.push_back(ShipInfo().initFromShip(*setup.getShip(i), shipList, config, flakConfig));
        }
        if (const Planet* pl = setup.getPlanet()) {
            ships.push_back(ShipInfo().initFromPlanet(*pl, shipList, config, flakConfig));
        }

        if (ships.size() < 2) {
            return;
        }

        // Group by owner, using a random permutation of owners
        std::sort(ships.begin(), ships.end(), PermutedSorter(rng));

        // Count players
        game::PlayerSet_t players;
        int player_count = 0;
        for (size_t i = 0; i < ships.size(); ++i) {
            if (!players.contains(ships[i].data.getOwner())) {
                players |= ships[i].data.getOwner();
                ++player_count;
            }
        }

        if (player_count < 2) {
            return;
        }

        // Now build fleets.
        std::auto_ptr<game::vcr::flak::Setup> flakSetup(new game::vcr::flak::Setup());
        int  cur_player = 0;
        int  cur_fcbo   = 0;
        bool cur_planet = false;
        int  cur_fleet_size = 0;
        for (size_t i = 0; i < ships.size(); ++i) {
            if (i == 0 || cur_player != ships[i].data.getOwner()
                || cur_fcbo != ships[i].fcbo_plus_100 / 100
                || cur_planet != ships[i].isPlanet()
                || cur_fleet_size >= flakConfig.MaximumFleetSize)
            {
                flakSetup->addFleet(ships[i].data.getOwner());
                cur_fcbo       = ships[i].fcbo_plus_100 / 100;
                cur_planet     = ships[i].isPlanet();
                cur_player     = ships[i].data.getOwner();
                cur_fleet_size = 0;
            }
            flakSetup->addShip(ships[i].data);
            ++cur_fleet_size;
        }

        // Now we have all the fleets, compute attack lists
        for (size_t i = 0; i < flakSetup->getNumFleets(); ++i) {
            flakSetup->startAttackList(i);
            computeAttackList(*flakSetup, flakSetup->getFleetByIndex(i), ships, opts, shipList, config, flakConfig, rng);
            flakSetup->endAttackList(i);
        }

        // Compute speeds, etc.
        game::vcr::flak::GameEnvironment env(config, shipList.beams(), shipList.launchers());
        flakSetup->initAfterSetup(flakConfig, env, rng);
        if (flakSetup->getNumFleets() == 0) {
            return;
        }

        // Set random seed */
        flakSetup->setSeed(rng());

        // Run it...
        game::vcr::flak::NullVisualizer vis;
        game::vcr::flak::Algorithm algo(vis, *flakSetup, env);
        algo.init(env);
        while (algo.playCycle(env))
            ;

        // Evaluate
        // Setting the ending status is host-side logic, not algorithm logic.
        algo.setEndingStatus(*flakSetup, env, rng);

        // Note that initAfterSetup() may have removed ships from the fight,
        // so we can no longer assume a 1:1 mapping between setup and flakSetup
        // (but setup<=>ships and flakSetup<=>algo).
        // Therefore iterate through the ships in the flakSetup and update the appropriate setup ships.
        // @change PCC2 did this the other way around.
        for (size_t i = 0; i < flakSetup->getNumShips(); ++i) {
            // Get old and new object
            const game::vcr::flak::Object& oldObj = flakSetup->getShipByIndex(i);
            game::vcr::flak::Object newObj(oldObj);
            algo.copyResult(i, newObj);

            // Write back to sim setup
            if (oldObj.isPlanet()) {
                if (Planet* pl = setup.getPlanet()) {
                    unpackFlakPlanet(newObj, oldObj, *pl, shipList, config);
                }
                if (Statistic* st = stats.at(setup.getNumShips())) {
                    st->merge(algo.getStatistic(i));
                }
            } else {
                Setup::Slot_t slot = 0;
                if (setup.findShipSlotById(oldObj.getId(), slot)) {
                    if (Ship* sh = setup.getShip(slot)) {
                        unpackFlakShip(newObj, *sh);
                    }
                    if (Statistic* st = stats.at(slot)) {
                        st->merge(algo.getStatistic(i));
                    }
                }
            }
        }

        // Add battle to VCR DB
        db->addNewBattle(new game::vcr::flak::Battle(flakSetup));
    }
}

/*
 *  Main Entry Point
 */

// Run one simulation.
void
game::sim::runSimulation(Setup& setup,
                         std::vector<game::vcr::Statistic>& stats,
                         Result& result,
                         const Configuration& opts,
                         const game::spec::ShipList& list,
                         const game::config::HostConfiguration& config,
                         const game::vcr::flak::Configuration& flakConfig,
                         util::RandomNumberGenerator& rng)
{
    // runSimulation(GSimState& state, const GSimOptions& opts, GSimBattleResult& result, ProgressMonitor& monitor)
    if (opts.hasRandomizeFCodesOnEveryFight()) {
        setup.setRandomFriendlyCodes(rng);
    }

    initializeStats(stats, setup);

    switch (opts.getMode()) {
     case Configuration::VcrHost:
        simulateHost(setup, opts, result, stats, list, config, rng, game::vcr::classic::Host);
        break;
     case Configuration::VcrNuHost:
        simulateHost(setup, opts, result, stats, list, config, rng, game::vcr::classic::NuHost);
        break;
     case Configuration::VcrPHost2:
        simulatePHost(setup, opts, result, stats, list, config, rng, game::vcr::classic::PHost2);
        break;
     case Configuration::VcrPHost3:
        simulatePHost(setup, opts, result, stats, list, config, rng, game::vcr::classic::PHost3);
        break;
     case Configuration::VcrPHost4:
        simulatePHost(setup, opts, result, stats, list, config, rng, game::vcr::classic::PHost4);
        break;
     case Configuration::VcrFLAK:
        simulateFLAK(setup, opts, result, stats, list, config, flakConfig, rng);
        break;
    }
}

void
game::sim::prepareSimulation(Setup& setup,
                             const Configuration& opts,
                             util::RandomNumberGenerator& rng)
{
    if (!opts.hasRandomizeFCodesOnEveryFight()) {
        setup.setRandomFriendlyCodes(rng);
    }
}
