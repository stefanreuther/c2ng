/**
  *  \file game/v3/udata/nameprovider.hpp
  *  \brief Interface game::v3::udata::NameProvider
  */
#ifndef C2NG_GAME_V3_UDATA_NAMEPROVIDER_HPP
#define C2NG_GAME_V3_UDATA_NAMEPROVIDER_HPP

#include "afl/string/string.hpp"

namespace game { namespace v3 { namespace udata {

    /** Interface to access names for util.dat formatting (MessageBuilder). */
    class NameProvider {
     public:
        /** Kind of name to access. */
        enum Type {
            HullFunctionName,            ///< Hull function name (%H). @see game::spec::BasicHullFunction::getName.
            HullName,                    ///< Hull name (%h). @see game::spec::Hull::getName.
            NativeGovernmentName,        ///< Native government name (%g). @see game::tables::NativeGovernmentName.
            NativeRaceName,              ///< Native race name (%n). @see game::tables::NativeRaceName.
            PlanetName,                  ///< Planet name (%p). @see game::map::Planet::getName.
            ShortRaceName                ///< Short race name (%r). @see game::Player::ShortName.
        };

        /** Virtual destructor. */
        virtual ~NameProvider()
            { }

        /** Get name.
            @param type Kind of name to access
            @param id   Id read from file
            @return name; empty if parameters not valid */
        virtual String_t getName(Type type, int id) const = 0;
    };

} } }

#endif
