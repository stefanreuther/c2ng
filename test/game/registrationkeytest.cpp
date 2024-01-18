/**
  *  \file test/game/registrationkeytest.cpp
  *  \brief Test for game::RegistrationKey
  */

#include "game/registrationkey.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.RegistrationKey")
{
    class Tester : public game::RegistrationKey {
     public:
        virtual Status getStatus() const
            { return Status(); }
        virtual String_t getLine(Line /*which*/) const
            { return String_t(); }
        virtual bool setLine(Line /*which*/, String_t /*value*/)
            { return false; }
        virtual int getMaxTechLevel(game::TechLevel /*which*/) const
            { return 100; }
    };
    Tester t;
}
