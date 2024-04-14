/**
  *  \file test/game/db/drawingatommaptest.cpp
  *  \brief Test for game::db::DrawingAtomMap
  */

#include "game/db/drawingatommap.hpp"

#include "afl/base/staticassert.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/value.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "util/atomtable.hpp"

using util::AtomTable;

namespace {
    // Image of the file for testSave().
    // Must be a global structure because it is indirectly used as a template parameter.
    struct Image {
        afl::bits::Value<afl::bits::UInt16LE> count, atomA, atomB;
        uint8_t lengthA;
        char charA;
        uint8_t lengthB;
        char charB;
    };
    static_assert(sizeof(Image) == 10, "sizeof Image");

    void fillAtomTable(AtomTable& tab)
    {
        // Occupy some slots in atom table so external and internal disagree
        // and mismatches are detected.
        tab.getAtomFromString("1");
        tab.getAtomFromString("2");
        tab.getAtomFromString("3");
    }
}

/** Simple accessors. */
AFL_TEST("game.db.DrawingAtomMap:accessors", a)
{
    game::db::DrawingAtomMap testee;
    a.check("01. isEmpty", testee.isEmpty());

    testee.add(999);
    a.check("11. isEmpty", !testee.isEmpty());

    testee.clear();
    a.check("21. isEmpty", testee.isEmpty());
}

/** Test saving. */
AFL_TEST("game.db.DrawingAtomMap:save", a)
{
    // Prepare
    game::db::DrawingAtomMap testee;
    AtomTable tab;
    fillAtomTable(tab);
    testee.add(tab.getAtomFromString("a"));
    testee.add(tab.getAtomFromString("b"));

    // Save
    afl::io::InternalStream stream;
    afl::charset::Utf8Charset cs;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    testee.save(stream, cs, tab, log, tx);

    // Result is
    //   word(2)        number of elements
    //   word(atom(A))  atom values
    //   word(atom(B))
    //   string("a")    atom names
    //   string("b")    atom names
    a.checkEqual("01. getSize", stream.getSize(), 10U);

    // Verify image
    Image image;
    afl::base::fromObject(image).copyFrom(stream.getContent());
    a.checkEqual("11. count", int(image.count), 2);
    a.checkEqual("12. lengthA", image.lengthA, 1);
    a.checkEqual("13. lengthB", image.lengthB, 1);

    // Verify atom names
    if (image.charA == 'a') {
        a.checkEqual("21. atomA", testee.get(image.atomA), tab.getAtomFromString("a"));
        a.checkEqual("22. atomB", testee.get(image.atomB), tab.getAtomFromString("b"));
        a.checkEqual("23. atomA", image.atomA, testee.getExternalValue(tab.getAtomFromString("a")));
        a.checkEqual("24. atomB", image.atomB, testee.getExternalValue(tab.getAtomFromString("b")));
        a.checkEqual("25. charB", image.charB, 'b');
    } else {
        a.checkEqual("26. atomB", testee.get(image.atomB), tab.getAtomFromString("a"));
        a.checkEqual("27. atomA", testee.get(image.atomA), tab.getAtomFromString("b"));
        a.checkEqual("28. atomB", image.atomB, testee.getExternalValue(tab.getAtomFromString("a")));
        a.checkEqual("29. atomA", image.atomA, testee.getExternalValue(tab.getAtomFromString("b")));
        a.checkEqual("30. charB", image.charB, 'a');
        a.checkEqual("31. charA", image.charA, 'b');
    }
}

/** Test loading. */
AFL_TEST("game.db.DrawingAtomMap:load", a)
{
    // Create image
    Image image;
    image.count = 2;
    image.atomA = 99;
    image.atomB = 77;
    image.lengthA = 1;
    image.charA = 'x';
    image.lengthB = 1;
    image.charB = 'y';

    // Load
    AtomTable tab;
    afl::charset::Utf8Charset cs;
    afl::io::ConstMemoryStream ms(afl::base::fromObject(image));
    fillAtomTable(tab);

    game::db::DrawingAtomMap testee;
    testee.load(ms, cs, tab);

    // Verify
    a.checkEqual("01. getStringFromAtom", tab.getStringFromAtom(testee.get(99)), "x");
    a.checkEqual("02. getStringFromAtom", tab.getStringFromAtom(testee.get(77)), "y");

    a.checkEqual("11. getExternalValue", testee.getExternalValue(tab.getAtomFromString("x")), 99U);
    a.checkEqual("12. getExternalValue", testee.getExternalValue(tab.getAtomFromString("y")), 77U);

    a.checkDifferent("21. get", testee.get(99), 0U);
    a.checkDifferent("22. get", testee.get(77), 0U);
    a.checkEqual("23. get", testee.get(0), 0U);
    a.checkEqual("24. get", testee.get(1000), 1000U);  // unmapped value is passed through

    a.checkEqual("31. getExternalValue", testee.getExternalValue(0), 0U);
    a.checkEqual("32. getExternalValue", testee.getExternalValue(1000), 1000U);  // unmapped value is passed through
}

/** Test saving: too many atoms.
    Result must be loadable. */
AFL_TEST("game.db.DrawingAtomMap:save:too-many", a)
{
    // Prepare
    game::db::DrawingAtomMap testee;
    AtomTable tab;
    fillAtomTable(tab);
    for (int i = 0; i < 20000; ++i) {
        testee.add(tab.getAtomFromString(afl::string::Format("a%d", i)));
    }

    // Save
    afl::io::InternalStream stream;
    afl::charset::Utf8Charset cs;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    testee.save(stream, cs, tab, log, tx);

    // Load
    game::db::DrawingAtomMap loadedMap;
    AtomTable loadedTable;
    stream.setPos(0);
    AFL_CHECK_SUCCEEDS(a("01. load"), loadedMap.load(stream, cs, loadedTable));

    // Must have loaded some content.
    // We're preserving the first, although that's not strictly contractual.
    util::Atom_t origAtom = tab.getAtomFromStringNC("a0");
    util::Atom_t loadedAtom = loadedTable.getAtomFromStringNC("a0");
    a.checkDifferent("11. origAtom",   origAtom,   AtomTable::NULL_ATOM);
    a.checkDifferent("12. loadedAtom", loadedAtom, AtomTable::NULL_ATOM);
    a.checkEqual("13. atom map", loadedMap.getExternalValue(loadedAtom), testee.getExternalValue(origAtom));
}

/** Test saving: too long string.
    Result must be loadable. */
AFL_TEST("game.db.DrawingAtomMap:save:too-long", a)
{
    // Prepare
    game::db::DrawingAtomMap testee;
    AtomTable tab;
    fillAtomTable(tab);

    const String_t str(300, 'x');
    testee.add(tab.getAtomFromString(str));
    uint16_t externalAtom = testee.getExternalValue(tab.getAtomFromString(str));

    // Save
    afl::io::InternalStream stream;
    afl::charset::Utf8Charset cs;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    testee.save(stream, cs, tab, log, tx);

    // Load
    game::db::DrawingAtomMap loadedMap;
    AtomTable loadedTable;
    stream.setPos(0);
    AFL_CHECK_SUCCEEDS(a("01. load"), loadedMap.load(stream, cs, loadedTable));

    // Must have loaded some content.
    util::Atom_t loadedAtom = loadedMap.get(externalAtom);
    String_t loadedString = loadedTable.getStringFromAtom(loadedAtom);
    a.checkEqual("11. loaded length", loadedString.size(), 255U);
    a.checkEqual("12. loaded content", loadedString.find_first_not_of('x'), String_t::npos);
}
