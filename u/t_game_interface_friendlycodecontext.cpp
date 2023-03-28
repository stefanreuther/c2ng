/**
  *  \file u/t_game_interface_friendlycodecontext.cpp
  *  \brief Test for game::interface::FriendlyCodeContext
  */

#include "game/interface/friendlycodecontext.hpp"

#include "t_game_interface.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Test FriendlyCodeContext. */
void
TestGameInterfaceFriendlyCodeContext::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Add a friendly code
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("cln", "sr-57,Clone ship", tx));
    TS_ASSERT_EQUALS(shipList->friendlyCodes().size(), 1U);

    // Test
    game::interface::FriendlyCodeContext testee(0, root, shipList, tx);
    interpreter::test::ContextVerifier v(testee, "testIt");
    v.verifyTypes();
    v.verifyBasics();
    v.verifyNotSerializable();
    TS_ASSERT(testee.getObject() == 0);

    // Verify individual properties
    v.verifyString("NAME", "cln");
    v.verifyString("DESCRIPTION", "Clone ship");
    v.verifyString("FLAGS", "sr");
    v.verifyInteger("RACES$", -1 - (1 << 5) - (1 << 7));
}

/** Test enumeration. */
void
TestGameInterfaceFriendlyCodeContext::testEnum()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Add a friendly code
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("a", "s,first", tx));
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("b", "s,second", tx));
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("c", "s,third", tx));
    TS_ASSERT_EQUALS(shipList->friendlyCodes().size(), 3U);

    // Verify
    game::interface::FriendlyCodeContext testee(1, root, shipList, tx);
    interpreter::test::ContextVerifier v(testee, "testEnum");
    v.verifyString("NAME", "b");
    TS_ASSERT(testee.next());
    v.verifyString("NAME", "c");
    TS_ASSERT(!testee.next());
}

/** Test error case: index out of range (does not happen normally). */
void
TestGameInterfaceFriendlyCodeContext::testRange()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Verify
    game::interface::FriendlyCodeContext testee(10, root, shipList, tx);
    interpreter::test::ContextVerifier v(testee, "testRange");
    v.verifyNull("NAME");
}
