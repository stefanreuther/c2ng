/**
  *  \file test/game/stringverifiertest.cpp
  *  \brief Test for game::StringVerifier
  */

#include "game/stringverifier.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.StringVerifier:interface")
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
AFL_TEST("game.StringVerifier:defaultIsValidString", a)
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

    a.checkEqual("01", t.isValidString(Tester::FriendlyCode, "pqrs"), false);   // too long
    a.checkEqual("02", t.isValidString(Tester::FriendlyCode, "abc"),  false);   // invalid character
    a.checkEqual("03", t.isValidString(Tester::FriendlyCode, "mno"),  true);
    a.checkEqual("04", t.isValidString(Tester::ShipName,     "pqrs"), true);
}
