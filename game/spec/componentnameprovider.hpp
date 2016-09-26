/**
  *  \file game/spec/componentnameprovider.hpp
  */
#ifndef C2NG_GAME_SPEC_COMPONENTNAMEPROVIDER_HPP
#define C2NG_GAME_SPEC_COMPONENTNAMEPROVIDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace game { namespace spec {

    class ComponentNameProvider : public afl::base::Deletable {
     public:
        enum Type {
            Hull,
            Engine,
            Beam,
            Torpedo
        };

        virtual String_t getName(Type type, int index, const String_t& name) const = 0;
        virtual String_t getShortName(Type type, int index, const String_t& name, const String_t& shortName) const = 0;
    };

} }

#endif
