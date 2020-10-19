/**
  *  \file game/spec/nullcomponentnameprovider.hpp
  *  \brief Class game::spec::NullComponentNameProvider
  */
#ifndef C2NG_GAME_SPEC_NULLCOMPONENTNAMEPROVIDER_HPP
#define C2NG_GAME_SPEC_NULLCOMPONENTNAMEPROVIDER_HPP

#include "game/spec/componentnameprovider.hpp"

namespace game { namespace spec {

    /** Null implementation of ComponentNameProvider.
        Returns just the stored names and does not try to manipulate them in any way. */
    class NullComponentNameProvider : public ComponentNameProvider {
     public:
        // ComponentNameProvider:
        virtual String_t getName(Type type, int index, const String_t& name) const;
        virtual String_t getShortName(Type type, int index, const String_t& name, const String_t& shortName) const;
    };

} }

#endif
