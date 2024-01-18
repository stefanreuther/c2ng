/**
  *  \file test/game/spec/costtest.cpp
  *  \brief Test for game::spec::Cost
  */

#include "game/spec/cost.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "util/numberformatter.hpp"

/** Tests various cases of fromString.

    This does not test invalid cases, as those are not yet defined. As of 20081216,
    invalid characters are ignored by GCost::fromString, and treated as if not present.
    Possible cases that need definition:
    - invalid cargo letters
    - overflow
    - signs

    Like PCC 1.x, we accept cargospecs and PHost format:
    - Cargospec:  123TDM
    - PHost:      T123 D123 M123 */

// Zero-initalisation
AFL_TEST("game.spec.Cost:init", a)
{
    game::spec::Cost value;
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 0);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("05. Money",         value.get(value.Money), 0);
    a.checkEqual("06. toPHostString", value.toPHostString(), "S0");
    a.check("07. isZero",             value.isZero());
}

// Blank cargospec
AFL_TEST("game.spec.Cost:fromString:blank", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 0);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("05. Money",         value.get(value.Money), 0);
    a.checkEqual("06. toPHostString", value.toPHostString(), "S0");
    a.check("07. isZero",             value.isZero());
}

// Zero cargospec
AFL_TEST("game.spec.Cost:fromString:zero", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("0td");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 0);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("05. Money",         value.get(value.Money), 0);
    a.checkEqual("06. toPHostString", value.toPHostString(), "S0");
    a.check("07. isZero",             value.isZero());
}

// Standard cargospec (torpedo cost)
AFL_TEST("game.spec.Cost:fromString:normal", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("1tdm 20$");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 1);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 1);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 1);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("05. Money",         value.get(value.Money), 20);
    a.checkEqual("06. toPHostString", value.toPHostString(), "T1 D1 M1 $20");
    a.check("07. isZero",            !value.isZero());
}

// Standard cargospec without space
AFL_TEST("game.spec.Cost:fromString:run-together", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("1tdm42$");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 1);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 1);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 1);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("05. Money",         value.get(value.Money), 42);
    a.checkEqual("06. toPHostString", value.toPHostString(), "T1 D1 M1 $42");
    a.check("07. isZero",            !value.isZero());
}

// Standard cargospec with duplication
AFL_TEST("game.spec.Cost:fromString:duplicate", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("1ttttdm");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 4);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 1);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 1);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("05. Money",         value.get(value.Money), 0);
    a.checkEqual("06. toPHostString", value.toPHostString(), "T4 D1 M1");
    a.check("07. isZero",            !value.isZero());
}

// Standard cargospec with addition
AFL_TEST("game.spec.Cost:fromString:add", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("10s 20s");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 0);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 30);
    a.checkEqual("05. Money",         value.get(value.Money), 0);
    a.checkEqual("06. toPHostString", value.toPHostString(), "S30");
    a.check("07. isZero",            !value.isZero());
}

// Standard cargospec, uppercase
AFL_TEST("game.spec.Cost:fromString:upper-case", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("10TDM 99S");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 10);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 10);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 10);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 99);
    a.checkEqual("05. Money",         value.get(value.Money), 0);
    a.checkEqual("06. toPHostString", value.toPHostString(), "T10 D10 M10 S99");
    a.check("07. isZero",            !value.isZero());
}

// PHost-style
AFL_TEST("game.spec.Cost:fromString:phost-style", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("T10 D20 M30 $77 S42");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 10);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 20);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 30);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 42);
    a.checkEqual("05. Money",         value.get(value.Money), 77);
    a.checkEqual("06. toPHostString", value.toPHostString(), "T10 D20 M30 S42 $77");
    a.check("07. isZero",            !value.isZero());
}

// PHost-style, lower-case
AFL_TEST("game.spec.Cost:fromString:phost-style:lower-case", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("t11 d22 m33 $44 S55");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 11);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 22);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 33);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 55);
    a.checkEqual("05. Money",         value.get(value.Money), 44);
    a.checkEqual("06. toPHostString", value.toPHostString(), "T11 D22 M33 S55 $44");
    a.check("07. isZero",            !value.isZero());
}

// PHost-style, with addition
AFL_TEST("game.spec.Cost:fromString:phost-style:add", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("t11 t22 t33");
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 66);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("05. Money",         value.get(value.Money), 0);
    a.checkEqual("06. toPHostString", value.toPHostString(), "T66");
}

// game::spec::Cost parses using CargoSpec::parse(..., true), so check "max" syntax
AFL_TEST("game.spec.Cost:fromString:max", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("tmax");
    a.checkEqual("01. Tritanium",  value.get(value.Tritanium), 10000);
    a.checkEqual("02. Duranium",   value.get(value.Duranium), 0);
    a.checkEqual("03. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("05. Money",      value.get(value.Money), 0);
    a.check("06. isZero",         !value.isZero());
}

AFL_TEST("game.spec.Cost:fromString:max:abbr", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("tm");
    a.checkEqual("01. Tritanium",  value.get(value.Tritanium), 10000);
    a.checkEqual("02. Duranium",   value.get(value.Duranium), 0);
    a.checkEqual("03. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("05. Money",      value.get(value.Money), 0);
    a.check("06. isZero",         !value.isZero());
}

AFL_TEST("game.spec.Cost:fromString:max+other", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("tmax d10");
    a.checkEqual("01. Tritanium",  value.get(value.Tritanium), 10000);
    a.checkEqual("02. Duranium",   value.get(value.Duranium), 10);
    a.checkEqual("03. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("05. Money",      value.get(value.Money), 0);
    a.check("06. isZero",         !value.isZero());
}

AFL_TEST("game.spec.Cost:fromString:max+other:abbr", a)
{
    game::spec::Cost value = game::spec::Cost::fromString("tm d10");
    a.checkEqual("01. Tritanium",  value.get(value.Tritanium), 10000);
    a.checkEqual("02. Duranium",   value.get(value.Duranium), 10);
    a.checkEqual("03. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("05. Money",      value.get(value.Money), 0);
    a.check("06. isZero",         !value.isZero());
}


/*
 *  Addition
 */

AFL_TEST("game.spec.Cost:add:single", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("t1");
    game::spec::Cost cb = game::spec::Cost::fromString("t42");
    ca += cb;
    a.checkEqual("01. Tritanium",  ca.get(ca.Tritanium), 43);
    a.checkEqual("02. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("03. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("04. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("05. Money",      ca.get(ca.Money), 0);
    a.check("06. isNonNegative",   ca.isNonNegative());
    a.check("07. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.spec.Cost:add:mixed", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("t1");
    game::spec::Cost cb = game::spec::Cost::fromString("s42");
    ca += cb;
    a.checkEqual("11. Tritanium",  ca.get(ca.Tritanium), 1);
    a.checkEqual("12. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("13. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("14. Supplies",   ca.get(ca.Supplies), 42);
    a.checkEqual("15. Money",      ca.get(ca.Money), 0);
    a.check("16. isNonNegative",   ca.isNonNegative());
    a.check("17. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.spec.Cost:add:sup+mc", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("s100");
    game::spec::Cost cb = game::spec::Cost::fromString("$200");
    ca += cb;
    a.checkEqual("21. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("22. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("23. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("24. Supplies",   ca.get(ca.Supplies), 100);
    a.checkEqual("25. Money",      ca.get(ca.Money), 200);
    a.check("26. isNonNegative",   ca.isNonNegative());
    a.check("27. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.spec.Cost:add:mc", a)
{
    game::spec::Cost ca;
    game::spec::Cost cb = game::spec::Cost::fromString("$200");
    ca += cb;
    a.checkEqual("31. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("32. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("33. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("34. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("35. Money",      ca.get(ca.Money), 200);
    a.check("36. isNonNegative",   ca.isNonNegative());
    a.check("37. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.spec.Cost:add:mineral", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("$200");
    ca.add(ca.Molybdenum, 20);
    a.checkEqual("41. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("42. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("43. Molybdenum", ca.get(ca.Molybdenum), 20);
    a.checkEqual("44. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("45. Money",      ca.get(ca.Money), 200);
    a.check("46. isNonNegative",   ca.isNonNegative());
}

AFL_TEST("game.spec.Cost:add:supplies", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("$200");
    ca.add(ca.Supplies, 3);
    a.checkEqual("51. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("52. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("53. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("54. Supplies",   ca.get(ca.Supplies), 3);
    a.checkEqual("55. Money",      ca.get(ca.Money), 200);
    a.check("56. isNonNegative",   ca.isNonNegative());
}

/*
 *  Subtraction
 */

AFL_TEST("game.spec.Cost:sub:single", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("t1");
    game::spec::Cost cb = game::spec::Cost::fromString("t42");
    ca -= cb;
    a.checkEqual("01. Tritanium",  ca.get(ca.Tritanium), -41);
    a.checkEqual("02. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("03. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("04. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("05. Money",      ca.get(ca.Money), 0);
    a.check("06. isNonNegative",  !ca.isNonNegative());
    a.check("07. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.spec.Cost:sub:mixed", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("t1");
    game::spec::Cost cb = game::spec::Cost::fromString("s42");
    ca -= cb;
    a.checkEqual("11. Tritanium",  ca.get(ca.Tritanium), 1);
    a.checkEqual("12. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("13. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("14. Supplies",   ca.get(ca.Supplies), -42);
    a.checkEqual("15. Money",      ca.get(ca.Money), 0);
    a.check("16. isNonNegative",  !ca.isNonNegative());
    a.check("17. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.spec.Cost:sub:sup+mc", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("s100");
    game::spec::Cost cb = game::spec::Cost::fromString("$200");
    ca -= cb;
    a.checkEqual("21. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("22. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("23. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("24. Supplies",   ca.get(ca.Supplies), 100);
    a.checkEqual("25. Money",      ca.get(ca.Money), -200);
    a.check("26. isNonNegative",  !ca.isNonNegative());
    a.check("27. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.spec.Cost:sub:money", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("$200");
    game::spec::Cost cb;
    ca -= cb;
    a.checkEqual("31. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("32. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("33. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("34. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("35. Money",      ca.get(ca.Money), 200);
    a.check("36. isNonNegative",   ca.isNonNegative());
    a.check("37. isNonNegative",   cb.isNonNegative());
}

/*
 *  Multiplication
 */

AFL_TEST("game.spec.Cost:mul:zero-by-int", a)
{
    game::spec::Cost ca;
    ca *= 10;
    a.checkEqual("01. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("02. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("03. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("04. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("05. Money",      ca.get(ca.Money), 0);
}

AFL_TEST("game.spec.Cost:mul:by-int", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("3tdm 42$");
    ca *= 10;
    a.checkEqual("11. Tritanium",  ca.get(ca.Tritanium), 30);
    a.checkEqual("12. Duranium",   ca.get(ca.Duranium), 30);
    a.checkEqual("13. Molybdenum", ca.get(ca.Molybdenum), 30);
    a.checkEqual("14. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("15. Money",      ca.get(ca.Money), 420);
}

AFL_TEST("game.spec.Cost:mul:infix", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("3tdm 42$");
    game::spec::Cost cb = ca * 10;
    a.checkEqual("21. Tritanium",  ca.get(ca.Tritanium), 3);
    a.checkEqual("22. Duranium",   ca.get(ca.Duranium), 3);
    a.checkEqual("23. Molybdenum", ca.get(ca.Molybdenum), 3);
    a.checkEqual("24. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("25. Money",      ca.get(ca.Money), 42);
    a.checkEqual("26. Tritanium",  cb.get(cb.Tritanium), 30);
    a.checkEqual("27. Duranium",   cb.get(cb.Duranium), 30);
    a.checkEqual("28. Molybdenum", cb.get(cb.Molybdenum), 30);
    a.checkEqual("29. Supplies",   cb.get(cb.Supplies), 0);
    a.checkEqual("30. Money",      cb.get(cb.Money), 420);
}

AFL_TEST("game.spec.Cost:mult:infix-anon", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("1t 2d 3m 4$ 5s") * 7;
    a.checkEqual("31. Tritanium",  ca.get(ca.Tritanium), 7);
    a.checkEqual("32. Duranium",   ca.get(ca.Duranium), 14);
    a.checkEqual("33. Molybdenum", ca.get(ca.Molybdenum), 21);
    a.checkEqual("34. Supplies",   ca.get(ca.Supplies), 35);
    a.checkEqual("35. Money",      ca.get(ca.Money), 28);
}

/** Test comparisons. */
AFL_TEST("game.spec.Cost:comparison", a)
{
    // ==, !=
    a.check("01", game::spec::Cost::fromString("") == game::spec::Cost());
    a.check("02", game::spec::Cost::fromString("100$") == game::spec::Cost::fromString("$100"));
    a.check("03", game::spec::Cost::fromString("5tdm") == game::spec::Cost::fromString("T5 5M 5d"));
    a.check("04", game::spec::Cost::fromString("5tdm") != game::spec::Cost::fromString("T5 5M 5d 1d"));
    a.check("05", game::spec::Cost::fromString("1t") != game::spec::Cost());
    a.check("06", game::spec::Cost::fromString("1d") != game::spec::Cost());
    a.check("07", game::spec::Cost::fromString("1m") != game::spec::Cost());
    a.check("08", game::spec::Cost::fromString("1$") != game::spec::Cost());
    a.check("09", game::spec::Cost::fromString("1s") != game::spec::Cost());
    a.check("10", game::spec::Cost::fromString("t1") != game::spec::Cost());
    a.check("11", game::spec::Cost::fromString("d1") != game::spec::Cost());
    a.check("12", game::spec::Cost::fromString("m1") != game::spec::Cost());
    a.check("13", game::spec::Cost::fromString("$1") != game::spec::Cost());
    a.check("14", game::spec::Cost::fromString("s1") != game::spec::Cost());
    a.check("15", game::spec::Cost::fromString("s100") != game::spec::Cost::fromString("$100"));
    a.check("16", game::spec::Cost::fromString("$100") != game::spec::Cost::fromString("s100"));
}

/** Test isEnoughFor(). */
AFL_TEST("game.spec.Cost:isEnoughFor", a)
{
    // Equality:
    a.check("01",  game::spec::Cost().isEnoughFor(game::spec::Cost()));
    a.check("02",  game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1t")));
    a.check("03",  game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1d")));
    a.check("04",  game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1m")));
    a.check("05",  game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1s")));
    a.check("06",  game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1$")));

    // Strictly more:
    a.check("11", !game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("2t")));
    a.check("12", !game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("2d")));
    a.check("13", !game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("2m")));
    a.check("14", !game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("2s")));
    a.check("15", !game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("2$")));

    // Element (Non-)Conversions:
    a.check("21",  game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1t")));
    a.check("22", !game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1t")));
    a.check("23", !game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1t")));
    a.check("24", !game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1t")));
    a.check("25", !game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1t")));

    a.check("31", !game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1d")));
    a.check("32",  game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1d")));
    a.check("33", !game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1d")));
    a.check("34", !game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1d")));
    a.check("35", !game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1d")));

    a.check("41", !game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1m")));
    a.check("42", !game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1m")));
    a.check("43",  game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1m")));
    a.check("44", !game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1m")));
    a.check("45", !game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1m")));

    a.check("51", !game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1s")));
    a.check("52", !game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1s")));
    a.check("53", !game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1s")));
    a.check("54",  game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1s")));
    a.check("55", !game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1s")));

    a.check("61", !game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1$")));
    a.check("62", !game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1$")));
    a.check("63", !game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1$")));
    a.check("64",  game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1$")));
    a.check("65",  game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1$")));

    // Combinations including supply sale:
    a.check("71",  game::spec::Cost::fromString("5t 3d 7m 99$").isEnoughFor(game::spec::Cost::fromString("3tdm 42$")));
    a.check("72",  game::spec::Cost::fromString("5t 3d 7m 99s").isEnoughFor(game::spec::Cost::fromString("3tdm 42$")));
    a.check("73", !game::spec::Cost::fromString("5t 3d 7m 99s").isEnoughFor(game::spec::Cost::fromString("4tdm 42$")));
    a.check("74",  game::spec::Cost::fromString("5t 3d 7m 22s 22$").isEnoughFor(game::spec::Cost::fromString("3tdm 42$")));
    a.check("75", !game::spec::Cost::fromString("5t 3d 7m 22s 22$").isEnoughFor(game::spec::Cost::fromString("3tdm 52$")));
}

/** Test getMaxAmount(). */
AFL_TEST("game.spec.Cost:getMaxAmount", a)
{
    using game::spec::Cost;

    // Divide zero by X
    a.checkEqual("01", Cost().getMaxAmount(9999, Cost()), 9999);
    a.checkEqual("02", Cost().getMaxAmount(9999, Cost::fromString("1t")), 0);
    a.checkEqual("03", Cost().getMaxAmount(9999, Cost::fromString("1d")), 0);
    a.checkEqual("04", Cost().getMaxAmount(9999, Cost::fromString("1m")), 0);
    a.checkEqual("05", Cost().getMaxAmount(9999, Cost::fromString("1s")), 0);
    a.checkEqual("06", Cost().getMaxAmount(9999, Cost::fromString("1$")), 0);

    // Divide X by zero
    a.checkEqual("11", Cost::fromString("1t").getMaxAmount(9999, Cost()), 9999);
    a.checkEqual("12", Cost::fromString("1d").getMaxAmount(9999, Cost()), 9999);
    a.checkEqual("13", Cost::fromString("1m").getMaxAmount(9999, Cost()), 9999);
    a.checkEqual("14", Cost::fromString("1s").getMaxAmount(9999, Cost()), 9999);
    a.checkEqual("15", Cost::fromString("1$").getMaxAmount(9999, Cost()), 9999);

    // Actual division
    a.checkEqual("21", Cost::fromString("100t 80d 20m").getMaxAmount(9999, Cost::fromString("1tdm")), 20);
    a.checkEqual("22", Cost::fromString("100t 80d 20m").getMaxAmount(3, Cost::fromString("1tdm")), 3);

    // Division with supply sale
    a.checkEqual("31", Cost::fromString("200s 100$").getMaxAmount(9999, Cost::fromString("1s 2$")), 100);
    a.checkEqual("32", Cost::fromString("200s 100$").getMaxAmount(9999, Cost::fromString("2s 1$")), 100);

    // Negative
    Cost neg;
    neg.set(Cost::Tritanium, -1);
    a.checkEqual("41", neg.getMaxAmount(9999, Cost()), 0);
    a.checkEqual("42", Cost().getMaxAmount(9999, neg), 0);
    a.checkEqual("43", Cost().getMaxAmount(-1, Cost()), 0);
}

/*
 *  Division
 */

AFL_TEST("game.spec.Cost:div:in-place", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("3tdm 42$");
    ca /= 2;
    a.checkEqual("01. Tritanium",  ca.get(ca.Tritanium), 1);
    a.checkEqual("02. Duranium",   ca.get(ca.Duranium), 1);
    a.checkEqual("03. Molybdenum", ca.get(ca.Molybdenum), 1);
    a.checkEqual("04. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("05. Money",      ca.get(ca.Money), 21);
}

AFL_TEST("game.spec.Cost:div:infix", a)
{
    game::spec::Cost ca = game::spec::Cost::fromString("13tdm 42$");
    game::spec::Cost cb = ca / 5;
    a.checkEqual("11. Tritanium",  ca.get(ca.Tritanium), 13);
    a.checkEqual("12. Duranium",   ca.get(ca.Duranium), 13);
    a.checkEqual("13. Molybdenum", ca.get(ca.Molybdenum), 13);
    a.checkEqual("14. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("15. Money",      ca.get(ca.Money), 42);
    a.checkEqual("16. Tritanium",  cb.get(cb.Tritanium), 2);
    a.checkEqual("17. Duranium",   cb.get(cb.Duranium), 2);
    a.checkEqual("18. Molybdenum", cb.get(cb.Molybdenum), 2);
    a.checkEqual("19. Supplies",   cb.get(cb.Supplies), 0);
    a.checkEqual("20. Money",      cb.get(cb.Money), 8);
}

/** Test format(). */
AFL_TEST("game.spec.Cost:format", a)
{
    afl::string::NullTranslator tx;
    util::NumberFormatter fmt(true, false);

    a.checkEqual("01", game::spec::Cost::fromString("3t 4d 5m 6s 7$").format(tx, fmt), "7 mc, 6 sup, 3 T, 4 D, 5 M");
    a.checkEqual("02", game::spec::Cost::fromString("3000t 4000d").format(tx, fmt), "3,000 T, 4,000 D");
    a.checkEqual("03", game::spec::Cost::fromString("100$ 50t 50d 50m").format(tx, fmt), "100 mc, 50 T/D/M");
    a.checkEqual("04", game::spec::Cost::fromString("50$ 50t 50d").format(tx, fmt), "50 mc/T/D");
    a.checkEqual("05", game::spec::Cost::fromString("0$").format(tx, fmt), "-");
}
