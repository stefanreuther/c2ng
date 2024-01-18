/**
  *  \file test/game/v3/udata/nameprovidertest.cpp
  *  \brief Test for game::v3::udata::NameProvider
  */

#include "game/v3/udata/nameprovider.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.v3.udata.NameProvider")
{
    class Tester : public game::v3::udata::NameProvider {
     public:
        virtual String_t getName(Type /*type*/, int /*id*/) const
            { return String_t(); }
    };
    Tester t;
}
