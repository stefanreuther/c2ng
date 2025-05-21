/**
  *  \file test/game/cargospectest.cpp
  *  \brief Test for game::CargoSpec
  */

#include "game/cargospec.hpp"
#include "afl/test/testrunner.hpp"

/*
 *  Parsing.
 *
 *  These tests are mostly the same as for Cost.
 */
AFL_TEST("game.CargoSpec:init", a)
{
    game::CargoSpec value;
    a.checkEqual("01. Tritanium",     value.get(value.Tritanium), 0);
    a.checkEqual("02. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("03. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("04. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("05. Money",         value.get(value.Money), 0);
    a.checkEqual("06. toPHostString", value.toPHostString(), "S0");
    a.check("07. isZero",             value.isZero());
}

// Blank cargospec
AFL_TEST("game.CargoSpec:parse:blank", a)
{
    game::CargoSpec value;
    a.check("11. parse",              value.parse("", false));
    a.checkEqual("12. Tritanium",     value.get(value.Tritanium), 0);
    a.checkEqual("13. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("14. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("15. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("16. Money",         value.get(value.Money), 0);
    a.checkEqual("17. toPHostString", value.toPHostString(), "S0");
    a.check("18. isZero",             value.isZero());
}

// Zero cargospec
AFL_TEST("game.CargoSpec:parse:zero", a)
{
    game::CargoSpec value;
    a.check("21. parse",              value.parse("0td", false));
    a.checkEqual("22. Tritanium",     value.get(value.Tritanium), 0);
    a.checkEqual("23. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("24. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("25. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("26. Money",         value.get(value.Money), 0);
    a.checkEqual("27. toPHostString", value.toPHostString(), "S0");
    a.check("28. isZero",             value.isZero());
}

// Standard cargospec (torpedo cost)
AFL_TEST("game.CargoSpec:parse:normal", a)
{
    game::CargoSpec value;
    a.check("31. parse",              value.parse("1tdm 20$", false));
    a.checkEqual("32. Tritanium",     value.get(value.Tritanium), 1);
    a.checkEqual("33. Duranium",      value.get(value.Duranium), 1);
    a.checkEqual("34. Molybdenum",    value.get(value.Molybdenum), 1);
    a.checkEqual("35. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("36. Money",         value.get(value.Money), 20);
    a.checkEqual("37. toPHostString", value.toPHostString(), "T1 D1 M1 $20");
    a.check("38. isZero",            !value.isZero());
}

// Standard cargospec without space
AFL_TEST("game.CargoSpec:parse:run-together", a)
{
    game::CargoSpec value;
    a.check("41. parse",              value.parse("1tdm42$", false));
    a.checkEqual("42. Tritanium",     value.get(value.Tritanium), 1);
    a.checkEqual("43. Duranium",      value.get(value.Duranium), 1);
    a.checkEqual("44. Molybdenum",    value.get(value.Molybdenum), 1);
    a.checkEqual("45. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("46. Money",         value.get(value.Money), 42);
    a.checkEqual("47. toPHostString", value.toPHostString(), "T1 D1 M1 $42");
    a.check("48. isZero",            !value.isZero());
}

// Standard cargospec with duplication
AFL_TEST("game.CargoSpec:parse:duplicate", a)
{
    game::CargoSpec value;
    a.check("51. parse",              value.parse("1ttttdm", false));
    a.checkEqual("52. Tritanium",     value.get(value.Tritanium), 4);
    a.checkEqual("53. Duranium",      value.get(value.Duranium), 1);
    a.checkEqual("54. Molybdenum",    value.get(value.Molybdenum), 1);
    a.checkEqual("55. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("56. Money",         value.get(value.Money), 0);
    a.checkEqual("57. toPHostString", value.toPHostString(), "T4 D1 M1");
    a.check("58. isZero",            !value.isZero());
}

// Standard cargospec with addition
AFL_TEST("game.CargoSpec:parse:add", a)
{
    game::CargoSpec value;
    a.check("61. parse",              value.parse("10s 20s", false));
    a.checkEqual("62. Tritanium",     value.get(value.Tritanium), 0);
    a.checkEqual("63. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("64. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("65. Supplies",      value.get(value.Supplies), 30);
    a.checkEqual("66. Money",         value.get(value.Money), 0);
    a.checkEqual("67. toPHostString", value.toPHostString(), "S30");
    a.check("68. isZero",            !value.isZero());
}

// Standard cargospec, uppercase
AFL_TEST("game.CargoSpec:parse:upper-case", a)
{
    game::CargoSpec value;
    a.check("71. parse",              value.parse("10TDM 99S", false));
    a.checkEqual("72. Tritanium",     value.get(value.Tritanium), 10);
    a.checkEqual("73. Duranium",      value.get(value.Duranium), 10);
    a.checkEqual("74. Molybdenum",    value.get(value.Molybdenum), 10);
    a.checkEqual("75. Supplies",      value.get(value.Supplies), 99);
    a.checkEqual("76. Money",         value.get(value.Money), 0);
    a.checkEqual("77. toPHostString", value.toPHostString(), "T10 D10 M10 S99");
    a.check("78. isZero",            !value.isZero());
}

// PHost-style
AFL_TEST("game.CargoSpec:parse:phost-style", a)
{
    game::CargoSpec value;
    a.check("81. parse",              value.parse("T10 D20 M30 $77 S42", false));
    a.checkEqual("82. Tritanium",     value.get(value.Tritanium), 10);
    a.checkEqual("83. Duranium",      value.get(value.Duranium), 20);
    a.checkEqual("84. Molybdenum",    value.get(value.Molybdenum), 30);
    a.checkEqual("85. Supplies",      value.get(value.Supplies), 42);
    a.checkEqual("86. Money",         value.get(value.Money), 77);
    a.checkEqual("87. toPHostString", value.toPHostString(), "T10 D20 M30 S42 $77");
    a.check("88. isZero",            !value.isZero());
}

// PHost-style, lower-case
AFL_TEST("game.CargoSpec:parse:phost-style:lower-case", a)
{
    game::CargoSpec value;
    a.check("91. parse",              value.parse("t11 d22 m33 $44 S55", false));
    a.checkEqual("92. Tritanium",     value.get(value.Tritanium), 11);
    a.checkEqual("93. Duranium",      value.get(value.Duranium), 22);
    a.checkEqual("94. Molybdenum",    value.get(value.Molybdenum), 33);
    a.checkEqual("95. Supplies",      value.get(value.Supplies), 55);
    a.checkEqual("96. Money",         value.get(value.Money), 44);
    a.checkEqual("97. toPHostString", value.toPHostString(), "T11 D22 M33 S55 $44");
    a.check("98. isZero",            !value.isZero());
}

// PHost-style, with addition
AFL_TEST("game.CargoSpec:parse:phost-style:add", a)
{
    game::CargoSpec value;
    a.check("101. parse",              value.parse("t11 t22 t33", false));
    a.checkEqual("102. Tritanium",     value.get(value.Tritanium), 66);
    a.checkEqual("103. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("104. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("105. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("106. Money",         value.get(value.Money), 0);
    a.checkEqual("107. toPHostString", value.toPHostString(), "T66");
}

// More types
AFL_TEST("game.CargoSpec:parse:types", a)
{
    game::CargoSpec value;
    a.check("111. parse",              value.parse("w5 f3", false));
    a.checkEqual("112. Tritanium",     value.get(value.Tritanium), 0);
    a.checkEqual("113. Duranium",      value.get(value.Duranium), 0);
    a.checkEqual("114. Molybdenum",    value.get(value.Molybdenum), 0);
    a.checkEqual("115. Supplies",      value.get(value.Supplies), 0);
    a.checkEqual("116. Money",         value.get(value.Money), 0);
    a.checkEqual("117",                value.get(value.Torpedoes), 5);
    a.checkEqual("118",                value.get(value.Fighters), 3);
    a.checkEqual("119. toPHostString", value.toPHostString(), "F3 W5");
}

// "max" syntax only if enabled
AFL_TEST("game.CargoSpec:parse:max:disabled", a)
{
    game::CargoSpec value;
    a.check("121", !value.parse("tmax", false));
}

AFL_TEST("game.CargoSpec:parse:max:enabled", a)
{
    game::CargoSpec value;
    a.check("122. parse",           value.parse("tmax", true));
    a.checkEqual("123. Tritanium",  value.get(value.Tritanium), 10000);
    a.checkEqual("124. Duranium",   value.get(value.Duranium), 0);
    a.checkEqual("125. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("126. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("127. Money",      value.get(value.Money), 0);
    a.check("128. isZero",         !value.isZero());
}

AFL_TEST("game.CargoSpec:parse:max:abbr", a)
{
    game::CargoSpec value;
    a.check("131. parse",           value.parse("tm", true));
    a.checkEqual("132. Tritanium",  value.get(value.Tritanium), 10000);
    a.checkEqual("133. Duranium",   value.get(value.Duranium), 0);
    a.checkEqual("134. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("135. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("136. Money",      value.get(value.Money), 0);
    a.check("137. isZero",         !value.isZero());
}

AFL_TEST("game.CargoSpec:parse:max+other", a)
{
    game::CargoSpec value;
    a.check("141. parse",           value.parse("tmax d10", true));
    a.checkEqual("142. Tritanium",  value.get(value.Tritanium), 10000);
    a.checkEqual("143. Duranium",   value.get(value.Duranium), 10);
    a.checkEqual("144. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("145. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("146. Money",      value.get(value.Money), 0);
    a.check("147. isZero",         !value.isZero());
}

AFL_TEST("game.CargoSpec:parse:max+other:abbr", a)
{
    game::CargoSpec value;
    a.check("151. parse",           value.parse("tm d10", true));
    a.checkEqual("152. Tritanium",  value.get(value.Tritanium), 10000);
    a.checkEqual("153. Duranium",   value.get(value.Duranium), 10);
    a.checkEqual("154. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("155. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("156. Money",      value.get(value.Money), 0);
    a.check("157. isZero",         !value.isZero());
}

// Sign
AFL_TEST("game.CargoSpec:parse:negative", a)
{
    game::CargoSpec value;
    a.check("161. parse",           value.parse("-10d", true));
    a.checkEqual("162. Tritanium",  value.get(value.Tritanium), 0);
    a.checkEqual("163. Duranium",   value.get(value.Duranium), -10);
    a.checkEqual("164. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("165. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("166. Money",      value.get(value.Money), 0);
    a.check("167. isZero",         !value.isZero());
}
AFL_TEST("game.CargoSpec:parse:positive", a)
{
    game::CargoSpec value;
    a.check("168. parse",           value.parse("+33d", true));
    a.checkEqual("169. Tritanium",  value.get(value.Tritanium), 0);
    a.checkEqual("170. Duranium",   value.get(value.Duranium), 33);
    a.checkEqual("171. Molybdenum", value.get(value.Molybdenum), 0);
    a.checkEqual("172. Supplies",   value.get(value.Supplies), 0);
    a.checkEqual("173. Money",      value.get(value.Money), 0);
    a.check("174. isZero",         !value.isZero());
}

/** Test parse errors. */
AFL_TEST("game.CargoSpec:parse:error", a)
{
    game::CargoSpec value;
    a.check("01", !value.parse("T", false));
    a.check("02", !value.parse("2", false));
    a.check("03", !value.parse("-D", false));
    a.check("04", !value.parse("-", false));
    a.check("05", !value.parse("-3", false));
    a.check("06", !value.parse("+", false));
    a.check("07", !value.parse("10TX", false));
    a.check("08", !value.parse("0x100M", false));
}

/*
 *  Addition operator.
 *
 *  These tests are mostly the same as for Cost.
 */
AFL_TEST("game.CargoSpec:add:single", a)
{
    game::CargoSpec ca("t1", false);
    game::CargoSpec cb("t42", false);
    ca += cb;
    a.checkEqual("01. Tritanium",  ca.get(ca.Tritanium), 43);
    a.checkEqual("02. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("03. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("04. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("05. Money",      ca.get(ca.Money), 0);
    a.check("06. isNonNegative",   ca.isNonNegative());
    a.check("07. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.CargoSpec:add:mixed", a)
{
    game::CargoSpec ca("t1", false);
    game::CargoSpec cb("s42", false);
    ca += cb;
    a.checkEqual("11. Tritanium",  ca.get(ca.Tritanium), 1);
    a.checkEqual("12. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("13. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("14. Supplies",   ca.get(ca.Supplies), 42);
    a.checkEqual("15. Money",      ca.get(ca.Money), 0);
    a.check("16. isNonNegative",   ca.isNonNegative());
    a.check("17. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.CargoSpec:add:sup+mc", a)
{
    game::CargoSpec ca("s100", false);
    game::CargoSpec cb("$200", false);
    ca += cb;
    a.checkEqual("21. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("22. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("23. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("24. Supplies",   ca.get(ca.Supplies), 100);
    a.checkEqual("25. Money",      ca.get(ca.Money), 200);
    a.check("26. isNonNegative",   ca.isNonNegative());
    a.check("27. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.CargoSpec:add:mc", a)
{
    game::CargoSpec ca;
    game::CargoSpec cb("$200", false);
    ca += cb;
    a.checkEqual("31. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("32. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("33. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("34. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("35. Money",      ca.get(ca.Money), 200);
    a.check("36. isNonNegative",   ca.isNonNegative());
    a.check("37. isNonNegative",   cb.isNonNegative());
}

/*
 *  Subtraction operator.
 *
 *  These tests are mostly the same as for Cost.
 */

AFL_TEST("game.CargoSpec:sub:single", a)
{
    game::CargoSpec ca("t1", false);
    game::CargoSpec cb("t42", false);
    ca -= cb;
    a.checkEqual("01. Tritanium",  ca.get(ca.Tritanium), -41);
    a.checkEqual("02. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("03. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("04. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("05. Money",      ca.get(ca.Money), 0);
    a.check("06. isNonNegative",  !ca.isNonNegative());
    a.check("07. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.CargoSpec:sub:mixed", a)
{
    game::CargoSpec ca("t1", false);
    game::CargoSpec cb("s42", false);
    ca -= cb;
    a.checkEqual("11. Tritanium",  ca.get(ca.Tritanium), 1);
    a.checkEqual("12. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("13. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("14. Supplies",   ca.get(ca.Supplies), -42);
    a.checkEqual("15. Money",      ca.get(ca.Money), 0);
    a.check("16. isNonNegative",  !ca.isNonNegative());
    a.check("17. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.CargoSpec:sub:sup+mc", a)
{
    game::CargoSpec ca("s100", false);
    game::CargoSpec cb("$200", false);
    ca -= cb;
    a.checkEqual("21. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("22. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("23. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("24. Supplies",   ca.get(ca.Supplies), 100);
    a.checkEqual("25. Money",      ca.get(ca.Money), -200);
    a.check("26. isNonNegative",  !ca.isNonNegative());
    a.check("27. isNonNegative",   cb.isNonNegative());
}

AFL_TEST("game.CargoSpec:sub:money", a)
{
    game::CargoSpec ca("$200", false);
    game::CargoSpec cb;
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
 *  Multiplication operator.
 *
 *  These tests are mostly the same as for Cost.
 */
AFL_TEST("game.CargoSpec:mul:zero-by-int", a)
{
    game::CargoSpec ca;
    ca *= 10;
    a.checkEqual("01. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("02. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("03. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("04. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("05. Money",      ca.get(ca.Money), 0);
}

AFL_TEST("game.CargoSpec:mul:by-int", a)
{
    game::CargoSpec ca("3tdm 42$", false);
    ca *= 10;
    a.checkEqual("11. Tritanium",  ca.get(ca.Tritanium), 30);
    a.checkEqual("12. Duranium",   ca.get(ca.Duranium), 30);
    a.checkEqual("13. Molybdenum", ca.get(ca.Molybdenum), 30);
    a.checkEqual("14. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("15. Money",      ca.get(ca.Money), 420);
}

AFL_TEST("game.CargoSpec:mul:infix", a)
{
    game::CargoSpec ca("3tdm 42$", false);
    game::CargoSpec cb = ca*10;
    a.checkEqual("21. Tritanium", cb.get(cb.Tritanium), 30);
    a.checkEqual("22. Duranium", cb.get(cb.Duranium), 30);
    a.checkEqual("23. Molybdenum", cb.get(cb.Molybdenum), 30);
    a.checkEqual("24. Supplies", cb.get(cb.Supplies), 0);
    a.checkEqual("25. Money", cb.get(cb.Money), 420);
}

/*
 *  Comparison operators.
 *
 *  These tests are mostly the same as for Cost.
 */
AFL_TEST("game.CargoSpec:comparison", a)
{
    // ==, !=
    a.check("01", game::CargoSpec("", false) == game::CargoSpec());
    a.check("02", game::CargoSpec("100$", false) == game::CargoSpec("$100", false));
    a.check("03", game::CargoSpec("5tdm", false) == game::CargoSpec("T5 5M 5d", false));
    a.check("04", game::CargoSpec("5tdm", false) != game::CargoSpec("T5 5M 5d 1d", false));
    a.check("05", game::CargoSpec("1t", false) != game::CargoSpec());
    a.check("06", game::CargoSpec("1d", false) != game::CargoSpec());
    a.check("07", game::CargoSpec("1m", false) != game::CargoSpec());
    a.check("08", game::CargoSpec("1$", false) != game::CargoSpec());
    a.check("09", game::CargoSpec("1s", false) != game::CargoSpec());
    a.check("10", game::CargoSpec("t1", false) != game::CargoSpec());
    a.check("11", game::CargoSpec("d1", false) != game::CargoSpec());
    a.check("12", game::CargoSpec("m1", false) != game::CargoSpec());
    a.check("13", game::CargoSpec("$1", false) != game::CargoSpec());
    a.check("14", game::CargoSpec("s1", false) != game::CargoSpec());
    a.check("15", game::CargoSpec("s100", false) != game::CargoSpec("$100", false));
    a.check("16", game::CargoSpec("$100", false) != game::CargoSpec("s100", false));
}

/** Mixed comparison.
    Because CargoSpec converts from Cost, these will work. */
AFL_TEST("game.CargoSpec:comparison:mixed", a)
{
    a.check("01", game::CargoSpec("5tdm", false) == game::spec::Cost::fromString("T5 5M 5d"));
    a.check("02", game::CargoSpec("5tdm", false) != game::spec::Cost::fromString("T5 5M 5d 1d"));
}

/*
 *  Division by scalar.
 */

AFL_TEST("game.CargoSpec:div:zero-by-int", a)
{
    game::CargoSpec ca;
    bool ok = ca.divide(10);
    a.check("01. divide", ok);
    a.checkEqual("02. Tritanium",  ca.get(ca.Tritanium), 0);
    a.checkEqual("03. Duranium",   ca.get(ca.Duranium), 0);
    a.checkEqual("04. Molybdenum", ca.get(ca.Molybdenum), 0);
    a.checkEqual("05. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("06. Money",      ca.get(ca.Money), 0);
}

AFL_TEST("game.CargoSpec:div:by-int", a)
{
    game::CargoSpec ca("30tdm 42$", false);
    bool ok = ca.divide(5);
    a.check("11. divide", ok);
    a.checkEqual("12. Tritanium",  ca.get(ca.Tritanium), 6);
    a.checkEqual("13. Duranium",   ca.get(ca.Duranium), 6);
    a.checkEqual("14. Molybdenum", ca.get(ca.Molybdenum), 6);
    a.checkEqual("15. Supplies",   ca.get(ca.Supplies), 0);
    a.checkEqual("16. Money",      ca.get(ca.Money), 8);
}

AFL_TEST("game.CargoSpec:div:by-zero", a)
{
    game::CargoSpec ca;
    bool ok = ca.divide(0);
    a.check("21. divide", !ok);
}

/*
 *  Division by cargo.
 */

AFL_TEST("game.CargoSpec:div:by-cargo", a)
{
    game::CargoSpec ca("100tdm", false);
    game::CargoSpec cb("25t 30d 10m", false);
    int32_t result;
    a.check("01. divide", ca.divide(cb, result));
    a.checkEqual("02. result", result, 3);
}

AFL_TEST("game.CargoSpec:div:by-cargo:missing-component", a)
{
    game::CargoSpec ca("100tdm", false);
    game::CargoSpec cb("25t 30d 10m 1$", false);
    int32_t result;
    a.check("03. divide", ca.divide(cb, result));
    a.checkEqual("04. result", result, 0);
}

AFL_TEST("game.CargoSpec:div:by-cargo:all-components-missing", a)
{
    game::CargoSpec ca("100tdm", false);
    game::CargoSpec cb("1$", false);
    int32_t result;
    a.check("05. divide", ca.divide(cb, result));
    a.checkEqual("06. result", result, 0);
}

AFL_TEST("game.CargoSpec:div:by-zero-cargo", a)
{
    game::CargoSpec ca("10t", false);
    game::CargoSpec cb("", false);
    int32_t result;
    a.check("07. divide", !ca.divide(cb, result));
}

AFL_TEST("game.CargoSpec:div:zero-cargo-by-zero-cargo", a)
{
    game::CargoSpec ca("", false);
    game::CargoSpec cb("", false);
    int32_t result;
    a.check("08. divide", !ca.divide(cb, result));
}

/** Test toCargoSpecString(). */
AFL_TEST("game.CargoSpec:toCargoSpecString", a)
{
    a.checkEqual("01", game::CargoSpec().toCargoSpecString(), "");
    a.checkEqual("02", game::CargoSpec("10t 3d", false).toCargoSpecString(), "10T 3D");
    a.checkEqual("03", game::CargoSpec("5d 5d 5d", false).toCargoSpecString(), "15D");
    a.checkEqual("04", game::CargoSpec("10t 10d 10m 30$", false).toCargoSpecString(), "10TDM 30$");
}

/*
 *  sellSuppliesIfNeeded().
 */

// Lack of money entirely compensated
AFL_TEST("game.CargoSpec:sellSuppliesIfNeeded:normal", a)
{
    game::CargoSpec ca("-5$ 10s", false);
    ca.sellSuppliesIfNeeded();
    a.checkEqual("01. toCargoSpecString", ca.toCargoSpecString(), "5S");
}

// Lack of money entirely compensated eating all supplies
AFL_TEST("game.CargoSpec:sellSuppliesIfNeeded:zero", a)
{
    game::CargoSpec ca("-5$ 5s", false);
    ca.sellSuppliesIfNeeded();
    a.checkEqual("11. toCargoSpecString", ca.toCargoSpecString(), "");
}

// Lack of supplies cannot be compensated
AFL_TEST("game.CargoSpec:sellSuppliesIfNeeded:missing-supplies", a)
{
    game::CargoSpec ca("10$ -5s", false);
    ca.sellSuppliesIfNeeded();
    a.checkEqual("21. toCargoSpecString", ca.toCargoSpecString(), "-5S 10$");
}

// Lack of money partially compensated
AFL_TEST("game.CargoSpec:sellSuppliesIfNeeded:missing-money", a)
{
    game::CargoSpec ca("-10$ 5s", false);
    ca.sellSuppliesIfNeeded();
    a.checkEqual("31. toCargoSpecString", ca.toCargoSpecString(), "-5$");
}

// Lack of everything left unchanged
AFL_TEST("game.CargoSpec:sellSuppliesIfNeeded:missing-everything", a)
{
    game::CargoSpec ca("-3$ -7s", false);
    ca.sellSuppliesIfNeeded();
    a.checkEqual("41. toCargoSpecString", ca.toCargoSpecString(), "-7S -3$");
}
