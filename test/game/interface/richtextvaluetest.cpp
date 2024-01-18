/**
  *  \file test/game/interface/richtextvaluetest.cpp
  *  \brief Test for game::interface::RichTextValue
  */

#include "game/interface/richtextvalue.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/test/valueverifier.hpp"
#include <memory>

AFL_TEST("game.interface.RichTextValue", a)
{
    game::interface::RichTextValue testee(*new util::rich::Text("hello"));

    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. toString", testee.toString(false), "hello");
    a.checkEqual("02. getText", testee.get()->getText(), "hello");

    std::auto_ptr<game::interface::RichTextValue> clone(testee.clone());
    a.checkEqual("11. clone", &*clone->get(), &*testee.get());
}
