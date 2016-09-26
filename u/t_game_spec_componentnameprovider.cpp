/**
  *  \file u/t_game_spec_componentnameprovider.cpp
  *  \brief Test for game::spec::ComponentNameProvider
  */

#include "game/spec/componentnameprovider.hpp"

#include "t_game_spec.hpp"

/** Interface test. */
void
TestGameSpecComponentNameProvider::testIt()
{
    class Tester : public game::spec::ComponentNameProvider {
     public:
        virtual String_t getName(Type /*type*/, int /*index*/, const String_t& /*name*/) const
            { return String_t(); }
        virtual String_t getShortName(Type /*type*/, int /*index*/, const String_t& /*name*/, const String_t& /*shortName*/) const
            { return String_t(); }
    };
    Tester t;
}

