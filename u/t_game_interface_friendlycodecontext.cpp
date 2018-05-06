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
#include "u/helper/contextverifier.hpp"

/** Test FriendlyCodeContext. */
void
TestGameInterfaceFriendlyCodeContext::testIt()
{
    // Create a root (FIXME: what for?)
    afl::base::Ref<game::Root> root(*new game::Root(afl::io::InternalDirectory::create("gameDir"),
                                                    *new game::test::SpecificationLoader(),
                                                    game::HostVersion(),
                                                    std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Registered, 9)),
                                                    std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                                    game::Root::Actions_t()));

    // Create a ship list
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Add a mission
    shipList->friendlyCodes().addCode(game::spec::FriendlyCode("cln", "sr-57,Clone ship"));
    TS_ASSERT_EQUALS(shipList->friendlyCodes().size(), 1U);

    // Test
    game::interface::FriendlyCodeContext testee(0, root, shipList);
    verifyTypes(testee);

    // Verify individual properties
    verifyString(testee, "NAME", "cln");
    verifyString(testee, "DESCRIPTION", "Clone ship");
    verifyString(testee, "FLAGS", "sr");
    verifyInteger(testee, "RACES$", -1 - (1 << 5) - (1 << 7));
}
