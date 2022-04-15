/**
  *  \file u/t_game_map_info_scriptlinkbuilder.cpp
  *  \brief Test for game::map::info::ScriptLinkBuilder
  */

#include "game/map/info/scriptlinkbuilder.hpp"

#include "t_game_map_info.hpp"

using game::SearchQuery;

/** Test makePlanetLink() with a specimen. */
void
TestGameMapInfoScriptLinkBuilder::testPlanet()
{
    game::map::info::ScriptLinkBuilder t;
    game::map::Planet p(42);
    TS_ASSERT_EQUALS(t.makePlanetLink(p), "q:UI.GotoScreen 2,42");
}

/** Test makeSearchLink() with some specimen. */
void
TestGameMapInfoScriptLinkBuilder::testSearch()
{
    game::map::info::ScriptLinkBuilder t;
    TS_ASSERT_EQUALS(t.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,     SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets), "Name='x'")), "q:UI.Search \"Name='x'\",\"p2\"");
    TS_ASSERT_EQUALS(t.makeSearchLink(SearchQuery(SearchQuery::MatchFalse,    SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets), "Name='x'")), "q:UI.Search \"Name='x'\",\"p3\"");
    TS_ASSERT_EQUALS(t.makeSearchLink(SearchQuery(SearchQuery::MatchName,     SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets), "yy")), "q:UI.Search \"yy\",\"p1\"");
    TS_ASSERT_EQUALS(t.makeSearchLink(SearchQuery(SearchQuery::MatchLocation, SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets), "333,444")), "q:UI.Search \"333,444\",\"p4\"");

    TS_ASSERT_EQUALS(t.makeSearchLink(SearchQuery(SearchQuery::MatchTrue,
                                                  SearchQuery::SearchObjects_t() + SearchQuery::SearchShips + SearchQuery::SearchOthers,
                                                  "Name='x'")),
                     "q:UI.Search \"Name='x'\",\"so2\"");
}

