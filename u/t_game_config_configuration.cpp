/**
  *  \file u/t_game_config_configuration.cpp
  */

#include "u/t_game_config.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/configuration.hpp"

/** Test index-to-create. */
void
TestGameConfigConfiguration::testIndexing()
{
    game::config::IntegerValueParser vp;
    const game::config::IntegerOptionDescriptor one = { "one", &vp };
    const game::config::IntegerOptionDescriptor two = { "two", &vp };
    game::config::Configuration fig;

    // Give option an initial value
    fig.setOption("one", "99", game::config::ConfigurationOption::Default);

    // Accessing as integer will change the type
    TS_ASSERT_EQUALS(fig[one](), 99);

    // Initial access to unset option will create it with the right type and default value
    TS_ASSERT_EQUALS(fig[two](), 0);
    fig[two].set(33);
    TS_ASSERT_EQUALS(fig[two](), 33);
}

/** Test accessing an option. */
void
TestGameConfigConfiguration::testAccess()
{
    game::config::Configuration testee;
    game::config::ConfigurationOption* opt = testee.getOptionByName("someoption");
    TS_ASSERT(!opt);

    testee.setOption("SomeOption", "somevalue", game::config::ConfigurationOption::Game);
    opt = testee.getOptionByName("someoption");
    TS_ASSERT(opt != 0);
    TS_ASSERT(opt->toString() == "somevalue");
}

/** Test enumeration. */
void
TestGameConfigConfiguration::testEnum()
{
    game::config::IntegerValueParser vp;
    const game::config::IntegerOptionDescriptor one = { "one", &vp };
    const game::config::IntegerOptionDescriptor two = { "two", &vp };
    game::config::Configuration testee;

    // Set first option
    testee[one].set(1);

    // Start enumeration
    afl::base::Ptr<game::config::Configuration::Enumerator_t> e(testee.getOptions());
    TS_ASSERT(e.get() != 0);

    // Verify first element
    game::config::Configuration::OptionInfo_t info;
    bool ok = e->getNextElement(info);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(info.first, "one");
    TS_ASSERT_EQUALS(info.second, &testee[one]);

    // Access second element. We cannot say what this does to the enumeration, but it should not crash it.
    testee[two].set(3);
    e->getNextElement(info);
}
