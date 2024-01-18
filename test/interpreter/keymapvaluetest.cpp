/**
  *  \file test/interpreter/keymapvaluetest.cpp
  *  \brief Test for interpreter::KeymapValue
  */

#include "interpreter/keymapvalue.hpp"

#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "util/keymap.hpp"
#include <memory>

/** Test KeymapValue. */
AFL_TEST("interpreter.KeymapValue:basics", a)
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
    a.checkEqual("01. getKeymap", testee.getKeymap(), pk);
    a.checkEqual("02. toString", testee.toString(false), testee.toString(true));
    a.checkEqual("03. toString", testee.toString(false).substr(0, 2), "#<");

    std::auto_ptr<interpreter::KeymapValue> copy(testee.clone());
    a.checkNonNull("11. clone", copy.get());
    a.checkEqual("12. getKeymap", copy->getKeymap(), pk);
    a.checkEqual("13. toString", copy->toString(false), testee.toString(false));

    // Store
    interpreter::test::ValueVerifier(testee, a).verifyNotSerializable();
}

/** Test makeKeymapValue(). */
AFL_TEST("interpreter.KeymapValue:makeKeymapValue", a)
{
    std::auto_ptr<interpreter::KeymapValue> p;

    // Create normal keymap
    util::Keymap k("ZZ");
    p.reset(interpreter::makeKeymapValue(&k));
    a.checkNonNull("01. makeKeymapValue", p.get());
    a.checkEqual("02. getKeymap", p->getKeymap(), &k);

    // Create null
    p.reset(interpreter::makeKeymapValue(0));
    a.checkNull("11. makeKeymapValue", p.get());
}
