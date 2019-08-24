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
        TS_ASSERT_EQUALS(util::Atom_t(image.atomA), tab.getAtomFromString("a"));
        TS_ASSERT_EQUALS(util::Atom_t(image.atomB), tab.getAtomFromString("b"));
        TS_ASSERT_EQUALS(image.charB, 'b');
    } else {
        TS_ASSERT_EQUALS(util::Atom_t(image.atomB), tab.getAtomFromString("a"));
        TS_ASSERT_EQUALS(util::Atom_t(image.atomA), tab.getAtomFromString("b"));
        TS_ASSERT_EQUALS(image.charA, 'b');
        TS_ASSERT_EQUALS(image.charB, 'a');
    }
}
