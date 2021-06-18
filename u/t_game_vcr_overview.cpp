/**
  *  \file u/t_game_vcr_overview.cpp
  *  \brief Test for game::vcr::Overview
  */

#include <algorithm>
#include "game/vcr/overview.hpp"

#include "t_game_vcr.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/database.hpp"

namespace {
    // A freighter (will be captured)
    game::vcr::Object makeFreighter(int id, int owner)
    {
        game::vcr::Object r;
        r.setMass(200);
        r.setShield(0);
        r.setDamage(0);
        r.setCrew(1);
        r.setId(id);
        r.setOwner(owner);
        r.setName(afl::string::Format("F%d", id));
        return r;
    }

    // A probe (will be destroyed)
    game::vcr::Object makeProbe(int id, int owner)
    {
        game::vcr::Object r;
        r.setMass(30);
        r.setShield(0);
        r.setDamage(0);
        r.setCrew(100);
        r.setId(id);
        r.setOwner(owner);
        r.setName(afl::string::Format("L%d", id));
        return r;
    }

    // A captor (will capture/destroy the other ship)
    game::vcr::Object makeCaptor(int id, int owner)
    {
        game::vcr::Object r;
        r.setMass(400);
        r.setShield(100);
        r.setDamage(0);
        r.setCrew(300);
        r.setId(id);
        r.setOwner(owner);
        r.setNumBeams(5);
        r.setBeamType(9);
        r.setName(afl::string::Format("C%d", id));
        return r;
    }

    struct Sort {
        template<typename T>
        bool operator()(const T& a, const T& b) const
            { return a.slot < b.slot; }
    };

    String_t toString(std::vector<game::vcr::Overview::Diagram::Participant> ps)
    {
        std::sort(ps.begin(), ps.end(), Sort());

        String_t result;
        for (size_t i = 0; i < ps.size(); ++i) {
            if (!result.empty()) {
                result += ' ';
            }
            result += afl::string::Format("%d:%d", ps[i].slot, ps[i].status);
        }
        return result;
    }
}

/** Test diagram building, general case.
    Exercises how groups are built.
    A: set up multiple fights.
    E: verify correct diagram being built */
void
TestGameVcrOverview::testDiagram()
{
    // Environment
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::PlayerList players;
    afl::string::NullTranslator tx;

    // Database
    game::vcr::classic::Database db;
    // 120 captures 110 (first group)
    db.addNewBattle(new game::vcr::classic::Battle(makeFreighter(110, 1), makeCaptor(120, 2), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 130 captures 140 (second group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(130, 2), makeFreighter(140, 1), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 150 captures 160 (third group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(150, 2), makeFreighter(160, 1), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 170 captures 180 (fourth group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(170, 4), makeFreighter(180, 1), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 120 captures 190 (join first group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(120, 2), makeFreighter(190, 7), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 130 captures 180 (joins second and fourth group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(130, 2), makeFreighter(180, 1), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Testee
    game::vcr::Overview ov(db, config, shipList);

    game::vcr::Overview::Diagram diag;
    ov.buildDiagram(diag, players, tx);

    // Verify
    TS_ASSERT_EQUALS(diag.units.size(), 9U);
    // --- Units ---
    // First group
    TS_ASSERT_EQUALS(diag.units[0].name, "F110 (ship #110)");
    TS_ASSERT_EQUALS(diag.units[1].name, "C120 (ship #120)");
    TS_ASSERT_EQUALS(diag.units[2].name, "F190 (ship #190)");

    // Second+Fourth group
    TS_ASSERT_EQUALS(diag.units[3].name, "C130 (ship #130)");
    TS_ASSERT_EQUALS(diag.units[4].name, "F140 (ship #140)");
    TS_ASSERT_EQUALS(diag.units[5].name, "C170 (ship #170)");
    TS_ASSERT_EQUALS(diag.units[6].name, "F180 (ship #180)");

    // Third group
    TS_ASSERT_EQUALS(diag.units[7].name, "C150 (ship #150)");
    TS_ASSERT_EQUALS(diag.units[8].name, "F160 (ship #160)");

    TS_ASSERT_EQUALS(diag.units[0].initialOwner, 1);
    TS_ASSERT_EQUALS(diag.units[1].initialOwner, 2);
    TS_ASSERT_EQUALS(diag.units[2].initialOwner, 7);
    TS_ASSERT_EQUALS(diag.units[3].initialOwner, 2);
    TS_ASSERT_EQUALS(diag.units[4].initialOwner, 1);
    TS_ASSERT_EQUALS(diag.units[5].initialOwner, 4);
    TS_ASSERT_EQUALS(diag.units[6].initialOwner, 1);
    TS_ASSERT_EQUALS(diag.units[7].initialOwner, 2);
    TS_ASSERT_EQUALS(diag.units[8].initialOwner, 1);

    // --- Battles ---
    TS_ASSERT_EQUALS(diag.battles.size(), 6U);
    TS_ASSERT_EQUALS(diag.battles[0].name, "F110 vs. C120");
    TS_ASSERT_EQUALS(diag.battles[1].name, "C130 vs. F140");
    TS_ASSERT_EQUALS(diag.battles[2].name, "C150 vs. F160");
    TS_ASSERT_EQUALS(diag.battles[3].name, "C170 vs. F180");
    TS_ASSERT_EQUALS(diag.battles[4].name, "C120 vs. F190");
    TS_ASSERT_EQUALS(diag.battles[5].name, "C130 vs. F180");

    TS_ASSERT_EQUALS(diag.battles[0].status, 2);
    TS_ASSERT_EQUALS(diag.battles[1].status, 2);
    TS_ASSERT_EQUALS(diag.battles[2].status, 2);
    TS_ASSERT_EQUALS(diag.battles[3].status, 4);
    TS_ASSERT_EQUALS(diag.battles[4].status, 2);
    TS_ASSERT_EQUALS(diag.battles[5].status, 2);

    TS_ASSERT_EQUALS(toString(diag.battles[0].participants), "0:2 1:0");
    TS_ASSERT_EQUALS(toString(diag.battles[1].participants), "3:0 4:2");
    TS_ASSERT_EQUALS(toString(diag.battles[2].participants), "7:0 8:2");
    TS_ASSERT_EQUALS(toString(diag.battles[3].participants), "5:0 6:4");
    TS_ASSERT_EQUALS(toString(diag.battles[4].participants), "1:0 2:2");
    TS_ASSERT_EQUALS(toString(diag.battles[5].participants), "3:0 6:2");
}

/** Test diagram building, kill.
    Exercises handling of a killed ship; this is not tested in testDiagram().
    A: set up a fight where a ship is killed (captor vs probe).
    E: verify correct diagram being built */
void
TestGameVcrOverview::testDiagramKill()
{
    // Environment
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::PlayerList players;
    afl::string::NullTranslator tx;

    // Database
    game::vcr::classic::Database db;
    // 30 destroys 31
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(30, 5), makeProbe(31, 6), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Testee
    game::vcr::Overview ov(db, config, shipList);

    game::vcr::Overview::Diagram diag;
    ov.buildDiagram(diag, players, tx);

    // Verify
    TS_ASSERT_EQUALS(diag.units.size(), 2U);
    // --- Units ---
    TS_ASSERT_EQUALS(diag.units[0].name, "C30 (ship #30)");
    TS_ASSERT_EQUALS(diag.units[1].name, "L31 (ship #31)");
    TS_ASSERT_EQUALS(diag.units[0].initialOwner, 5);
    TS_ASSERT_EQUALS(diag.units[1].initialOwner, 6);

    // --- Battles ---
    TS_ASSERT_EQUALS(diag.battles.size(), 1U);
    TS_ASSERT_EQUALS(diag.battles[0].name, "C30 vs. L31");
    TS_ASSERT_EQUALS(diag.battles[0].status, -1);
    TS_ASSERT_EQUALS(toString(diag.battles[0].participants), "0:0 1:-1");
}

/** Test diagram building, stalemate.
    Exercises handling of a stalemate; this is not tested in testDiagram().
    A: set up a fight with a stalemate (freighter vs freighter).
    E: verify correct diagram being built */
void
TestGameVcrOverview::testDiagramStalemate()
{
    // Environment
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::PlayerList players;
    afl::string::NullTranslator tx;

    // Database
    game::vcr::classic::Database db;
    // Freighter 40 vs 41
    db.addNewBattle(new game::vcr::classic::Battle(makeFreighter(41, 5), makeFreighter(40, 6), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Testee
    game::vcr::Overview ov(db, config, shipList);

    game::vcr::Overview::Diagram diag;
    ov.buildDiagram(diag, players, tx);

    // Verify
    TS_ASSERT_EQUALS(diag.units.size(), 2U);
    // --- Units ---
    TS_ASSERT_EQUALS(diag.units[0].name, "F41 (ship #41)");
    TS_ASSERT_EQUALS(diag.units[1].name, "F40 (ship #40)");
    TS_ASSERT_EQUALS(diag.units[0].initialOwner, 5);
    TS_ASSERT_EQUALS(diag.units[1].initialOwner, 6);

    // --- Battles ---
    TS_ASSERT_EQUALS(diag.battles.size(), 1U);
    TS_ASSERT_EQUALS(diag.battles[0].name, "F41 vs. F40");
    TS_ASSERT_EQUALS(diag.battles[0].status, 0);
    TS_ASSERT_EQUALS(toString(diag.battles[0].participants), "0:0 1:0");
}

/** Test score summary building, kill.
    A: set up a fight where a ship is killed (captor vs probe).
    E: verify correct results being built */
void
TestGameVcrOverview::testPointsKill()
{
    // Environment
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    // Database
    game::vcr::classic::Database db;
    // 30 destroys 31
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(30, 5), makeProbe(31, 6), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Testee
    game::vcr::Overview ov(db, config, shipList);

    game::vcr::Overview::ScoreSummary sum;
    ov.buildScoreSummary(sum);

    // Verify
    TS_ASSERT_EQUALS(sum.players.toInteger(), (1U << 5) | (1U << 6));

    TS_ASSERT_EQUALS(sum.scores.at(5)->getBuildMillipoints().min(), 1000); /* Host: 1 PBP */
    TS_ASSERT_EQUALS(sum.scores.at(5)->getBuildMillipoints().max(), 1000);
    TS_ASSERT_EQUALS(sum.scores.at(5)->getExperience().min(), 0);
    TS_ASSERT_EQUALS(sum.scores.at(5)->getExperience().max(), 0);
    TS_ASSERT_EQUALS(sum.scores.at(5)->getTonsDestroyed().min(), 30);
    TS_ASSERT_EQUALS(sum.scores.at(5)->getTonsDestroyed().max(), 30);

    TS_ASSERT_EQUALS(sum.scores.at(6)->getBuildMillipoints().min(), 0);
    TS_ASSERT_EQUALS(sum.scores.at(6)->getBuildMillipoints().max(), 0);
    TS_ASSERT_EQUALS(sum.scores.at(6)->getExperience().min(), 0);
    TS_ASSERT_EQUALS(sum.scores.at(6)->getExperience().max(), 0);
    TS_ASSERT_EQUALS(sum.scores.at(6)->getTonsDestroyed().min(), 0);
    TS_ASSERT_EQUALS(sum.scores.at(6)->getTonsDestroyed().max(), 0);

    TS_ASSERT_EQUALS(sum.numBattles, 1);
}

