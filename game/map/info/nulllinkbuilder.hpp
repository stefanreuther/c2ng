/**
  *  \file game/map/info/nulllinkbuilder.hpp
  *  \class game::map::info::NullLinkBuilder
  */
#ifndef C2NG_GAME_MAP_INFO_NULLLINKBUILDER_HPP
#define C2NG_GAME_MAP_INFO_NULLLINKBUILDER_HPP

#include "game/map/info/linkbuilder.hpp"

namespace game { namespace map { namespace info {

    /** Null LinkBuilder.
        Returns an empty value for each call, meaning no links are generated. */
    class NullLinkBuilder : public LinkBuilder {
     public:
        // LinkBuilder:
        String_t makePlanetLink(const Planet& pl) const;
        String_t makeSearchLink(const SearchQuery& q) const;
    };

} } }

#endif
