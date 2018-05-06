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
    afl::base::Ref<game::config::Configuration::Enumerator_t> e(testee.getOptions());

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

/** Test merge. */
void
TestGameConfigConfiguration::testMerge()
{
    using game::config::ConfigurationOption;

    game::config::IntegerValueParser vp;
    const game::config::IntegerOptionDescriptor one = { "one", &vp };
    const game::config::IntegerOptionDescriptor three = { "three", &vp };

    // Make configuration a
    game::config::Configuration a;
    a[one].set(1);
    a[one].setSource(ConfigurationOption::User);
    a.setOption("two", "2", ConfigurationOption::Game);

    // Make configuration b
    game::config::Configuration b;
    b.setOption("one", "11", ConfigurationOption::System);
    b.setOption("two", "22", ConfigurationOption::Default);
    b[three].set(33);
    b[three].setSource(ConfigurationOption::User);

    // Merge
    a.merge(b);

    // Verify
    ConfigurationOption* p1 = a.getOptionByName("one");
    TS_ASSERT(p1 != 0);
    TS_ASSERT_EQUALS(p1->toString(), "11");
    TS_ASSERT_EQUALS(p1->getSource(), ConfigurationOption::User);

    ConfigurationOption* p2 = a.getOptionByName("two");
    TS_ASSERT(p2 != 0);
    TS_ASSERT_EQUALS(p2->toString(), "2");
    TS_ASSERT_EQUALS(p2->getSource(), ConfigurationOption::Game);

    ConfigurationOption* p3 = a.getOptionByName("three");
    TS_ASSERT(p3 != 0);
    TS_ASSERT_EQUALS(p3->toString(), "33");
    TS_ASSERT_EQUALS(p3->getSource(), ConfigurationOption::User);
}

