/**
  *  \file test/game/map/info/scriptlinkbuildertest.cpp
  *  \brief Test for game::map::info::ScriptLinkBuilder
  */

#include "game/map/info/scriptlinkbuilder.hpp"
#include "afl/test/testrunner.hpp"

using game::SearchQuery;

/** Test makePlanetLink() with a specimen. */
AFL_TEST("game.map.info.ScriptLinkBuilder:makePlanetLink", a)
{
    game::map::info::ScriptLinkBuilder t;
    game::map::Planet p(42);
    a.checkEqual("", t.makePlanetLink(p), "q:UI.GotoScreen 2,42");
}

/** Test makeSearchLink() with some specimen. */
AFL_TEST("game.map.info.ScriptLinkBuilder:makeSearchLink", a)
{
    game::map::info::ScriptLinkBuilder t;
    a.checkEqual("01. MatchTrue",     t.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,     SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets), "Name='x'")), "q:UI.Search \"Name='x'\",\"p2\"");
    a.checkEqual("02. MatchFalse",    t.makeSearchLink(SearchQuery(SearchQuery::MatchFalse,    SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets), "Name='x'")), "q:UI.Search \"Name='x'\",\"p3\"");
    a.checkEqual("03. MatchName",     t.makeSearchLink(SearchQuery(SearchQuery::MatchName,     SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets), "yy")), "q:UI.Search \"yy\",\"p1\"");
    a.checkEqual("04. MatchLocation", t.makeSearchLink(SearchQuery(SearchQuery::MatchLocation, SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets), "333,444")), "q:UI.Search \"333,444\",\"p4\"");

    a.checkEqual("11. Objects", t.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                             SearchQuery::SearchObjects_t() + SearchQuery::SearchShips + SearchQuery::SearchOthers,
                                                             "Name='x'")),
                 "q:UI.Search \"Name='x'\",\"so2\"");
}
