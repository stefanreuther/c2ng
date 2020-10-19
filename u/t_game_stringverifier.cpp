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
        virtual bool isValidString(Context /*ctx*/, const String_t& /*text*/) const
            { return false; }
        virtual bool isValidCharacter(Context /*ctx*/, afl::charset::Unichar_t /*ch*/) const
            { return false; }
        virtual size_t getMaxStringLength(Context /*ctx*/) const
            { return 0; }
        virtual Tester* clone() const
            { return new Tester(); }
    };
    Tester t;
}

/** Test defaultIsValidString(). */
void
TestGameStringVerifier::testDefaultIsValidString()
{
    class Tester : public game::StringVerifier {
     public:
        virtual bool isValidString(Context ctx, const String_t& text) const
            { return defaultIsValidString(ctx, text); }
        virtual bool isValidCharacter(Context /*ctx*/, afl::charset::Unichar_t ch) const
            { return ch != 'a'; }
        virtual size_t getMaxStringLength(Context ctx) const
            { return ctx == FriendlyCode ? 3 : 10; }
        virtual Tester* clone() const
            { return new Tester(); }
    };
    Tester t;

    TS_ASSERT_EQUALS(t.isValidString(Tester::FriendlyCode, "pqrs"), false);   // too long
    TS_ASSERT_EQUALS(t.isValidString(Tester::FriendlyCode, "abc"),  false);   // invalid character
    TS_ASSERT_EQUALS(t.isValidString(Tester::FriendlyCode, "mno"),  true);
    TS_ASSERT_EQUALS(t.isValidString(Tester::ShipName,     "pqrs"), true);
}

