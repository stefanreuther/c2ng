/**
  *  \file test/server/play/configurationpackertest.cpp
  *  \brief Test for server::play::ConfigurationPacker
  */

#include "server/play/configurationpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/test/root.hpp"
#include <memory>

namespace {
    std::auto_ptr<server::Value_t> fetchSlice(afl::test::Assert a, int n, String_t name)
    {
        // Populate a root
        afl::base::Ref<game::Root> r(game::test::makeRoot(game::HostVersion()));
        game::config::HostConfiguration& config = r->hostConfiguration();
        config.setOption("gamename", "ConfigPackerTest", game::config::ConfigurationOption::Game);
        config.setOption("maximumfightersonbase", "30", game::config::ConfigurationOption::Game);
        config.setOption("strikesperfighter", "12",     game::config::ConfigurationOption::Game);
        config.setOption("terraformrate", "3,4,5",      game::config::ConfigurationOption::Game);
        config.setOption("experiencelevelnames", "Noob,Nieswurz,Brotfahrer,Ladehugo,Erdwurm", game::config::ConfigurationOption::Game);

        // Produce value
        server::play::ConfigurationPacker testee(*r, n);
        a(name).checkEqual("getName", testee.getName(), name);
        return std::auto_ptr<server::Value_t>(testee.buildValue());
    }
}

/** Basic test. Test the cfg0 (=everything) slice which PCC2 Web uses nowadays. */
AFL_TEST("server.play.ConfigurationPacker:basics", a)
{
    std::auto_ptr<server::Value_t> value(fetchSlice(a, 0, "cfg0"));
    afl::data::Access ap(value.get());

    // StringOption
    a.checkEqual("01", ap("GAMENAME").toString(), "ConfigPackerTest");

    // GenericIntegerArrayOption
    a.checkEqual("11", ap("TERRAFORMRATE")[0].toInteger(), 3);
    a.checkEqual("12", ap("TERRAFORMRATE")[1].toInteger(), 4);
    a.checkEqual("13", ap("TERRAFORMRATE")[10].toInteger(), 5);

    // CostArrayOption
    a.checkEqual("21", ap("STARBASECOST")[0]("T").toInteger(), 402);

    // IntegerOption
    a.checkEqual("31", ap("CPENABLEALLIES").toInteger(), 1);

    // StringArrayOption
    a.checkEqual("41", ap("EXPERIENCELEVELNAMES").toString(), "Noob,Nieswurz,Brotfahrer,Ladehugo,Erdwurm");

    afl::data::StringList_t list;
    ap.getHashKeys(list);
    a.check("41. size", list.size() > 100U);
}

/** Test the other slices. */
AFL_TEST("server.play.ConfigurationPacker:slices", a)
{
    std::auto_ptr<server::Value_t> planetSlice(fetchSlice(a, 1, "cfg1"));
    std::auto_ptr<server::Value_t> combatSlice(fetchSlice(a, 2, "cfg2"));
    std::auto_ptr<server::Value_t> baseSlice(fetchSlice(a, 3, "cfg3"));

    afl::data::Access planet(planetSlice.get());
    afl::data::Access combat(combatSlice.get());
    afl::data::Access base(baseSlice.get());

    a.checkEqual("01", planet("TERRAFORMRATE")[0].toInteger(), 3);
    a.checkEqual("02", combat("STRIKESPERFIGHTER")[0].toInteger(), 12);
    a.checkEqual("03", base("MAXIMUMFIGHTERSONBASE")[0].toInteger(), 30);
}
