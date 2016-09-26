/**
  *  \file game/spec/nullcomponentnameprovider.hpp
  */
#ifndef C2NG_GAME_SPEC_NULLCOMPONENTNAMEPROVIDER_HPP
#define C2NG_GAME_SPEC_NULLCOMPONENTNAMEPROVIDER_HPP

#include "game/spec/componentnameprovider.hpp"

namespace game { namespace spec {

    class NullComponentNameProvider : public ComponentNameProvider {
     public:
        virtual String_t getName(Type type, int index, const String_t& name) const;
        virtual String_t getShortName(Type type, int index, const String_t& name, const String_t& shortName) const;
    };

} }

#endif
