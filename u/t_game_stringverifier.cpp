/**
  *  \file u/t_game_stringverifier.cpp
  *  \brief Test for game::StringVerifier
  */

#include "game/stringverifier.hpp"

#include "t_game.hpp"

/** Interface test. */
void
TestGameStringVerifier::testIt()
{
    class Tester : public game::StringVerifier {
     public:
        virtual bool isValidString(Context /*ctx*/, const String_t& /*text*/)
            { return false; }
        virtual bool isValidCharacter(Context /*ctx*/, afl::charset::Unichar_t /*ch*/)
            { return false; }
        virtual size_t getMaxStringLength(Context /*ctx*/)
            { return 0; }
        virtual Tester* clone() const
            { return new Tester(); }
    };
    Tester t;
}

