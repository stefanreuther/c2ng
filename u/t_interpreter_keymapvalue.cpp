/**
  *  \file u/t_interpreter_keymapvalue.cpp
  *  \brief Test for interpreter::KeymapValue
  */

#include <memory>
#include "interpreter/keymapvalue.hpp"

#include "t_interpreter.hpp"
#include "util/keymap.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/charset/utf8charset.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"
#include "interpreter/error.hpp"

/** Test KeymapValue. */
void
TestInterpreterKeymapValue::testIt()
{
    // Create a keymap.
    // This assumes that a keymap can live on its own and a KeymapRef_t is just a dumb pointer.
    util::Keymap k("ZZ");

    // Verify that KeymapRef_t actually is a dumb pointer!
    util::Keymap* pk = &k;
    util::KeymapRef_t& rpk = pk;
    (void) rpk;

    // Testee
    interpreter::KeymapValue testee(pk);

    // Verify
    TS_ASSERT_EQUALS(testee.getKeymap(), pk);
    TS_ASSERT_EQUALS(testee.toString(false), testee.toString(true));
    TS_ASSERT_EQUALS(testee.toString(false).substr(0, 2), "#<");

    std::auto_ptr<interpreter::KeymapValue> copy(testee.clone());
    TS_ASSERT(copy.get() != 0);
    TS_ASSERT_EQUALS(copy->getKeymap(), pk);
    TS_ASSERT_EQUALS(copy->toString(false), testee.toString(false));

    // Store
    interpreter::TagNode out;
    afl::io::InternalSink aux;
    afl::charset::Utf8Charset cs;
    interpreter::vmio::NullSaveContext ctx;
    TS_ASSERT_THROWS(testee.store(out, aux, cs, ctx), interpreter::Error);
}

/** Test makeKeymapValue(). */
void
TestInterpreterKeymapValue::testMake()
{
    std::auto_ptr<interpreter::KeymapValue> p;

    // Create normal keymap
    util::Keymap k("ZZ");
    p.reset(interpreter::makeKeymapValue(&k));
    TS_ASSERT(p.get() != 0);
    TS_ASSERT_EQUALS(p->getKeymap(), &k);

    // Create null
    p.reset(interpreter::makeKeymapValue(0));
    TS_ASSERT(p.get() == 0);
}

