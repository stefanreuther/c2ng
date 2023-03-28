/**
  *  \file u/t_game_interface_richtextvalue.cpp
  *  \brief Test for game::interface::RichTextValue
  */

#include <memory>
#include "game/interface/richtextvalue.hpp"

#include "t_game_interface.hpp"
#include "interpreter/test/valueverifier.hpp"

void
TestGameInterfaceRichTextValue::testIt()
{
    game::interface::RichTextValue testee(*new util::rich::Text("hello"));

    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    TS_ASSERT_EQUALS(testee.toString(false), "hello");
    TS_ASSERT_EQUALS(testee.get()->getText(), "hello");

    std::auto_ptr<game::interface::RichTextValue> clone(testee.clone());
    TS_ASSERT_EQUALS(&*clone->get(), &*testee.get());
}

