/**
  *  \file test/client/screenhistorytest.cpp
  *  \brief Test for client::ScreenHistory
  */

#include "client/screenhistory.hpp"

#include "afl/test/testrunner.hpp"

using client::ScreenHistory;

// Test "Reference" type
AFL_TEST("client.ScreenHistory:Reference", a)
{
    ScreenHistory::Reference one;
    ScreenHistory::Reference two(ScreenHistory::Ship, 99, 0);

    a.check("one isSet", !one.isSet());
    a.check("two isSet",  two.isSet());

    a.checkEqual("one type", one.getType(), ScreenHistory::Null);
    a.checkEqual("two type", two.getType(), ScreenHistory::Ship);

    a.checkEqual("two x", two.getX(), 99);
    a.checkEqual("two y", two.getY(), 0);

    a.check("equal one self", one == one);
    a.check("equal two self", two == two);
    a.check("unequal one self", !(one != one));
    a.check("unequal two self", !(two != two));

    a.check("equal both", !(one == two));
    a.check("unequal both", one != two);
}

// Test push/pop sequence
AFL_TEST("client.ScreenHistory:push+pop", a)
{
    ScreenHistory h(10);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,     99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));

    // Verify pop
    a.check("pop 1a", h.pop() == ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    a.check("pop 1b", h.pop() == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));
    a.check("pop 1c", h.pop() == ScreenHistory::Reference(ScreenHistory::Ship, 99, 0));

    a.check("pop 2a", h.pop() == ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    a.check("pop 2b", h.pop() == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));
    a.check("pop 2c", h.pop() == ScreenHistory::Reference(ScreenHistory::Ship, 99, 0));
}

// Test push/pop sequence with limited size
AFL_TEST("client.ScreenHistory:push+pop:limit", a)
{
    ScreenHistory h(2);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,     99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));

    // Verify pop
    a.check("pop 1a", h.pop() == ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    a.check("pop 1b", h.pop() == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));

    a.check("pop 2a", h.pop() == ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    a.check("pop 2b", h.pop() == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));
}

// Test pop from empty
AFL_TEST("client.ScreenHistory:push:empty", a)
{
    ScreenHistory h(10);
    a.check("pop", h.pop() == ScreenHistory::Reference(ScreenHistory::Null, 0, 0));
}

// Test pop from empty
AFL_TEST("client.ScreenHistory:clear:empty", a)
{
    ScreenHistory h(10);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,     99, 0));
    h.clear();

    a.check("pop", h.pop() == ScreenHistory::Reference(ScreenHistory::Null, 0, 0));
}

// Test push, getAll sequence
AFL_TEST("client.ScreenHistory:push+getAll", a)
{
    // Prepare
    ScreenHistory h(10);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,     99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));

    // Operate
    afl::base::Memory<const ScreenHistory::Reference> all = h.getAll();

    // Test
    a.checkEqual("size", all.size(), 3U);
    a.check("index 0", *all.at(0) == ScreenHistory::Reference(ScreenHistory::Ship, 99, 0));
    a.check("index 1", *all.at(1) == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));
    a.check("index 2", *all.at(2) == ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
}

// Test push sequence with redudant states
AFL_TEST("client.ScreenHistory:push:redundant", a)
{
    // Prepare
    ScreenHistory h(10);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,     99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   88, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   99, 0));       // redundant to previous
    h.push(ScreenHistory::Reference(ScreenHistory::Starchart, 99, 77));     // overwritten by next
    h.push(ScreenHistory::Reference(ScreenHistory::Starchart, 33, 55));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   99, 0));

    // Test
    afl::base::Memory<const ScreenHistory::Reference> all = h.getAll();
    a.checkEqual("size", all.size(), 5U);
    a.check("index 0", *all.at(0) == ScreenHistory::Reference(ScreenHistory::Ship, 99, 0));
    a.check("index 1", *all.at(1) == ScreenHistory::Reference(ScreenHistory::Planet, 88, 0));
    a.check("index 2", *all.at(2) == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));
    a.check("index 3", *all.at(3) == ScreenHistory::Reference(ScreenHistory::Starchart, 33, 55));
    a.check("index 4", *all.at(4) == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));
}

// Test push sequence with redudant states: big loop (A-B-C-A simplified to B-C-A)
AFL_TEST("client.ScreenHistory:push:redundant:big-loop", a)
{
    // Prepare
    ScreenHistory h(10);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,     99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,     99, 0));

    // Test
    afl::base::Memory<const ScreenHistory::Reference> all = h.getAll();
    a.checkEqual("size", all.size(), 3U);
    a.check("index 0", *all.at(0) == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));
    a.check("index 1", *all.at(1) == ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    a.check("index 2", *all.at(2) == ScreenHistory::Reference(ScreenHistory::Ship, 99, 0));
}

// Test push sequence with redudant states: small loop (A-B-C-B-C simplified to A-B-C-B)
AFL_TEST("client.ScreenHistory:push:redundant:small-loop", a)
{
    // Prepare
    ScreenHistory h(10);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,     99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::HistoryShip, 99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::HistoryShip, 99, 0));

    // Test
    afl::base::Memory<const ScreenHistory::Reference> all = h.getAll();
    a.checkEqual("size", all.size(), 4U);
    a.check("index 0", *all.at(0) == ScreenHistory::Reference(ScreenHistory::Ship, 99, 0));
    a.check("index 1", *all.at(1) == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));
    a.check("index 2", *all.at(2) == ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    a.check("index 3", *all.at(3) == ScreenHistory::Reference(ScreenHistory::HistoryShip, 99, 0));
}

// Test rotate operation
AFL_TEST("client.ScreenHistory:rotate", a)
{
    // Prepare
    ScreenHistory h(10);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,     99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,   99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));

    // Operate
    h.rotate();

    // Test
    afl::base::Memory<const ScreenHistory::Reference> all = h.getAll();
    a.checkEqual("size", all.size(), 3U);
    a.check("index 0", *all.at(0) == ScreenHistory::Reference(ScreenHistory::Starbase, 99, 0));
    a.check("index 1", *all.at(1) == ScreenHistory::Reference(ScreenHistory::Ship, 99, 0));
    a.check("index 2", *all.at(2) == ScreenHistory::Reference(ScreenHistory::Planet, 99, 0));
}

// Test applyMask
AFL_TEST("client.ScreenHistory:applyMask", a)
{
    // Prepare
    ScreenHistory h(10);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,        99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,      99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Starbase,    99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::HistoryShip, 99, 0));

    // Operate
    static const bool MASK[] = { true, false, false, true };
    h.applyMask(MASK);

    // Test
    afl::base::Memory<const ScreenHistory::Reference> all = h.getAll();
    a.checkEqual("size", all.size(), 2U);
    a.check("index 0", *all.at(0) == ScreenHistory::Reference(ScreenHistory::Ship, 99, 0));
    a.check("index 1", *all.at(1) == ScreenHistory::Reference(ScreenHistory::HistoryShip, 99, 0));
}

// Test applyMask, degenerate case
AFL_TEST("client.ScreenHistory:applyMask:empty", a)
{
    // Prepare
    ScreenHistory h(10);
    h.push(ScreenHistory::Reference(ScreenHistory::Ship,        99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Planet,      99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::Starbase,    99, 0));
    h.push(ScreenHistory::Reference(ScreenHistory::HistoryShip, 99, 0));

    // Operate
    h.applyMask(afl::base::Nothing);

    // Test
    afl::base::Memory<const ScreenHistory::Reference> all = h.getAll();
    a.checkEqual("size", all.size(), 0U);
}
