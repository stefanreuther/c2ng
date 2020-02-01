/**
  *  \file u/t_game_cargospec.cpp
  *  \brief Test for game::CargoSpec
  */

#include "game/cargospec.hpp"

#include "t_game.hpp"

/** Test parsing.
    These tests are mostly the same as for Cost. */
void
TestGameCargoSpec::testParse()
{
    {
        game::CargoSpec value;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 0);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT_EQUALS(value.toPHostString(), "S0");
        TS_ASSERT(value.isZero());
    }

    {
        // Blank cargospec
        game::CargoSpec value;
        TS_ASSERT(value.parse("", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 0);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT_EQUALS(value.toPHostString(), "S0");
        TS_ASSERT(value.isZero());
    }

    {
        // Zero cargospec
        game::CargoSpec value;
        TS_ASSERT(value.parse("0td", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 0);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT_EQUALS(value.toPHostString(), "S0");
        TS_ASSERT(value.isZero());
    }

    {
        // Standard cargospec (torpedo cost)
        game::CargoSpec value;
        TS_ASSERT(value.parse("1tdm 20$", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 1);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 1);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 1);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 20);
        TS_ASSERT_EQUALS(value.toPHostString(), "T1 D1 M1 $20");
        TS_ASSERT(!value.isZero());
    }

    {
        // Standard cargospec without space
        game::CargoSpec value;
        TS_ASSERT(value.parse("1tdm42$", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 1);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 1);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 1);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 42);
        TS_ASSERT_EQUALS(value.toPHostString(), "T1 D1 M1 $42");
        TS_ASSERT(!value.isZero());
    }

    {
        // Standard cargospec with duplication
        game::CargoSpec value;
        TS_ASSERT(value.parse("1ttttdm", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 4);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 1);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 1);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT_EQUALS(value.toPHostString(), "T4 D1 M1");
        TS_ASSERT(!value.isZero());
    }

    {
        // Standard cargospec with addition
        game::CargoSpec value;
        TS_ASSERT(value.parse("10s 20s", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 0);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 30);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT_EQUALS(value.toPHostString(), "S30");
        TS_ASSERT(!value.isZero());
    }

    {
        // Standard cargospec, uppercase
        game::CargoSpec value;
        TS_ASSERT(value.parse("10TDM 99S", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 10);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 10);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 99);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT_EQUALS(value.toPHostString(), "T10 D10 M10 S99");
        TS_ASSERT(!value.isZero());
    }

    {
        // PHost-style
        game::CargoSpec value;
        TS_ASSERT(value.parse("T10 D20 M30 $77 S42", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 20);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 30);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 42);
        TS_ASSERT_EQUALS(value.get(value.Money), 77);
        TS_ASSERT_EQUALS(value.toPHostString(), "T10 D20 M30 S42 $77");
        TS_ASSERT(!value.isZero());
    }

    {
        // PHost-style, lower-case
        game::CargoSpec value;
        TS_ASSERT(value.parse("t11 d22 m33 $44 S55", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 11);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 22);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 33);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 55);
        TS_ASSERT_EQUALS(value.get(value.Money), 44);
        TS_ASSERT_EQUALS(value.toPHostString(), "T11 D22 M33 S55 $44");
        TS_ASSERT(!value.isZero());
    }

    {
        // PHost-style, with addition
        game::CargoSpec value;
        TS_ASSERT(value.parse("t11 t22 t33", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 66);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT_EQUALS(value.toPHostString(), "T66");
    }

    {
        // More types
        game::CargoSpec value;
        TS_ASSERT(value.parse("w5 f3", false));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 0);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT_EQUALS(value.get(value.Torpedoes), 5);
        TS_ASSERT_EQUALS(value.get(value.Fighters), 3);
        TS_ASSERT_EQUALS(value.toPHostString(), "F3 W5");
    }

    {
        // "max" syntax only if enabled
        game::CargoSpec value;
        TS_ASSERT(!value.parse("tmax", false));;
    }
    {
        game::CargoSpec value;
        TS_ASSERT(value.parse("tmax", true));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10000);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }

    {
        game::CargoSpec value;
        TS_ASSERT(value.parse("tm", true));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10000);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }

    {
        game::CargoSpec value;
        TS_ASSERT(value.parse("tmax d10", true));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10000);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 10);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }

    {
        game::CargoSpec value;
        TS_ASSERT(value.parse("tm d10", true));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10000);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 10);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }

    // Sign
    {
        game::CargoSpec value;
        TS_ASSERT(value.parse("-10d", true));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 0);
        TS_ASSERT_EQUALS(value.get(value.Duranium), -10);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }
    {
        game::CargoSpec value;
        TS_ASSERT(value.parse("+33d", true));;
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 0);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 33);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }
}

/** Test parse errors. */
void
TestGameCargoSpec::testParseError()
{
    game::CargoSpec value;
    TS_ASSERT(!value.parse("T", false));
    TS_ASSERT(!value.parse("2", false));
    TS_ASSERT(!value.parse("-D", false));
    TS_ASSERT(!value.parse("-", false));
    TS_ASSERT(!value.parse("-3", false));
    TS_ASSERT(!value.parse("+", false));
    TS_ASSERT(!value.parse("10TX", false));
    TS_ASSERT(!value.parse("0x100M", false));
}

/** Test addition operator.
    These tests are mostly the same as for Cost. */
void
TestGameCargoSpec::testAdd()
{
    // +=
    {
        game::CargoSpec a("t1", false);
        game::CargoSpec b("t42", false);
        a += b;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 43);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 0);
        TS_ASSERT(a.isNonNegative());
        TS_ASSERT(b.isNonNegative());
    }

    {
        game::CargoSpec a("t1", false);
        game::CargoSpec b("s42", false);
        a += b;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 1);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 42);
        TS_ASSERT_EQUALS(a.get(a.Money), 0);
        TS_ASSERT(a.isNonNegative());
        TS_ASSERT(b.isNonNegative());
    }

    {
        game::CargoSpec a("s100", false);
        game::CargoSpec b("$200", false);
        a += b;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 0);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 100);
        TS_ASSERT_EQUALS(a.get(a.Money), 200);
        TS_ASSERT(a.isNonNegative());
        TS_ASSERT(b.isNonNegative());
    }

    {
        game::CargoSpec a;
        game::CargoSpec b("$200", false);
        a += b;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 0);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 200);
        TS_ASSERT(a.isNonNegative());
        TS_ASSERT(b.isNonNegative());
    }
}

/** Test subtraction operator.
    These tests are mostly the same as for Cost. */
void
TestGameCargoSpec::testSubtract()
{
    // -=
    {
        game::CargoSpec a("t1", false);
        game::CargoSpec b("t42", false);
        a -= b;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), -41);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 0);
        TS_ASSERT(!a.isNonNegative());
        TS_ASSERT(b.isNonNegative());
    }

    {
        game::CargoSpec a("t1", false);
        game::CargoSpec b("s42", false);
        a -= b;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 1);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), -42);
        TS_ASSERT_EQUALS(a.get(a.Money), 0);
        TS_ASSERT(!a.isNonNegative());
        TS_ASSERT(b.isNonNegative());
    }

    {
        game::CargoSpec a("s100", false);
        game::CargoSpec b("$200", false);
        a -= b;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 0);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 100);
        TS_ASSERT_EQUALS(a.get(a.Money), -200);
        TS_ASSERT(!a.isNonNegative());
        TS_ASSERT(b.isNonNegative());
    }

    {
        game::CargoSpec a("$200", false);
        game::CargoSpec b;
        a -= b;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 0);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 200);
        TS_ASSERT(a.isNonNegative());
        TS_ASSERT(b.isNonNegative());
    }
}

/** Test multiplication operator.
    These tests are mostly the same as for Cost. */
void
TestGameCargoSpec::testMult()
{
    // *=, *
    {
        game::CargoSpec a;
        a *= 10;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 0);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 0);
    }

    {
        game::CargoSpec a("3tdm 42$", false);
        a *= 10;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 30);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 30);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 30);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 420);
    }

    {
        game::CargoSpec a("3tdm 42$", false);
        game::CargoSpec b = a*10;
        TS_ASSERT_EQUALS(b.get(b.Tritanium), 30);
        TS_ASSERT_EQUALS(b.get(b.Duranium), 30);
        TS_ASSERT_EQUALS(b.get(b.Molybdenum), 30);
        TS_ASSERT_EQUALS(b.get(b.Supplies), 0);
        TS_ASSERT_EQUALS(b.get(b.Money), 420);
    }
}

/** Test comparison operators.
    These tests are mostly the same as for Cost. */
void
TestGameCargoSpec::testCompare()
{
    // ==, !=
    TS_ASSERT(game::CargoSpec("", false) == game::CargoSpec());
    TS_ASSERT(game::CargoSpec("100$", false) == game::CargoSpec("$100", false));
    TS_ASSERT(game::CargoSpec("5tdm", false) == game::CargoSpec("T5 5M 5d", false));
    TS_ASSERT(game::CargoSpec("5tdm", false) != game::CargoSpec("T5 5M 5d 1d", false));
    TS_ASSERT(game::CargoSpec("1t", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("1d", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("1m", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("1$", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("1s", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("t1", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("d1", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("m1", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("$1", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("s1", false) != game::CargoSpec());
    TS_ASSERT(game::CargoSpec("s100", false) != game::CargoSpec("$100", false));
    TS_ASSERT(game::CargoSpec("$100", false) != game::CargoSpec("s100", false));
}

/** Mixed comparison.
    Because CargoSpec converts from Cost, these will work. */
void
TestGameCargoSpec::testMixedCompare()
{
    TS_ASSERT(game::CargoSpec("5tdm", false) == game::spec::Cost::fromString("T5 5M 5d"));
    TS_ASSERT(game::CargoSpec("5tdm", false) != game::spec::Cost::fromString("T5 5M 5d 1d"));
}

/** Test division by scalar. */
void
TestGameCargoSpec::testDivide1()
{
    {
        game::CargoSpec a;
        bool ok = a.divide(10);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 0);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 0);
    }

    {
        game::CargoSpec a("30tdm 42$", false);
        bool ok = a.divide(5);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 6);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 6);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 6);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 8);
    }

    {
        game::CargoSpec a;
        bool ok = a.divide(0);
        TS_ASSERT(!ok);
    }
}

/** Test division by cargo. */
void
TestGameCargoSpec::testDivide2()
{
    {
        game::CargoSpec a("100tdm", false);
        game::CargoSpec b("25t 30d 10m", false);
        int32_t result;
        TS_ASSERT(a.divide(b, result));
        TS_ASSERT_EQUALS(result, 3);
    }
    {
        game::CargoSpec a("100tdm", false);
        game::CargoSpec b("25t 30d 10m 1$", false);
        int32_t result;
        TS_ASSERT(a.divide(b, result));
        TS_ASSERT_EQUALS(result, 0);
    }
    {
        game::CargoSpec a("100tdm", false);
        game::CargoSpec b("1$", false);
        int32_t result;
        TS_ASSERT(a.divide(b, result));
        TS_ASSERT_EQUALS(result, 0);
    }
    {
        game::CargoSpec a("10t", false);
        game::CargoSpec b("", false);
        int32_t result;
        TS_ASSERT(!a.divide(b, result));
    }
    {
        game::CargoSpec a("", false);
        game::CargoSpec b("", false);
        int32_t result;
        TS_ASSERT(!a.divide(b, result));
    }
}

/** Test toCargoSpecString(). */
void
TestGameCargoSpec::testToString()
{
    TS_ASSERT_EQUALS(game::CargoSpec().toCargoSpecString(), "");
    TS_ASSERT_EQUALS(game::CargoSpec("10t 3d", false).toCargoSpecString(), "10T 3D");
    TS_ASSERT_EQUALS(game::CargoSpec("5d 5d 5d", false).toCargoSpecString(), "15D");
    TS_ASSERT_EQUALS(game::CargoSpec("10t 10d 10m 30$", false).toCargoSpecString(), "10TDM 30$");
}

/** Test sellSuppliesIfNeeded(). */
void
TestGameCargoSpec::testSellSuppliesIfNeeded()
{
    // Lack of money entirely compensated
    {
        game::CargoSpec a("-5$ 10s", false);
        a.sellSuppliesIfNeeded();
        TS_ASSERT_EQUALS(a.toCargoSpecString(), "5S");
    }

    // Lack of money entirely compensated eating all supplies
    {
        game::CargoSpec a("-5$ 5s", false);
        a.sellSuppliesIfNeeded();
        TS_ASSERT_EQUALS(a.toCargoSpecString(), "");
    }

    // Lack of supplies cannot be compensated
    {
        game::CargoSpec a("10$ -5s", false);
        a.sellSuppliesIfNeeded();
        TS_ASSERT_EQUALS(a.toCargoSpecString(), "-5S 10$");
    }

    // Lack of money partially compensated
    {
        game::CargoSpec a("-10$ 5s", false);
        a.sellSuppliesIfNeeded();
        TS_ASSERT_EQUALS(a.toCargoSpecString(), "-5$");
    }

    // Lack of everything left unchanged
    {
        game::CargoSpec a("-3$ -7s", false);
        a.sellSuppliesIfNeeded();
        TS_ASSERT_EQUALS(a.toCargoSpecString(), "-7S -3$");
    }
}

