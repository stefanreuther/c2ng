/**
  *  \file u/t_game_map_info_info.cpp
  *  \brief Test for game::map::info::Info
  *
  *  Large parts of info.cpp are covered by the test for game::map::info::Browser.
  *  This module tests some missing details.
  */

#include "game/map/info/info.hpp"

#include "t_game_map_info.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "game/turn.hpp"
#include "game/teamsettings.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/xml/writer.hpp"

using afl::io::xml::TagNode;
using afl::string::NullTranslator;
using game::TeamSettings;
using game::Turn;
using game::UnitScoreDefinitionList;
using game::config::HostConfiguration;
using game::map::Point;
using game::map::Universe;
using util::NumberFormatter;

namespace gmi = game::map::info;

namespace {
    const int PLAYER = 3;

    /*
     *  Test Harness for renderStarchartEmpireSummary
     */

    struct StarchartTestHarness {
        Turn turn;
        TeamSettings teams;
        util::NumberFormatter fmt;
        afl::string::NullTranslator tx;

        StarchartTestHarness()
            : turn(), teams(), fmt(true, true), tx()
            { teams.setViewpointPlayer(PLAYER); }
    };

    void addPlanet(StarchartTestHarness& h, game::Id_t id, int x, int y)
    {
        game::map::Planet* pl = h.turn.universe().planets().create(id);
        pl->setPosition(Point(x, y));
        pl->setOwner(PLAYER);
        pl->setPlayability(game::map::Object::Playable);
    }


    /*
     *  Test Harness for renderShipExperienceSummary
     */

    struct ExperienceTestHarness {
        Universe univ;
        UnitScoreDefinitionList shipScores;
        UnitScoreDefinitionList::Index_t expIndex;
        HostConfiguration config;

        util::NumberFormatter fmt;
        afl::string::NullTranslator tx;

        ExperienceTestHarness()
            : univ(), shipScores(), expIndex(), config(), fmt(true, true), tx()
            {
                UnitScoreDefinitionList::Definition defn = { "Experience", game::ScoreId_ExpLevel, 10 };
                expIndex = shipScores.add(defn);

                config[HostConfiguration::NumExperienceLevels].set(3);
                config[HostConfiguration::ExperienceLevelNames].set("Noob,Apprentice,Wizard,God");
            }
    };

    void addShip(ExperienceTestHarness& h, game::Id_t id, int16_t level)
    {
        game::map::Ship* sh = h.univ.ships().create(id);
        sh->setOwner(PLAYER);
        sh->setPlayability(game::map::Object::Playable);
        sh->unitScores().set(h.expIndex, level, 10);
    }


    /*
     *  General
     */

    template<typename T>
    String_t toString(const T& n)
    {
        afl::io::InternalSink sink;
        afl::io::xml::Writer(sink).visit(n);
        return afl::string::fromBytes(sink.getContent());
    }
}

/** Test size reporting, empty empire.
    A: prepare universe with no planet at all
    E: map extent not shown at all */
void
TestGameMapInfoInfo::testEmpireZeroSize()
{
    StarchartTestHarness h;
    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.fmt, h.tx);

    TS_ASSERT_EQUALS(toString(tab),
                     "<table><tr><td width=\"18\"><font color=\"white\">Your Empire</font></td><td width=\"22\"/></tr>"
                     "<tr><td>Planets:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Starships:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Total Planets:</td><td><font color=\"green\">0</font></td></tr></table>");
}

/** Test size reporting, single planet.
    A: prepare universe with one planet.
    E: map extent shown as "Location" */
void
TestGameMapInfoInfo::testEmpireZeroUnit()
{
    StarchartTestHarness h;
    addPlanet(h, 100, 2300, 2400);

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.fmt, h.tx);

    TS_ASSERT_EQUALS(toString(tab),
                     "<table><tr><td width=\"18\"><font color=\"white\">Your Empire</font></td><td width=\"22\"/></tr>"
                     "<tr><td>Planets:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>\xC2\xA0 East-West Location:</td><td><font color=\"green\">at 2300</font></td></tr>"
                     "<tr><td>\xC2\xA0 North-South Location:</td><td><font color=\"green\">at 2400</font></td></tr>"
                     "<tr><td>Starships:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Total Planets:</td><td><font color=\"green\">0</font></td></tr></table>");
}

/** Test size reporting, same X.
    A: prepare universe with two planets at same X.
    E: map extent shown as "Location" for X, "Range" for Y */
void
TestGameMapInfoInfo::testEmpireZeroSameX()
{
    StarchartTestHarness h;
    addPlanet(h, 100, 2300, 2400);
    addPlanet(h, 200, 2300, 2500);

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.fmt, h.tx);

    TS_ASSERT_EQUALS(toString(tab),
                     "<table><tr><td width=\"18\"><font color=\"white\">Your Empire</font></td><td width=\"22\"/></tr>"
                     "<tr><td>Planets:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>\xC2\xA0 East-West Location:</td><td><font color=\"green\">at 2300</font></td></tr>"
                     "<tr><td>\xC2\xA0 North-South Range:</td><td><font color=\"green\">101 ly from 2400 to 2500</font></td></tr>"
                     "<tr><td>Starships:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Total Planets:</td><td><font color=\"green\">0</font></td></tr></table>");
}

/** Test size reporting, general case.
    A: prepare universe with two planets at different coordinates.
    E: map extent shown as "Range" for both */
void
TestGameMapInfoInfo::testEmpireDifferent()
{
    StarchartTestHarness h;
    addPlanet(h, 100, 1100, 2800);
    addPlanet(h, 200, 2900, 1200);

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.fmt, h.tx);

    TS_ASSERT_EQUALS(toString(tab),
                     "<table><tr><td width=\"18\"><font color=\"white\">Your Empire</font></td><td width=\"22\"/></tr>"
                     "<tr><td>Planets:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>\xC2\xA0 East-West Range:</td><td><font color=\"green\">1801 ly from 1100 to 2900</font></td></tr>"
                     "<tr><td>\xC2\xA0 North-South Range:</td><td><font color=\"green\">1601 ly from 1200 to 2800</font></td></tr>"
                     "<tr><td>Starships:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Total Planets:</td><td><font color=\"green\">0</font></td></tr></table>");
}

/** Test size reporting, wraparound case.
    A: prepare universe with two planets at corners.
    E: map extent shown as "Range", using wrap */
void
TestGameMapInfoInfo::testEmpireWrap()
{
    StarchartTestHarness h;
    addPlanet(h, 100, 1100, 2800);
    addPlanet(h, 200, 2900, 1200);
    h.turn.universe().config().setConfiguration(game::map::Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.fmt, h.tx);

    TS_ASSERT_EQUALS(toString(tab),
                     "<table><tr><td width=\"18\"><font color=\"white\">Your Empire</font></td><td width=\"22\"/></tr>"
                     "<tr><td>Planets:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>\xC2\xA0 East-West Range:</td><td><font color=\"green\">201 ly from 2900 to 1100</font></td></tr>"
                     "<tr><td>\xC2\xA0 North-South Range:</td><td><font color=\"green\">401 ly from 2800 to 1200</font></td></tr>"
                     "<tr><td>Starships:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Total Planets:</td><td><font color=\"green\">0</font></td></tr></table>");
}

/** Test size reporting, wraparound case.
    A: prepare universe with many planets next to each other from left to right.
    E: map extent shown as "Range", using wrap for Y but not for X */
void
TestGameMapInfoInfo::testEmpireWrap2()
{
    StarchartTestHarness h;
    addPlanet(h, 100, 1100, 2800);
    addPlanet(h, 200, 2900, 1200);
    for (int i = 1; i < 36; ++i) {
        addPlanet(h, 100+i, 1100 + 50*i, 2700);
    }
    h.turn.universe().config().setConfiguration(game::map::Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.fmt, h.tx);

    TS_ASSERT_EQUALS(toString(tab),
                     "<table><tr><td width=\"18\"><font color=\"white\">Your Empire</font></td><td width=\"22\"/></tr>"
                     "<tr><td>Planets:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>\xC2\xA0 East-West Range:</td><td><font color=\"green\">1801 ly from 1100 to 2900</font></td></tr>"
                     "<tr><td>\xC2\xA0 North-South Range:</td><td><font color=\"green\">501 ly from 2700 to 1200</font></td></tr>"
                     "<tr><td>Starships:</td><td><font color=\"green\">0</font></td></tr>"
                     "<tr><td>Total Planets:</td><td><font color=\"green\">0</font></td></tr></table>");
}

/** Test experience reporting.
    A: prepare universe with a couple of ships.
    E: ship counts correctly reported */
void
TestGameMapInfoInfo::testExperience()
{
    ExperienceTestHarness h;

    // 5 noobs, no apprentice, 2 wizards, 7 gods
    for (int i = 1; i <= 5; ++i) {
        addShip(h, i, 0);
    }
    for (int i = 1; i <= 2; ++i) {
        addShip(h, 10 + i, 2);
    }
    for (int i = 1; i <= 7; ++i) {
        addShip(h, 20 + i, 3);
    }

    TagNode tab("table");
    gmi::renderShipExperienceSummary(tab, h.univ, true, h.shipScores, h.config, h.fmt, h.tx);

    TS_ASSERT_EQUALS(toString(tab),
                     "<table><tr><td width=\"17\"><font color=\"white\">Ships by Experience Level</font></td><td align=\"right\" width=\"3\"/></tr>"
                     "<tr><td><a href=\"q:UI.Search \'Level=0 And Owner$=My.Race$\', \'2s\'\">Noob</a></td><td align=\"right\"><font color=\"green\">5</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search \'Level=2 And Owner$=My.Race$\', \'2s\'\">Wizard</a></td><td align=\"right\"><font color=\"green\">2</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search \'Level=3 And Owner$=My.Race$\', \'2s\'\">God</a></td><td align=\"right\"><font color=\"green\">7</font></td></tr></table>");
}
