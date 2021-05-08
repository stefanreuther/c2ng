/**
  *  \file u/t_game_db_drawingatommap.cpp
  *  \brief Test for game::db::DrawingAtomMap
  */

#include "game/db/drawingatommap.hpp"

#include "t_game_db.hpp"
#include "afl/io/internalstream.hpp"
#include "util/atomtable.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/value.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/sys/log.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/constmemorystream.hpp"

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
void
TestGameDbDrawingAtomMap::testIt()
{
    game::db::DrawingAtomMap testee;
    TS_ASSERT(testee.isEmpty());

    testee.add(999);
    TS_ASSERT(!testee.isEmpty());

    testee.clear();
    TS_ASSERT(testee.isEmpty());
}

/** Test saving. */
void
TestGameDbDrawingAtomMap::testSave()
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
    TS_ASSERT_EQUALS(stream.getSize(), 10U);

    // Verify image
    Image image;
    afl::base::fromObject(image).copyFrom(stream.getContent());
    TS_ASSERT_EQUALS(int(image.count), 2);
    TS_ASSERT_EQUALS(image.lengthA, 1);
    TS_ASSERT_EQUALS(image.lengthB, 1);

    // Verify atom names
    if (image.charA == 'a') {
        TS_ASSERT_EQUALS(testee.get(image.atomA), tab.getAtomFromString("a"));
        TS_ASSERT_EQUALS(testee.get(image.atomB), tab.getAtomFromString("b"));
        TS_ASSERT_EQUALS(image.atomA, testee.getExternalValue(tab.getAtomFromString("a")));
        TS_ASSERT_EQUALS(image.atomB, testee.getExternalValue(tab.getAtomFromString("b")));
        TS_ASSERT_EQUALS(image.charB, 'b');
    } else {
        TS_ASSERT_EQUALS(testee.get(image.atomB), tab.getAtomFromString("a"));
        TS_ASSERT_EQUALS(testee.get(image.atomA), tab.getAtomFromString("b"));
        TS_ASSERT_EQUALS(image.atomB, testee.getExternalValue(tab.getAtomFromString("a")));
        TS_ASSERT_EQUALS(image.atomA, testee.getExternalValue(tab.getAtomFromString("b")));
        TS_ASSERT_EQUALS(image.charB, 'a');
        TS_ASSERT_EQUALS(image.charA, 'b');
    }
}

/** Test loading. */
void
TestGameDbDrawingAtomMap::testLoad()
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
    TS_ASSERT_EQUALS(tab.getStringFromAtom(testee.get(99)), "x");
    TS_ASSERT_EQUALS(tab.getStringFromAtom(testee.get(77)), "y");

    TS_ASSERT_EQUALS(testee.getExternalValue(tab.getAtomFromString("x")), 99U);
    TS_ASSERT_EQUALS(testee.getExternalValue(tab.getAtomFromString("y")), 77U);

    TS_ASSERT_DIFFERS(testee.get(99), 0U);
    TS_ASSERT_DIFFERS(testee.get(77), 0U);
    TS_ASSERT_EQUALS(testee.get(0), 0U);
    TS_ASSERT_EQUALS(testee.get(1000), 1000U);  // unmapped value is passed through

    TS_ASSERT_EQUALS(testee.getExternalValue(0), 0U);
    TS_ASSERT_EQUALS(testee.getExternalValue(1000), 1000U);  // unmapped value is passed through
}

