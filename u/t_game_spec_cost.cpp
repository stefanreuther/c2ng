/**
  *  \file u/game/spec/t_cost.cpp
  */

#include "game/spec/cost.hpp"

#include "u/t_game_spec.hpp"
/** Tests various cases of GCost::fromString.

    This does not test invalid cases, as those are not yet defined. As of 20081216,
    invalid characters are ignored by GCost::fromString, and treated as if not present.
    Possible cases that need definition:
    - invalid cargo letters
    - overflow
    - signs */
void
TestGameSpecCost::testParse()
{
    /* Like PCC 1.x, we accept cargospecs and PHost format:
          Cargospec:  123TDM
          PHost:      T123 D123 M123 */
    {
        // Zero-initalisation
        game::spec::Cost value;
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
        game::spec::Cost value = game::spec::Cost::fromString("");
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
        game::spec::Cost value = game::spec::Cost::fromString("0td");
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
        game::spec::Cost value = game::spec::Cost::fromString("1tdm 20$");
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
        game::spec::Cost value = game::spec::Cost::fromString("1tdm42$");
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
        game::spec::Cost value = game::spec::Cost::fromString("1ttttdm");
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
        game::spec::Cost value = game::spec::Cost::fromString("10s 20s");
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
        game::spec::Cost value = game::spec::Cost::fromString("10TDM 99S");
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
        game::spec::Cost value = game::spec::Cost::fromString("T10 D20 M30 $77 S42");
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
        game::spec::Cost value = game::spec::Cost::fromString("t11 d22 m33 $44 S55");
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
        game::spec::Cost value = game::spec::Cost::fromString("t11 t22 t33");
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 66);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT_EQUALS(value.toPHostString(), "T66");
    }

    {
        // game::spec::Cost parses using CargoSpec::parse(..., true), so check "max" syntax
        game::spec::Cost value = game::spec::Cost::fromString("tmax");
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10000);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }

    {
        game::spec::Cost value = game::spec::Cost::fromString("tm");
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10000);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 0);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }

    {
        game::spec::Cost value = game::spec::Cost::fromString("tmax d10");
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10000);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 10);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }

    {
        game::spec::Cost value = game::spec::Cost::fromString("tm d10");
        TS_ASSERT_EQUALS(value.get(value.Tritanium), 10000);
        TS_ASSERT_EQUALS(value.get(value.Duranium), 10);
        TS_ASSERT_EQUALS(value.get(value.Molybdenum), 0);
        TS_ASSERT_EQUALS(value.get(value.Supplies), 0);
        TS_ASSERT_EQUALS(value.get(value.Money), 0);
        TS_ASSERT(!value.isZero());
    }
}

void
TestGameSpecCost::testAdd()
{
    // +=
    {
        game::spec::Cost a = game::spec::Cost::fromString("t1");
        game::spec::Cost b = game::spec::Cost::fromString("t42");
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
        game::spec::Cost a = game::spec::Cost::fromString("t1");
        game::spec::Cost b = game::spec::Cost::fromString("s42");
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
        game::spec::Cost a = game::spec::Cost::fromString("s100");
        game::spec::Cost b = game::spec::Cost::fromString("$200");
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
        game::spec::Cost a;
        game::spec::Cost b = game::spec::Cost::fromString("$200");
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

void
TestGameSpecCost::testSubtract()
{
    // -=
    {
        game::spec::Cost a = game::spec::Cost::fromString("t1");
        game::spec::Cost b = game::spec::Cost::fromString("t42");
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
        game::spec::Cost a = game::spec::Cost::fromString("t1");
        game::spec::Cost b = game::spec::Cost::fromString("s42");
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
        game::spec::Cost a = game::spec::Cost::fromString("s100");
        game::spec::Cost b = game::spec::Cost::fromString("$200");
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
        game::spec::Cost a = game::spec::Cost::fromString("$200");
        game::spec::Cost b;
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

void
TestGameSpecCost::testMult()
{
    // *=, *
    {
        game::spec::Cost a;
        a *= 10;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 0);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 0);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 0);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 0);
    }

    {
        game::spec::Cost a = game::spec::Cost::fromString("3tdm 42$");
        a *= 10;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 30);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 30);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 30);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 420);
    }

    {
        game::spec::Cost a = game::spec::Cost::fromString("3tdm 42$");
        game::spec::Cost b = a * 10;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 3);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 3);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 3);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 0);
        TS_ASSERT_EQUALS(a.get(a.Money), 42);
        TS_ASSERT_EQUALS(b.get(b.Tritanium), 30);
        TS_ASSERT_EQUALS(b.get(b.Duranium), 30);
        TS_ASSERT_EQUALS(b.get(b.Molybdenum), 30);
        TS_ASSERT_EQUALS(b.get(b.Supplies), 0);
        TS_ASSERT_EQUALS(b.get(b.Money), 420);
    }

    {
        game::spec::Cost a = game::spec::Cost::fromString("1t 2d 3m 4$ 5s") * 7;
        TS_ASSERT_EQUALS(a.get(a.Tritanium), 7);
        TS_ASSERT_EQUALS(a.get(a.Duranium), 14);
        TS_ASSERT_EQUALS(a.get(a.Molybdenum), 21);
        TS_ASSERT_EQUALS(a.get(a.Supplies), 35);
        TS_ASSERT_EQUALS(a.get(a.Money), 28);
    }
}

void
TestGameSpecCost::testCompare()
{
    // ==, !=
    TS_ASSERT(game::spec::Cost::fromString("") == game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("100$") == game::spec::Cost::fromString("$100"));
    TS_ASSERT(game::spec::Cost::fromString("5tdm") == game::spec::Cost::fromString("T5 5M 5d"));
    TS_ASSERT(game::spec::Cost::fromString("5tdm") != game::spec::Cost::fromString("T5 5M 5d 1d"));
    TS_ASSERT(game::spec::Cost::fromString("1t") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("1d") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("1m") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("1$") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("1s") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("t1") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("d1") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("m1") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("$1") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("s1") != game::spec::Cost());
    TS_ASSERT(game::spec::Cost::fromString("s100") != game::spec::Cost::fromString("$100"));
    TS_ASSERT(game::spec::Cost::fromString("$100") != game::spec::Cost::fromString("s100"));
}

void
TestGameSpecCost::testEnough()
{
    // isEnoughFor()
    // Equality:
    TS_ASSERT( game::spec::Cost().isEnoughFor(game::spec::Cost()));
    TS_ASSERT( game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1t")));
    TS_ASSERT( game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1d")));
    TS_ASSERT( game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1m")));
    TS_ASSERT( game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1s")));
    TS_ASSERT( game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1$")));

    // Strictly more:
    TS_ASSERT(!game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("2t")));
    TS_ASSERT(!game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("2d")));
    TS_ASSERT(!game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("2m")));
    TS_ASSERT(!game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("2s")));
    TS_ASSERT(!game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("2$")));

    // Element (Non-)Conversions:
    TS_ASSERT( game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1t")));
    TS_ASSERT(!game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1t")));
    TS_ASSERT(!game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1t")));
    TS_ASSERT(!game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1t")));
    TS_ASSERT(!game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1t")));

    TS_ASSERT(!game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1d")));
    TS_ASSERT( game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1d")));
    TS_ASSERT(!game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1d")));
    TS_ASSERT(!game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1d")));
    TS_ASSERT(!game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1d")));

    TS_ASSERT(!game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1m")));
    TS_ASSERT(!game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1m")));
    TS_ASSERT( game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1m")));
    TS_ASSERT(!game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1m")));
    TS_ASSERT(!game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1m")));

    TS_ASSERT(!game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1s")));
    TS_ASSERT(!game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1s")));
    TS_ASSERT(!game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1s")));
    TS_ASSERT( game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1s")));
    TS_ASSERT(!game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1s")));

    TS_ASSERT(!game::spec::Cost::fromString("1t").isEnoughFor(game::spec::Cost::fromString("1$")));
    TS_ASSERT(!game::spec::Cost::fromString("1d").isEnoughFor(game::spec::Cost::fromString("1$")));
    TS_ASSERT(!game::spec::Cost::fromString("1m").isEnoughFor(game::spec::Cost::fromString("1$")));
    TS_ASSERT( game::spec::Cost::fromString("1s").isEnoughFor(game::spec::Cost::fromString("1$")));
    TS_ASSERT( game::spec::Cost::fromString("1$").isEnoughFor(game::spec::Cost::fromString("1$")));

    // Combinations including supply sale:
    TS_ASSERT( game::spec::Cost::fromString("5t 3d 7m 99$").isEnoughFor(game::spec::Cost::fromString("3tdm 42$")));
    TS_ASSERT( game::spec::Cost::fromString("5t 3d 7m 99s").isEnoughFor(game::spec::Cost::fromString("3tdm 42$")));
    TS_ASSERT(!game::spec::Cost::fromString("5t 3d 7m 99s").isEnoughFor(game::spec::Cost::fromString("4tdm 42$")));
    TS_ASSERT( game::spec::Cost::fromString("5t 3d 7m 22s 22$").isEnoughFor(game::spec::Cost::fromString("3tdm 42$")));
    TS_ASSERT(!game::spec::Cost::fromString("5t 3d 7m 22s 22$").isEnoughFor(game::spec::Cost::fromString("3tdm 52$")));
}
