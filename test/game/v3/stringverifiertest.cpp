/**
  *  \file test/game/v3/stringverifiertest.cpp
  *  \brief Test for game::v3::StringVerifier
  */

#include "game/v3/stringverifier.hpp"

#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    std::auto_ptr<afl::charset::Charset> makeCharset()
    {
        return std::auto_ptr<afl::charset::Charset>(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
    }

    typedef game::StringVerifier SV_t;
}

AFL_TEST("game.v3.StringVerifier:basics", a)
{
    game::v3::StringVerifier testee(makeCharset());
    a.check("01", testee.isValidString(SV_t::FriendlyCode, ""));
    a.check("02", testee.isValidString(SV_t::FriendlyCode, "foo"));
    a.check("03", !testee.isValidString(SV_t::FriendlyCode, "foo1"));

    a.check("11", testee.isValidString(SV_t::ShipName, ""));
    a.check("12", testee.isValidString(SV_t::ShipName, "xxxxxxxxxx"));
    a.check("13", testee.isValidString(SV_t::ShipName, "xxxxxxxxxxyyyyyyyyyy"));
    a.check("14", !testee.isValidString(SV_t::ShipName, "xxxxxxxxxxyyyyyyyyyyz"));

    a.check("21", testee.isValidCharacter(SV_t::ShipName, ' '));
    a.check("22", testee.isValidCharacter(SV_t::ShipName, 0xFF));
    a.check("23", !testee.isValidCharacter(SV_t::ShipName, 0x100));
    a.check("24", !testee.isValidCharacter(SV_t::ShipName, 0x1000));
    a.check("25", !testee.isValidCharacter(SV_t::ShipName, 0x10000));
}

AFL_TEST("game.v3.StringVerifier:friendly-code", a)
{
    game::v3::StringVerifier testee(makeCharset());

    a.check("01", testee.isValidString(SV_t::FriendlyCode, "   "));
    a.check("02", testee.isValidString(SV_t::FriendlyCode, "~~~"));
    a.check("03", !testee.isValidString(SV_t::FriendlyCode, "\xC0\x80"));
    a.check("04", !testee.isValidString(SV_t::FriendlyCode, "\xC2\x80"));
    a.check("05", !testee.isValidString(SV_t::FriendlyCode, "\xE2\x86\x91"));

    a.check("11", testee.isValidCharacter(SV_t::FriendlyCode, ' '));
    a.check("12", testee.isValidCharacter(SV_t::FriendlyCode, 126));
    a.check("13", !testee.isValidCharacter(SV_t::FriendlyCode, 127));
    a.check("14", !testee.isValidCharacter(SV_t::FriendlyCode, 180));
}

AFL_TEST("game.v3.StringVerifier:message", a)
{
    game::v3::StringVerifier testee(makeCharset());

    a.check("01", testee.isValidString(SV_t::Message, "   "));
    a.check("02", testee.isValidString(SV_t::Message, "~~~"));
    a.check("03", testee.isValidString(SV_t::Message, "\xC2\x80"));
    a.check("04", testee.isValidString(SV_t::Message, "\xC3\xB2"));
    a.check("05", !testee.isValidString(SV_t::Message, "\xC3\xB3"));

    a.check("11", testee.isValidCharacter(SV_t::Message, ' '));
    a.check("12", testee.isValidCharacter(SV_t::Message, 126));
    a.check("13", testee.isValidCharacter(SV_t::Message, 127));
    a.check("14", testee.isValidCharacter(SV_t::Message, 180));
    a.check("15", testee.isValidCharacter(SV_t::Message, 242));
    a.check("16", !testee.isValidCharacter(SV_t::Message, 243));
}

AFL_TEST("game.v3.StringVerifier:clone", a)
{
    game::v3::StringVerifier testee(makeCharset());
    std::auto_ptr<SV_t> dup(testee.clone());

    a.checkNonNull("01. clone", dup.get());

    a.checkEqual("11. getMaxStringLength", testee.getMaxStringLength(SV_t::PlayerLongName), 30U);
    a.checkEqual("12. getMaxStringLength", dup->getMaxStringLength(SV_t::PlayerLongName), 30U);

    // ok
    a.check("21. isValidString", testee.isValidString(SV_t::PlayerAdjectiveName, "H\xC3\xB6----------"));
    a.check("22. isValidString", dup->isValidString(SV_t::PlayerAdjectiveName, "H\xC3\xB6----------"));

    // too long
    a.check("31. isValidString", !testee.isValidString(SV_t::PlayerAdjectiveName, "H\xC3\xB6-----------"));
    a.check("32. isValidString", !dup->isValidString(SV_t::PlayerAdjectiveName, "H\xC3\xB6-----------"));

    // wrong character
    a.check("41. isValidString", !testee.isValidString(SV_t::PlayerAdjectiveName, "H\xE2\x86\x91"));
    a.check("42. isValidString", !dup->isValidString(SV_t::PlayerAdjectiveName, "H\xE2\x86\x91"));
}
