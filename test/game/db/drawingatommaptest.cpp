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
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "util/atomtable.hpp"

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

    void fillAtomTable(util::AtomTable& tab)
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
    util::AtomTable tab;
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
    util::AtomTable tab;
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
