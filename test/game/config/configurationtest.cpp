/**
  *  \file test/game/config/configurationtest.cpp
  *  \brief Test for game::config::Configuration
  */

#include "game/config/configuration.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"

using afl::base::Ref;
using game::config::Configuration;

/** Test index-to-create. */
AFL_TEST("game.config.Configuration:index", a)
{
    game::config::IntegerValueParser vp;
    const game::config::IntegerOptionDescriptor one = { "one", &vp };
    const game::config::IntegerOptionDescriptor two = { "two", &vp };
    Ref<Configuration> rfig = Configuration::create();
    Configuration& fig = *rfig;

    // Give option an initial value
    fig.setOption("one", "99", game::config::ConfigurationOption::Default);

    // Accessing as integer will change the type
    a.checkEqual("01. index one", fig[one](), 99);

    // Initial access to unset option will create it with the right type and default value
    a.checkEqual("11. index two", fig[two](), 0);
    fig[two].set(33);
    a.checkEqual("12. index two", fig[two](), 33);
}

/** Test accessing an option. */
AFL_TEST("game.config.Configuration:getOptionByName", a)
{
    Ref<Configuration> rtestee = Configuration::create();
    Configuration& testee = *rtestee;
    game::config::ConfigurationOption* opt = testee.getOptionByName("someoption");
    a.checkNull("01. getOptionByName", opt);

    testee.setOption("SomeOption", "somevalue", game::config::ConfigurationOption::Game);
    opt = testee.getOptionByName("someoption");
    a.checkNonNull("11. getOptionByName", opt);
    a.check("12. toString", opt->toString() == "somevalue");
}

/** Test enumeration. */
AFL_TEST("game.config.Configuration:getOptions", a)
{
    game::config::IntegerValueParser vp;
    const game::config::IntegerOptionDescriptor one = { "one", &vp };
    const game::config::IntegerOptionDescriptor two = { "two", &vp };
    Ref<Configuration> rtestee = Configuration::create();
    Configuration& testee = *rtestee;

    // Set first option
    testee[one].set(1);

    // Start enumeration
    Ref<Configuration::Enumerator_t> e(testee.getOptions());

    // Verify first element
    Configuration::OptionInfo_t info;
    bool ok = e->getNextElement(info);
    a.check("01. getNextElement", ok);
    a.checkEqual("02. first", info.first, "one");
    a.checkEqual("03. second", info.second, &testee[one]);

    // Access second element. We cannot say what this does to the enumeration, but it should not crash it.
    testee[two].set(3);
    e->getNextElement(info);
}

/** Test merge. */
AFL_TEST("game.config.Configuration:merge", a)
{
    using game::config::ConfigurationOption;

    game::config::IntegerValueParser vp;
    const game::config::IntegerOptionDescriptor one = { "one", &vp };
    const game::config::IntegerOptionDescriptor three = { "three", &vp };

    // Make configuration a
    Ref<Configuration> rca = Configuration::create();
    Configuration& ca = *rca;
    ca[one].set(1);
    ca[one].setSource(ConfigurationOption::User);
    ca.setOption("two", "2", ConfigurationOption::Game);

    // Make configuration b
    Ref<Configuration> rcb = Configuration::create();
    Configuration& cb = *rcb;
    cb.setOption("one", "11", ConfigurationOption::System);
    cb.setOption("two", "22", ConfigurationOption::Default);
    cb[three].set(33);
    cb[three].setSource(ConfigurationOption::User);

    // Merge
    ca.merge(cb);

    // Verify
    ConfigurationOption* p1 = ca.getOptionByName("one");
    a.checkNonNull("01. getOptionByName one", p1);
    a.checkEqual("02. toString", p1->toString(), "11");
    a.checkEqual("03. getSource", p1->getSource(), ConfigurationOption::User);

    ConfigurationOption* p2 = ca.getOptionByName("two");
    a.checkNonNull("11. getOptionByName two", p2);
    a.checkEqual("12. toString", p2->toString(), "2");
    a.checkEqual("13. getSource", p2->getSource(), ConfigurationOption::Game);

    ConfigurationOption* p3 = ca.getOptionByName("three");
    a.checkNonNull("21. getOptionByName three", p3);
    a.checkEqual("22. toString", p3->toString(), "33");
    a.checkEqual("23. getSource", p3->getSource(), ConfigurationOption::User);
}

/** Test subtract. */
AFL_TEST("game.config.Configuration:subtract", a)
{
    using game::config::ConfigurationOption;

    // Make configuration a
    Ref<Configuration> ca = Configuration::create();
    ca->setOption("one", "1a", ConfigurationOption::User);
    ca->setOption("two", "2a", ConfigurationOption::User);
    ca->setOption("three", "3a", ConfigurationOption::User);

    // Make configuration b
    Ref<Configuration> cb = Configuration::create();
    cb->setOption("One", "1a", ConfigurationOption::User);
    cb->setOption("two", "2b", ConfigurationOption::User);

    // Merge
    ca->subtract(*cb);

    // Verify
    ConfigurationOption* p1 = ca->getOptionByName("one");
    a.checkNonNull("01. getOptionByName one", p1);
    a.checkEqual("02. toString", p1->toString(), "1a");
    a.checkEqual("03. getSource", p1->getSource(), ConfigurationOption::Default);

    ConfigurationOption* p2 = ca->getOptionByName("two");
    a.checkNonNull("11. getOptionByName two", p2);
    a.checkEqual("12. toString", p2->toString(), "2a");
    a.checkEqual("13. getSource", p2->getSource(), ConfigurationOption::User);

    ConfigurationOption* p3 = ca->getOptionByName("three");
    a.checkNonNull("21. getOptionByName three", p3);
    a.checkEqual("22. toString", p3->toString(), "3a");
    a.checkEqual("23. getSource", p3->getSource(), ConfigurationOption::User);
}
