/**
  *  \file u/t_game_interface_friendlycodecontext.cpp
  *  \brief Test for game::interface::FriendlyCodeContext
  */

#include "game/interface/friendlycodecontext.hpp"

#include "t_game_interface.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/stringverifier.hpp"
#include "afl/io/internaldirectory.hpp"
#include "u/helper/contextverifier.hpp"

/** Test FriendlyCodeContext. */
void
TestGameInterfaceFriendlyCodeContext::testIt()
{
    // Create a root (FIXME: what for?)
    afl::charset::Utf8Charset charset;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::base::Ref<game::Root> root(*new game::Root(afl::io::InternalDirectory::create("specDir"),
                                                    afl::io::InternalDirectory::create("gameDir"),
                                                    *new game::v3::SpecificationLoader(charset, tx, log),
                                                    game::HostVersion(),
                                                    std::auto_ptr<game::RegistrationKey>(new game::v3::RegistrationKey(charset)),
                                                    std::auto_ptr<game::StringVerifier>(new game::v3::StringVerifier(std::auto_ptr<afl::charset::Charset>(charset.clone())))));

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
