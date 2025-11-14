/**
  *  \file test/game/map/info/infotest.cpp
  *  \brief Test for game::map::info::Info
  */

#include "game/map/info/info.hpp"

#include "afl/io/internalsink.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/writer.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/info/nulllinkbuilder.hpp"
#include "game/map/info/scriptlinkbuilder.hpp"
#include "game/teamsettings.hpp"
#include "game/turn.hpp"

using afl::base::Ref;
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
        game::map::Configuration mapConfig;
        TeamSettings teams;
        util::NumberFormatter fmt;
        afl::string::NullTranslator tx;

        StarchartTestHarness()
            : turn(), mapConfig(), teams(), fmt(true, true), tx()
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
        UnitScoreDefinitionList theScores;
        UnitScoreDefinitionList::Index_t expIndex;
        Ref<HostConfiguration> config;

        util::NumberFormatter fmt;
        afl::string::NullTranslator tx;

        ExperienceTestHarness()
            : univ(), theScores(), expIndex(), config(HostConfiguration::create()), fmt(true, true), tx()
            {
                UnitScoreDefinitionList::Definition defn = { "Experience", game::ScoreId_ExpLevel, 10 };
                expIndex = theScores.add(defn);

                (*config)[HostConfiguration::NumExperienceLevels].set(3);
                (*config)[HostConfiguration::ExperienceLevelNames].set("Noob,Apprentice,Wizard,God");
            }
    };

    void addShip(ExperienceTestHarness& h, game::Id_t id, int16_t level)
    {
        game::map::Ship* sh = h.univ.ships().create(id);
        sh->setOwner(PLAYER);
        sh->setPlayability(game::map::Object::Playable);
        sh->unitScores().set(h.expIndex, level, 10);
    }

    void addPlanet(ExperienceTestHarness& h, game::Id_t id, int16_t level)
    {
        game::map::Planet* pl = h.univ.planets().create(id);
        pl->setOwner(PLAYER);
        pl->setPlayability(game::map::Object::Playable);
        pl->unitScores().set(h.expIndex, level, 10);
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
AFL_TEST("game.map.info.Info:renderStarchartEmpireSummary:empty", a)
{
    StarchartTestHarness h;
    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.mapConfig, h.fmt, h.tx);

    a.checkEqual("", toString(tab),
                 "<table><tr><td width=\"18\"><font color=\"white\">Your Empire</font></td><td width=\"22\"/></tr>"
                 "<tr><td>Planets:</td><td><font color=\"green\">0</font></td></tr>"
                 "<tr><td>Starships:</td><td><font color=\"green\">0</font></td></tr>"
                 "<tr><td>Total Planets:</td><td><font color=\"green\">0</font></td></tr></table>");
}

/** Test size reporting, single planet.
    A: prepare universe with one planet.
    E: map extent shown as "Location" */
AFL_TEST("game.map.info.Info:renderStarchartEmpireSummary:unit", a)
{
    StarchartTestHarness h;
    addPlanet(h, 100, 2300, 2400);

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.mapConfig, h.fmt, h.tx);

    a.checkEqual("", toString(tab),
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
AFL_TEST("game.map.info.Info:renderStarchartEmpireSummary:same-x", a)
{
    StarchartTestHarness h;
    addPlanet(h, 100, 2300, 2400);
    addPlanet(h, 200, 2300, 2500);

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.mapConfig, h.fmt, h.tx);

    a.checkEqual("", toString(tab),
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
AFL_TEST("game.map.info.Info:renderStarchartEmpireSummary:general", a)
{
    StarchartTestHarness h;
    addPlanet(h, 100, 1100, 2800);
    addPlanet(h, 200, 2900, 1200);

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.mapConfig, h.fmt, h.tx);

    a.checkEqual("", toString(tab),
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
AFL_TEST("game.map.info.Info:renderStarchartEmpireSummary:wrap", a)
{
    StarchartTestHarness h;
    addPlanet(h, 100, 1100, 2800);
    addPlanet(h, 200, 2900, 1200);
    h.mapConfig.setConfiguration(game::map::Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.mapConfig, h.fmt, h.tx);

    a.checkEqual("", toString(tab),
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
AFL_TEST("game.map.info.Info:renderStarchartEmpireSummary:wrap2", a)
{
    StarchartTestHarness h;
    addPlanet(h, 100, 1100, 2800);
    addPlanet(h, 200, 2900, 1200);
    for (int i = 1; i < 36; ++i) {
        addPlanet(h, 100+i, 1100 + 50*i, 2700);
    }
    h.mapConfig.setConfiguration(game::map::Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    TagNode tab("table");
    gmi::renderStarchartEmpireSummary(tab, gmi::computeStarchartInfo(h.turn, h.teams), h.turn.universe(), h.teams, h.mapConfig, h.fmt, h.tx);

    a.checkEqual("", toString(tab),
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
AFL_TEST("game.map.info.Info:renderShipExperienceSummary", a)
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

    // With ScriptLinkBuilder
    {
        TagNode tab("table");
        gmi::renderShipExperienceSummary(tab, h.univ, true, h.theScores, *h.config, h.fmt, h.tx, game::map::info::ScriptLinkBuilder());

        a.checkEqual("01. ScriptLinkBuilder", toString(tab),
                     "<table><tr><td width=\"17\"><font color=\"white\">Ships by Experience Level</font></td><td align=\"right\" width=\"3\"/></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Level=0 And Played&quot;,&quot;s2&quot;\">Noob</a></td><td align=\"right\"><font color=\"green\">5</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Level=2 And Played&quot;,&quot;s2&quot;\">Wizard</a></td><td align=\"right\"><font color=\"green\">2</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Level=3 And Played&quot;,&quot;s2&quot;\">God</a></td><td align=\"right\"><font color=\"green\">7</font></td></tr></table>");
    }

    // With NullLinkBuilder
    {
        TagNode tab("table");
        gmi::renderShipExperienceSummary(tab, h.univ, true, h.theScores, *h.config, h.fmt, h.tx, game::map::info::NullLinkBuilder());

        a.checkEqual("NullLinkBuilder", toString(tab),
                     "<table><tr><td width=\"17\"><font color=\"white\">Ships by Experience Level</font></td><td align=\"right\" width=\"3\"/></tr>"
                     "<tr><td>Noob</td><td align=\"right\"><font color=\"green\">5</font></td></tr>"
                     "<tr><td>Wizard</td><td align=\"right\"><font color=\"green\">2</font></td></tr>"
                     "<tr><td>God</td><td align=\"right\"><font color=\"green\">7</font></td></tr></table>");
    }
}

AFL_TEST("game.map.info.Info:renderPlanetExperienceSummary", a)
{
    ExperienceTestHarness h;

    // 3 noobs, no apprentice, 4 wizards, 5 gods
    for (int i = 1; i <= 3; ++i) {
        addPlanet(h, i, 0);
    }
    for (int i = 1; i <= 4; ++i) {
        addPlanet(h, 10 + i, 2);
    }
    for (int i = 1; i <= 5; ++i) {
        addPlanet(h, 20 + i, 3);
    }

    // With ScriptLinkBuilder
    {
        TagNode tab("table");
        gmi::renderPlanetExperienceSummary(tab, h.univ, h.theScores, *h.config, h.fmt, h.tx, game::map::info::ScriptLinkBuilder());

        a.checkEqual("01. ScriptLinkBuilder", toString(tab),
                     "<table><tr><td width=\"17\"><font color=\"white\">Planets by Experience Level</font></td><td align=\"right\" width=\"3\"/></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Level=0 And Played&quot;,&quot;p2&quot;\">Noob</a></td><td align=\"right\"><font color=\"green\">3</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Level=2 And Played&quot;,&quot;p2&quot;\">Wizard</a></td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td><a href=\"q:UI.Search &quot;Level=3 And Played&quot;,&quot;p2&quot;\">God</a></td><td align=\"right\"><font color=\"green\">5</font></td></tr></table>");
    }

    // With NullLinkBuilder
    {
        TagNode tab("table");
        gmi::renderPlanetExperienceSummary(tab, h.univ, h.theScores, *h.config, h.fmt, h.tx, game::map::info::NullLinkBuilder());

        a.checkEqual("11. NullLinkBuilder", toString(tab),
                     "<table><tr><td width=\"17\"><font color=\"white\">Planets by Experience Level</font></td><td align=\"right\" width=\"3\"/></tr>"
                     "<tr><td>Noob</td><td align=\"right\"><font color=\"green\">3</font></td></tr>"
                     "<tr><td>Wizard</td><td align=\"right\"><font color=\"green\">4</font></td></tr>"
                     "<tr><td>God</td><td align=\"right\"><font color=\"green\">5</font></td></tr></table>");
    }
}
