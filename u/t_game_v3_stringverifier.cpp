/**
  *  \file u/t_game_v3_stringverifier.cpp
  *  \brief Test for game::v3::StringVerifier
  */

#include "game/v3/stringverifier.hpp"

#include "t_game_v3.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"

namespace {
    std::auto_ptr<afl::charset::Charset> makeCharset()
    {
        return std::auto_ptr<afl::charset::Charset>(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
    }

    typedef game::StringVerifier SV_t;
}

void
TestGameV3StringVerifier::testMain()
{
    game::v3::StringVerifier testee(makeCharset());
    TS_ASSERT(testee.isValidString(SV_t::FriendlyCode, ""));
    TS_ASSERT(testee.isValidString(SV_t::FriendlyCode, "foo"));
    TS_ASSERT(!testee.isValidString(SV_t::FriendlyCode, "foo1"));

    TS_ASSERT(testee.isValidString(SV_t::ShipName, ""));
    TS_ASSERT(testee.isValidString(SV_t::ShipName, "xxxxxxxxxx"));
    TS_ASSERT(testee.isValidString(SV_t::ShipName, "xxxxxxxxxxyyyyyyyyyy"));
    TS_ASSERT(!testee.isValidString(SV_t::ShipName, "xxxxxxxxxxyyyyyyyyyyz"));

    TS_ASSERT(testee.isValidCharacter(SV_t::ShipName, ' '));
    TS_ASSERT(testee.isValidCharacter(SV_t::ShipName, 0xFF));
    TS_ASSERT(!testee.isValidCharacter(SV_t::ShipName, 0x100));
    TS_ASSERT(!testee.isValidCharacter(SV_t::ShipName, 0x1000));
    TS_ASSERT(!testee.isValidCharacter(SV_t::ShipName, 0x10000));
}

void
TestGameV3StringVerifier::testFCode()
{
    game::v3::StringVerifier testee(makeCharset());

    TS_ASSERT(testee.isValidString(SV_t::FriendlyCode, "   "));
    TS_ASSERT(testee.isValidString(SV_t::FriendlyCode, "~~~"));
    TS_ASSERT(!testee.isValidString(SV_t::FriendlyCode, "\xC0\x80"));
    TS_ASSERT(!testee.isValidString(SV_t::FriendlyCode, "\xC2\x80"));
    TS_ASSERT(!testee.isValidString(SV_t::FriendlyCode, "\xE2\x86\x91"));

    TS_ASSERT(testee.isValidCharacter(SV_t::FriendlyCode, ' '));
    TS_ASSERT(testee.isValidCharacter(SV_t::FriendlyCode, 126));
    TS_ASSERT(!testee.isValidCharacter(SV_t::FriendlyCode, 127));
    TS_ASSERT(!testee.isValidCharacter(SV_t::FriendlyCode, 180));
}

void
TestGameV3StringVerifier::testMessage()
{
    game::v3::StringVerifier testee(makeCharset());

    TS_ASSERT(testee.isValidString(SV_t::Message, "   "));
    TS_ASSERT(testee.isValidString(SV_t::Message, "~~~"));
    TS_ASSERT(testee.isValidString(SV_t::Message, "\xC2\x80"));
    TS_ASSERT(testee.isValidString(SV_t::Message, "\xC3\xB2"));
    TS_ASSERT(!testee.isValidString(SV_t::Message, "\xC3\xB3"));

    TS_ASSERT(testee.isValidCharacter(SV_t::Message, ' '));
    TS_ASSERT(testee.isValidCharacter(SV_t::Message, 126));
    TS_ASSERT(testee.isValidCharacter(SV_t::Message, 127));
    TS_ASSERT(testee.isValidCharacter(SV_t::Message, 180));
    TS_ASSERT(testee.isValidCharacter(SV_t::Message, 242));
    TS_ASSERT(!testee.isValidCharacter(SV_t::Message, 243));
}
