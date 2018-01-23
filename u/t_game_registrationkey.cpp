/**
  *  \file u/t_game_registrationkey.cpp
  *  \brief Test for game::RegistrationKey
  */

#include "game/registrationkey.hpp"

#include "t_game.hpp"

/** Interface test. */
void
TestGameRegistrationKey::testIt()
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

