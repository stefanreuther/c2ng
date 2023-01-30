/**
  *  \file u/t_server_play_configurationpacker.cpp
  *  \brief Test for server::play::ConfigurationPacker
  */

#include "server/play/configurationpacker.hpp"

#include <memory>
#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/config/hostconfiguration.hpp"

namespace {
    std::auto_ptr<server::Value_t> fetchSlice(int n, String_t name)
    {
        // Session
        afl::io::NullFileSystem fs;
        afl::string::NullTranslator tx;
        game::Session session(tx, fs);

        // Populate session
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        game::config::HostConfiguration& config = session.getRoot()->hostConfiguration();
        config.setOption("gamename", "ConfigPackerTest", game::config::ConfigurationOption::Game);
        config.setOption("maximumfightersonbase", "30", game::config::ConfigurationOption::Game);
        config.setOption("strikesperfighter", "12",     game::config::ConfigurationOption::Game);
        config.setOption("terraformrate", "3,4,5",      game::config::ConfigurationOption::Game);

        // Produce value
        server::play::ConfigurationPacker testee(session, n);
        TSM_ASSERT_EQUALS(name, testee.getName(), name);
        return std::auto_ptr<server::Value_t>(testee.buildValue());
    }
}

/** Basic test. Test the cfg0 (=everything) slice which PCC2 Web uses nowadays. */
void
TestServerPlayConfigurationPacker::testIt()
{
    std::auto_ptr<server::Value_t> value(fetchSlice(0, "cfg0"));
    afl::data::Access a(value.get());

    // StringOption
    TS_ASSERT_EQUALS(a("GAMENAME").toString(), "ConfigPackerTest");

    // GenericIntegerArrayOption
    TS_ASSERT_EQUALS(a("TERRAFORMRATE")[0].toInteger(), 3);
    TS_ASSERT_EQUALS(a("TERRAFORMRATE")[1].toInteger(), 4);
    TS_ASSERT_EQUALS(a("TERRAFORMRATE")[10].toInteger(), 5);

    // CostArrayOption
    TS_ASSERT_EQUALS(a("STARBASECOST")[0]("T").toInteger(), 402);

    // IntegerOption
    TS_ASSERT_EQUALS(a("CPENABLEALLIES").toInteger(), 1);

    afl::data::StringList_t list;
    a.getHashKeys(list);
    TS_ASSERT(list.size() > 100U);
}

/** Test the other slices. */
void
TestServerPlayConfigurationPacker::testSlices()
{
    std::auto_ptr<server::Value_t> planetSlice(fetchSlice(1, "cfg1"));
    std::auto_ptr<server::Value_t> combatSlice(fetchSlice(2, "cfg2"));
    std::auto_ptr<server::Value_t> baseSlice(fetchSlice(3, "cfg3"));

    afl::data::Access planet(planetSlice.get());
    afl::data::Access combat(combatSlice.get());
    afl::data::Access base(baseSlice.get());

    TS_ASSERT_EQUALS(planet("TERRAFORMRATE")[0].toInteger(), 3);
    TS_ASSERT_EQUALS(combat("STRIKESPERFIGHTER")[0].toInteger(), 12);
    TS_ASSERT_EQUALS(base("MAXIMUMFIGHTERSONBASE")[0].toInteger(), 30);
}

