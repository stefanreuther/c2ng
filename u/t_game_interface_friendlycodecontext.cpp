/**
  *  \file u/t_game_interface_friendlycodecontext.cpp
  *  \brief Test for game::interface::FriendlyCodeContext
  */

#include "game/interface/friendlycodecontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "afl/charset/utf8charset.hpp"

/** Test FriendlyCodeContext. */
void
TestGameInterfaceFriendlyCodeContext::testIt()
{
    afl::string::NullTranslator tx;

    // Create a root (FIXME: what for?)
    afl::base::Ref<game::Root> root(*new game::Root(afl::io::InternalDirectory::create("gameDir"),
                                                    *new game::test::SpecificationLoader(),
                                                    game::HostVersion(),
                                                    std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Registered, 9)),
                                                    std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                                    std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                                    game::Root::Actions_t()));

    // Create a ship list
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Add a mission
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("cln", "sr-57,Clone ship", tx));
    TS_ASSERT_EQUALS(shipList->friendlyCodes().size(), 1U);

    // Test
    game::interface::FriendlyCodeContext testee(0, root, shipList, tx);
    interpreter::test::ContextVerifier v(testee, "testIt");
    v.verifyTypes();
    v.verifyBasics();

    // Verify individual properties
    v.verifyString("NAME", "cln");
    v.verifyString("DESCRIPTION", "Clone ship");
    v.verifyString("FLAGS", "sr");
    v.verifyInteger("RACES$", -1 - (1 << 5) - (1 << 7));
}
