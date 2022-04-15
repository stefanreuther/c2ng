/**
  *  \file game/map/info/linkbuilder.hpp
  *  \brief Interface game::map::info::LinkBuilder
  */
#ifndef C2NG_GAME_MAP_INFO_LINKBUILDER_HPP
#define C2NG_GAME_MAP_INFO_LINKBUILDER_HPP

#include "game/map/planet.hpp"
#include "game/searchquery.hpp"
#include "afl/string/string.hpp"

namespace game { namespace map { namespace info {

    /** Interface to build links. */
    class LinkBuilder {
     public:
        /** Virtual destructor. */
        virtual ~LinkBuilder()
            { }

        /** Make link to a planet.
            @param pl Planet
            @return Link text */
        virtual String_t makePlanetLink(const Planet& pl) const = 0;

        /** Make link to a search result.
            @param q Search query
            @return Link text */
        virtual String_t makeSearchLink(const SearchQuery& q) const = 0;
    };

} } }

#endif
