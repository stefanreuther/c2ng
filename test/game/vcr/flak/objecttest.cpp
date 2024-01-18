/**
  *  \file test/game/vcr/flak/objecttest.cpp
  *  \brief Test for game::vcr::flak::Object
  */

#include "game/vcr/flak/object.hpp"

#include "afl/base/staticassert.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.vcr.flak.Object:basics", a)
{
    game::vcr::flak::Object testee;

    // Get/Set roundtrip
    testee.setMaxFightersLaunched(7);
    testee.setRating(8);
    testee.setCompensation(9);
    testee.setEndingStatus(10);

    a.checkEqual("01. getMaxFightersLaunched", testee.getMaxFightersLaunched(), 7);
    a.checkEqual("02. getRating",              testee.getRating(), 8);
    a.checkEqual("03. getCompensation",        testee.getCompensation(), 9);
    a.checkEqual("04. getEndingStatus",        testee.getEndingStatus(), 10);
}

AFL_TEST("game.vcr.flak.Object:pack", a)
{
    static const uint8_t data[] = { 'U','S','S',' ','D',0xFC,'l','l',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                    5,0,        // 5 damage
                                    4,1,        // 260 crew
                                    2,2,        // ID 514
                                    7,0,        // player 7
                                    63,0,       // hull 63
                                    1,0,        // level 1
                                    3,0,        // 3 beams
                                    10,0,       // type 10
                                    12,0,       // 12 launchers
                                    2,3,        // 770 torpedoes
                                    2,0,        // type 2
                                    0,0,        // 0 bays
                                    0,0,        // 0 fighters
                                    200,0,      // 200 kt
                                    100,0,      // 100% shield
                                    3,0,        // max 3 fighters launched
                                    77,0,1,0,   // rating 65613
                                    50,0,       // compensation 50
                                    0,0,        // not a planet
                                    255,255 };  // was destroyed
    game::vcr::flak::structures::Ship sh;
    static_assert(sizeof(sh) == sizeof(data), "sizeof Ship");
    std::memcpy(&sh, data, sizeof(sh));
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    // Test loading
    game::vcr::flak::Object                    testee(sh, cs);
    a.checkEqual("01. getName",                testee.getName(), "USS D\xC3\xBCll");
    a.checkEqual("02. getDamage",              testee.getDamage(), 5);
    a.checkEqual("03. getCrew",                testee.getCrew(), 260);
    a.checkEqual("04. getId",                  testee.getId(), 514);
    a.checkEqual("05. getOwner",               testee.getOwner(), 7);
    a.checkEqual("06. getHull",                testee.getHull(), 63);
    a.checkEqual("07. getExperienceLevel",     testee.getExperienceLevel(), 1);
    a.checkEqual("08. getNumBeams",            testee.getNumBeams(), 3);
    a.checkEqual("09. getBeamType",            testee.getBeamType(), 10);
    a.checkEqual("10. getNumLaunchers",        testee.getNumLaunchers(), 12);
    a.checkEqual("11. getNumTorpedoes",        testee.getNumTorpedoes(), 770);
    a.checkEqual("12. getTorpedoType",         testee.getTorpedoType(), 2);
    a.checkEqual("13. getNumBays",             testee.getNumBays(), 0);
    a.checkEqual("14. getNumFighters",         testee.getNumFighters(), 0);
    a.checkEqual("15. getMass",                testee.getMass(), 200);
    a.checkEqual("16. getShield",              testee.getShield(), 100);
    a.checkEqual("17. getMaxFightersLaunched", testee.getMaxFightersLaunched(), 3);
    a.checkEqual("18. getRating",              testee.getRating(), 65613);
    a.checkEqual("19. getCompensation",        testee.getCompensation(), 50);
    a.checkEqual("20. isPlanet",               testee.isPlanet(), false);
    a.checkEqual("21. getEndingStatus",        testee.getEndingStatus(), -1);

    // Test saving
    game::vcr::flak::structures::Ship sh2;
    testee.pack(sh2, cs);
    a.checkEqualContent("31. content", afl::base::ConstBytes_t(afl::base::fromObject(sh)), afl::base::ConstBytes_t(afl::base::fromObject(sh2)));
}
