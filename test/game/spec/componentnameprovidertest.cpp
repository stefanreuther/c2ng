/**
  *  \file test/game/spec/componentnameprovidertest.cpp
  *  \brief Test for game::spec::ComponentNameProvider
  */

#include "game/spec/componentnameprovider.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.spec.ComponentNameProvider")
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
