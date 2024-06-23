/**
  *  \file test/game/vcr/overviewtest.cpp
  *  \brief Test for game::vcr::Overview
  */

#include "game/vcr/overview.hpp"

#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/database.hpp"
#include <algorithm>

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
AFL_TEST("game.vcr.Overview:buildDiagram", a)
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
    db.addNewBattle(new game::vcr::classic::Battle(makeFreighter(110, 1), makeCaptor(120, 2), 1, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 130 captures 140 (second group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(130, 2), makeFreighter(140, 1), 1, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 150 captures 160 (third group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(150, 2), makeFreighter(160, 1), 1, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 170 captures 180 (fourth group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(170, 4), makeFreighter(180, 1), 1, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 120 captures 190 (join first group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(120, 2), makeFreighter(190, 7), 1, 0))
        ->setType(game::vcr::classic::Host, 0);
    // 130 captures 180 (joins second and fourth group)
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(130, 2), makeFreighter(180, 1), 1, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Testee
    game::vcr::Overview ov(db, config, shipList);

    game::vcr::Overview::Diagram diag;
    ov.buildDiagram(diag, players, tx);

    // Verify
    a.checkEqual("01. size", diag.units.size(), 9U);
    // --- Units ---
    // First group
    a.checkEqual("02. name", diag.units[0].name, "F110 (ship #110)");
    a.checkEqual("03. name", diag.units[1].name, "C120 (ship #120)");
    a.checkEqual("04. name", diag.units[2].name, "F190 (ship #190)");

    // Second+Fourth group
    a.checkEqual("11. name", diag.units[3].name, "C130 (ship #130)");
    a.checkEqual("12. name", diag.units[4].name, "F140 (ship #140)");
    a.checkEqual("13. name", diag.units[5].name, "C170 (ship #170)");
    a.checkEqual("14. name", diag.units[6].name, "F180 (ship #180)");

    // Third group
    a.checkEqual("21. name", diag.units[7].name, "C150 (ship #150)");
    a.checkEqual("22. name", diag.units[8].name, "F160 (ship #160)");

    a.checkEqual("31. initialOwner", diag.units[0].initialOwner, 1);
    a.checkEqual("32. initialOwner", diag.units[1].initialOwner, 2);
    a.checkEqual("33. initialOwner", diag.units[2].initialOwner, 7);
    a.checkEqual("34. initialOwner", diag.units[3].initialOwner, 2);
    a.checkEqual("35. initialOwner", diag.units[4].initialOwner, 1);
    a.checkEqual("36. initialOwner", diag.units[5].initialOwner, 4);
    a.checkEqual("37. initialOwner", diag.units[6].initialOwner, 1);
    a.checkEqual("38. initialOwner", diag.units[7].initialOwner, 2);
    a.checkEqual("39. initialOwner", diag.units[8].initialOwner, 1);

    // --- Battles ---
    a.checkEqual("41. size", diag.battles.size(), 6U);
    a.checkEqual("42. name", diag.battles[0].name, "F110 vs. C120");
    a.checkEqual("43. name", diag.battles[1].name, "C130 vs. F140");
    a.checkEqual("44. name", diag.battles[2].name, "C150 vs. F160");
    a.checkEqual("45. name", diag.battles[3].name, "C170 vs. F180");
    a.checkEqual("46. name", diag.battles[4].name, "C120 vs. F190");
    a.checkEqual("47. name", diag.battles[5].name, "C130 vs. F180");

    a.checkEqual("51. status", diag.battles[0].status, 2);
    a.checkEqual("52. status", diag.battles[1].status, 2);
    a.checkEqual("53. status", diag.battles[2].status, 2);
    a.checkEqual("54. status", diag.battles[3].status, 4);
    a.checkEqual("55. status", diag.battles[4].status, 2);
    a.checkEqual("56. status", diag.battles[5].status, 2);

    a.checkEqual("61. participants", toString(diag.battles[0].participants), "0:2 1:0");
    a.checkEqual("62. participants", toString(diag.battles[1].participants), "3:0 4:2");
    a.checkEqual("63. participants", toString(diag.battles[2].participants), "7:0 8:2");
    a.checkEqual("64. participants", toString(diag.battles[3].participants), "5:0 6:4");
    a.checkEqual("65. participants", toString(diag.battles[4].participants), "1:0 2:2");
    a.checkEqual("66. participants", toString(diag.battles[5].participants), "3:0 6:2");
}

/** Test diagram building, kill.
    Exercises handling of a killed ship; this is not tested in testDiagram().
    A: set up a fight where a ship is killed (captor vs probe).
    E: verify correct diagram being built */
AFL_TEST("game.vcr.Overview:buildDiagram:kill", a)
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
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(30, 5), makeProbe(31, 6), 1, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Testee
    game::vcr::Overview ov(db, config, shipList);

    game::vcr::Overview::Diagram diag;
    ov.buildDiagram(diag, players, tx);

    // Verify
    a.checkEqual("01. size", diag.units.size(), 2U);
    // --- Units ---
    a.checkEqual("02. name", diag.units[0].name, "C30 (ship #30)");
    a.checkEqual("03. name", diag.units[1].name, "L31 (ship #31)");
    a.checkEqual("04. initialOwner", diag.units[0].initialOwner, 5);
    a.checkEqual("05. initialOwner", diag.units[1].initialOwner, 6);

    // --- Battles ---
    a.checkEqual("11. size",         diag.battles.size(), 1U);
    a.checkEqual("12. name",         diag.battles[0].name, "C30 vs. L31");
    a.checkEqual("13. status",       diag.battles[0].status, -1);
    a.checkEqual("14. participants", toString(diag.battles[0].participants), "0:0 1:-1");
}

/** Test diagram building, stalemate.
    Exercises handling of a stalemate; this is not tested in testDiagram().
    A: set up a fight with a stalemate (freighter vs freighter).
    E: verify correct diagram being built */
AFL_TEST("game.vcr.Overview:buildDiagram:stalemate", a)
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
    db.addNewBattle(new game::vcr::classic::Battle(makeFreighter(41, 5), makeFreighter(40, 6), 1, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Testee
    game::vcr::Overview ov(db, config, shipList);

    game::vcr::Overview::Diagram diag;
    ov.buildDiagram(diag, players, tx);

    // Verify
    a.checkEqual("01. size", diag.units.size(), 2U);
    // --- Units ---
    a.checkEqual("02. name", diag.units[0].name, "F41 (ship #41)");
    a.checkEqual("03. name", diag.units[1].name, "F40 (ship #40)");
    a.checkEqual("04. initialOwner", diag.units[0].initialOwner, 5);
    a.checkEqual("05. initialOwner", diag.units[1].initialOwner, 6);

    // --- Battles ---
    a.checkEqual("11. size",         diag.battles.size(), 1U);
    a.checkEqual("12. name",         diag.battles[0].name, "F41 vs. F40");
    a.checkEqual("13. status",       diag.battles[0].status, 0);
    a.checkEqual("14. participants", toString(diag.battles[0].participants), "0:0 1:0");
}

/** Test score summary building, kill.
    A: set up a fight where a ship is killed (captor vs probe).
    E: verify correct results being built */
AFL_TEST("game.vcr.Overview:buildScoreSummary:kill", a)
{
    // Environment
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    // Database
    game::vcr::classic::Database db;
    // 30 destroys 31
    db.addNewBattle(new game::vcr::classic::Battle(makeCaptor(30, 5), makeProbe(31, 6), 1, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Testee
    game::vcr::Overview ov(db, config, shipList);

    game::vcr::Overview::ScoreSummary sum;
    ov.buildScoreSummary(sum);

    // Verify
    a.checkEqual("01. players", sum.players.toInteger(), (1U << 5) | (1U << 6));

    a.checkEqual("11. getBuildMillipoints", sum.scores.at(5)->getBuildMillipoints().min(), 1000); /* Host: 1 PBP */
    a.checkEqual("12. getBuildMillipoints", sum.scores.at(5)->getBuildMillipoints().max(), 1000);
    a.checkEqual("13. getExperience",       sum.scores.at(5)->getExperience().min(), 0);
    a.checkEqual("14. getExperience",       sum.scores.at(5)->getExperience().max(), 0);
    a.checkEqual("15. getTonsDestroyed",    sum.scores.at(5)->getTonsDestroyed().min(), 30);
    a.checkEqual("16. getTonsDestroyed",    sum.scores.at(5)->getTonsDestroyed().max(), 30);

    a.checkEqual("21. getBuildMillipoints", sum.scores.at(6)->getBuildMillipoints().min(), 0);
    a.checkEqual("22. getBuildMillipoints", sum.scores.at(6)->getBuildMillipoints().max(), 0);
    a.checkEqual("23. getExperience",       sum.scores.at(6)->getExperience().min(), 0);
    a.checkEqual("24. getExperience",       sum.scores.at(6)->getExperience().max(), 0);
    a.checkEqual("25. getTonsDestroyed",    sum.scores.at(6)->getTonsDestroyed().min(), 0);
    a.checkEqual("26. getTonsDestroyed",    sum.scores.at(6)->getTonsDestroyed().max(), 0);

    a.checkEqual("31. numBattles", sum.numBattles, 1U);
}
