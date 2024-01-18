/**
  *  \file test/game/interface/friendlycodecontexttest.cpp
  *  \brief Test for game::interface::FriendlyCodeContext
  */

#include "game/interface/friendlycodecontext.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Test FriendlyCodeContext. */
AFL_TEST("game.interface.FriendlyCodeContext:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Add a friendly code
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("cln", "sr-57,Clone ship", tx));
    a.checkEqual("01. size", shipList->friendlyCodes().size(), 1U);

    // Test
    game::interface::FriendlyCodeContext testee(0, root, shipList, tx);
    interpreter::test::ContextVerifier v(testee, a);
    v.verifyTypes();
    v.verifyBasics();
    v.verifyNotSerializable();
    a.checkNull("11. getObject", testee.getObject());

    // Verify individual properties
    v.verifyString("NAME", "cln");
    v.verifyString("DESCRIPTION", "Clone ship");
    v.verifyString("FLAGS", "sr");
    v.verifyInteger("RACES$", -1 - (1 << 5) - (1 << 7));
}

/** Test enumeration. */
AFL_TEST("game.interface.FriendlyCodeContext:enum", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Add a friendly code
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("a", "s,first", tx));
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("b", "s,second", tx));
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("c", "s,third", tx));
    a.checkEqual("01. size", shipList->friendlyCodes().size(), 3U);

    // Verify
    game::interface::FriendlyCodeContext testee(1, root, shipList, tx);
    interpreter::test::ContextVerifier v(testee, a);
    v.verifyString("NAME", "b");
    a.check("11. next", testee.next());
    v.verifyString("NAME", "c");
    a.check("12. next", !testee.next());
}

/** Test error case: index out of range (does not happen normally). */
AFL_TEST("game.interface.FriendlyCodeContext:null", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Verify
    game::interface::FriendlyCodeContext testee(10, root, shipList, tx);
    interpreter::test::ContextVerifier v(testee, a);
    v.verifyNull("NAME");
}
