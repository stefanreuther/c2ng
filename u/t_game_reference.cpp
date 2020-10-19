/**
  *  \file u/t_game_reference.cpp
  *  \brief Test for game::Reference
  */

#include "game/reference.hpp"

#include "t_game.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/translator.hpp"

void
TestGameReference::testAccessor()
{
    using game::Reference;
    using game::map::Point;

    afl::string::NullTranslator tx;
    Point pt;

    // Default initialized
    {
        Reference testee;
        TS_ASSERT(!testee.isSet());
        TS_ASSERT_EQUALS(testee.getType(), Reference::Null);
        TS_ASSERT_EQUALS(testee.getId(), 0);
        TS_ASSERT_EQUALS(testee.getPos(pt), false);
        TS_ASSERT_EQUALS(testee.toString(tx), "");
        TS_ASSERT_EQUALS(testee == Reference(), true);
        TS_ASSERT_EQUALS(testee == Reference(Reference::Ship, 77), false);
        TS_ASSERT_EQUALS(testee != Reference(), false);
        TS_ASSERT_EQUALS(testee != Reference(Reference::Ship, 77), true);
    }

    // Initialize from type/Id
    {
        Reference testee(Reference::Planet, 12);
        TS_ASSERT(testee.isSet());
        TS_ASSERT_EQUALS(testee.getType(), Reference::Planet);
        TS_ASSERT_EQUALS(testee.getId(), 12);
        TS_ASSERT_EQUALS(testee.getPos(pt), false);
        TS_ASSERT_EQUALS(testee.toString(tx), "Planet #12");
        TS_ASSERT_EQUALS(testee == Reference(), false);
        TS_ASSERT_EQUALS(testee == Reference(Reference::Planet, 77), false);
        TS_ASSERT_EQUALS(testee == Reference(Reference::Planet, 12), true);
        TS_ASSERT_EQUALS(testee != Reference(), true);
        TS_ASSERT_EQUALS(testee != Reference(Reference::Planet, 77), true);
        TS_ASSERT_EQUALS(testee != Reference(Reference::Planet, 12), false);
    }

    // Initialize from point
    {
        Reference testee(Point(1000, 2000));
        TS_ASSERT(testee.isSet());
        TS_ASSERT_EQUALS(testee.getType(), Reference::MapLocation);
        // unspecified -> TS_ASSERT_EQUALS(testee.getId(), 12);
        TS_ASSERT_EQUALS(testee.getPos(pt), true);
        TS_ASSERT_EQUALS(pt, Point(1000, 2000));
        TS_ASSERT_EQUALS(testee.toString(tx), "(1000,2000)");
        TS_ASSERT_EQUALS(testee == Reference(), false);
        TS_ASSERT_EQUALS(testee == Reference(Reference::Planet, 77), false);
        TS_ASSERT_EQUALS(testee == Reference(Reference::Planet, 12), false);
        TS_ASSERT_EQUALS(testee == Reference(pt), true);
        TS_ASSERT_EQUALS(testee == pt, true);
        TS_ASSERT_EQUALS(testee != Reference(), true);
        TS_ASSERT_EQUALS(testee != Reference(Reference::Planet, 77), true);
        TS_ASSERT_EQUALS(testee != Reference(Reference::Planet, 12), true);
        TS_ASSERT_EQUALS(testee != pt, false);
    }
}

void
TestGameReference::testToString()
{
    using game::Reference;
    afl::test::Translator tx("<", ">");

    TS_ASSERT_EQUALS(Reference().toString(tx), "");

    TS_ASSERT_EQUALS(Reference(Reference::Null,       0).toString(tx), "");
    TS_ASSERT_EQUALS(Reference(Reference::Special,    0).toString(tx), "");
    TS_ASSERT_EQUALS(Reference(Reference::Player,     9).toString(tx), "<Player #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Ship,       9).toString(tx), "<Ship #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Planet,     9).toString(tx), "<Planet #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Starbase,   9).toString(tx), "<Starbase #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Storm,      9).toString(tx), "<Ion Storm #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Minefield,  9).toString(tx), "<Minefield #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Ufo,        9).toString(tx), "<Ufo #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Hull,       9).toString(tx), "<Hull #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Engine,     9).toString(tx), "<Engine #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Beam,       9).toString(tx), "<Beam Weapon #9>");
    TS_ASSERT_EQUALS(Reference(Reference::Torpedo,    9).toString(tx), "<Torpedo Type #9>");

    TS_ASSERT_EQUALS(Reference(game::map::Point(1234, 4567)).toString(tx), "<(1234,4567)>");
}

