/**
  *  \file game/map/info/scriptlinkbuilder.cpp
  *  \class game::map::info::ScriptLinkBuilder
  */

#include "game/map/info/scriptlinkbuilder.hpp"
#include "afl/string/format.hpp"
#include "interpreter/values.hpp"      // quoteString

using afl::string::Format;
using game::SearchQuery;

namespace {
    String_t toString(SearchQuery::MatchType ty)
    {
        switch (ty) {
         case SearchQuery::MatchName:     return "1";
         case SearchQuery::MatchTrue:     return "2";
         case SearchQuery::MatchFalse:    return "3";
         case SearchQuery::MatchLocation: return "4";
        }
        return String_t();
    }
}

String_t
game::map::info::ScriptLinkBuilder::makePlanetLink(const Planet& pl) const
{
    return Format("q:UI.GotoScreen 2,%d", pl.getId());
}

String_t
game::map::info::ScriptLinkBuilder::makeSearchLink(const SearchQuery& q) const
{
    return Format("q:UI.Search %s,%s",
                  interpreter::quoteString(q.getQuery()),
                  interpreter::quoteString(q.getSearchObjectsAsString() + toString(q.getMatchType())));
}
