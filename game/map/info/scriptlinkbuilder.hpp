/**
  *  \file game/map/info/scriptlinkbuilder.hpp
  *  \class game::map::info::ScriptLinkBuilder
  */
#ifndef C2NG_GAME_MAP_INFO_SCRIPTLINKBUILDER_HPP
#define C2NG_GAME_MAP_INFO_SCRIPTLINKBUILDER_HPP

#include "game/map/info/linkbuilder.hpp"

namespace game { namespace map { namespace info {

    /** LinkBuilder using script commands.
        For each kind of link, generates the text "q:CMD", where CMD is a script command (UI.Search, UI.GotoScreen). */
    class ScriptLinkBuilder : public LinkBuilder {
     public:
        // LinkBuilder:
        String_t makePlanetLink(const Planet& pl) const;
        String_t makeSearchLink(const SearchQuery& q) const;
    };

} } }

#endif
