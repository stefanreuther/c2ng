/**
  *  \file test/game/referencetest.cpp
  *  \brief Test for game::Reference
  */

#include "game/reference.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "afl/test/translator.hpp"

using game::Reference;
using game::map::Point;

// Default initialized
AFL_TEST("game.Reference:basics:default", a)
{
    afl::string::NullTranslator tx;
    Point pt;
    Reference testee;

    a.check     ("01. isSet",      !testee.isSet());
    a.checkEqual("02. getType",     testee.getType(), Reference::Null);
    a.checkEqual("03. getId",       testee.getId(), 0);
    a.checkEqual("04. getPosition", testee.getPosition().get(pt), false);
    a.checkEqual("05. toString",    testee.toString(tx), "");
    a.checkEqual("06. eq",          testee == Reference(), true);
    a.checkEqual("07. eq",          testee == Reference(Reference::Ship, 77), false);
    a.checkEqual("08. ne",          testee != Reference(), false);
    a.checkEqual("09. ne",          testee != Reference(Reference::Ship, 77), true);
    a.check     ("10. orElse",     !testee.orElse(testee).isSet());
    a.check     ("11. orElse",      testee.orElse(Reference(Reference::Ship, 77)) == Reference(Reference::Ship, 77));
}

// Initialize from type/Id
AFL_TEST("game.Reference:basics:object", a)
{
    afl::string::NullTranslator tx;
    Point pt;
    Reference testee(Reference::Planet, 12);

    a.check     ("01. isSet",       testee.isSet());
    a.checkEqual("02. getType",     testee.getType(), Reference::Planet);
    a.checkEqual("03. getId",       testee.getId(), 12);
    a.checkEqual("04. getPosition", testee.getPosition().get(pt), false);
    a.checkEqual("05. toString",    testee.toString(tx), "Planet #12");
    a.checkEqual("06. eq",          testee == Reference(), false);
    a.checkEqual("07. eq",          testee == Reference(Reference::Planet, 77), false);
    a.checkEqual("08. eq",          testee == Reference(Reference::Planet, 12), true);
    a.checkEqual("09. ne",          testee != Reference(), true);
    a.checkEqual("10. ne",          testee != Reference(Reference::Planet, 77), true);
    a.checkEqual("11. ne",          testee != Reference(Reference::Planet, 12), false);
    a.check     ("12. orElse",      testee.orElse(testee).isSet());
    a.check     ("13. orElse",      testee.orElse(Reference(Reference::Ship, 77)) == Reference(Reference::Planet, 12));
}

// Initialize from point
AFL_TEST("game.Reference:basics:location", a)
{
    afl::string::NullTranslator tx;
    Point pt;
    Reference testee(Point(1000, 2000));

    a.check     ("41. isSet",       testee.isSet());
    a.checkEqual("42. getType",     testee.getType(), Reference::MapLocation);
    // unspecified -> a.checkEqual("43. getId", testee.getId(), 12);
    a.checkEqual("44. getPosition", testee.getPosition().get(pt), true);
    a.checkEqual("45. pt",          pt, Point(1000, 2000));
    a.checkEqual("46. toString",    testee.toString(tx), "(1000,2000)");
    a.checkEqual("47. eq",          testee == Reference(), false);
    a.checkEqual("48. eq",          testee == Reference(Reference::Planet, 77), false);
    a.checkEqual("49. eq",          testee == Reference(Reference::Planet, 12), false);
    a.checkEqual("50. eq",          testee == Reference(pt), true);
    a.checkEqual("51. eq",          testee == pt, true);
    a.checkEqual("52. ne",          testee != Reference(), true);
    a.checkEqual("53. ne",          testee != Reference(Reference::Planet, 77), true);
    a.checkEqual("54. ne",          testee != Reference(Reference::Planet, 12), true);
    a.checkEqual("55. ne",          testee != pt, false);
    a.check     ("56. orElse",      testee.orElse(testee).isSet());
    a.check     ("57. orElse",      testee.orElse(Reference(Reference::Ship, 77)) == Reference(Point(1000,2000)));
}

AFL_TEST("game.Reference:toString", a)
{
    using game::Reference;
    afl::test::Translator tx("<", ">");

    a.checkEqual("01", Reference().toString(tx), "");

    a.checkEqual("11", Reference(Reference::Null,       0).toString(tx), "");
    a.checkEqual("12", Reference(Reference::Special,    0).toString(tx), "");
    a.checkEqual("13", Reference(Reference::Player,     9).toString(tx), "<Player #9>");
    a.checkEqual("14", Reference(Reference::Ship,       9).toString(tx), "<Ship #9>");
    a.checkEqual("15", Reference(Reference::Planet,     9).toString(tx), "<Planet #9>");
    a.checkEqual("16", Reference(Reference::Starbase,   9).toString(tx), "<Starbase #9>");
    a.checkEqual("17", Reference(Reference::IonStorm,   9).toString(tx), "<Ion Storm #9>");
    a.checkEqual("18", Reference(Reference::Minefield,  9).toString(tx), "<Minefield #9>");
    a.checkEqual("19", Reference(Reference::Ufo,        9).toString(tx), "<Ufo #9>");
    a.checkEqual("20", Reference(Reference::Hull,       9).toString(tx), "<Hull #9>");
    a.checkEqual("21", Reference(Reference::Engine,     9).toString(tx), "<Engine #9>");
    a.checkEqual("22", Reference(Reference::Beam,       9).toString(tx), "<Beam Weapon #9>");
    a.checkEqual("23", Reference(Reference::Torpedo,    9).toString(tx), "<Torpedo Type #9>");

    a.checkEqual("31", Reference(game::map::Point(1234, 4567)).toString(tx), "<(1234,4567)>");
}
