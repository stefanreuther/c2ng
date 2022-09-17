/**
  *  \file u/t_game_config_configurationeditor.cpp
  *  \brief Test for game::config::ConfigurationEditor
  */

#include "game/config/configurationeditor.hpp"

#include "t_game_config.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/translator.hpp"
#include "game/config/booleanvalueparser.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/stringoption.hpp"
#include "game/config/aliasoption.hpp"

using game::config::AliasOptionDescriptor;
using game::config::BooleanValueParser;
using game::config::ConfigurationEditor;
using game::config::ConfigurationOption;
using game::config::IntegerOptionDescriptor;
using game::config::IntegerValueParser;

/** General functionality test.
    A: create a ConfigurationEditor. Add some options. Verify content.
    E: correct content reported */
void
TestGameConfigConfigurationEditor::testIt()
{
    // A Configuration for testing
    static const IntegerOptionDescriptor intOption  = { "int",  &IntegerValueParser::instance };
    static const IntegerOptionDescriptor boolOption = { "bool", &BooleanValueParser::instance };
    static const game::config::StringOptionDescriptor stringOption = { "string" };

    // A ConfigurationEditor
    ConfigurationEditor ed;
    ed.addDivider(0, "divider 0");
    ed.addToggle(1, "int 1", intOption);
    ed.addToggle(1, "bool 2", boolOption);
    ed.addGeneric(1, "string 3", 77, "value 3")
        .addOption(stringOption);

    // Configuration Instance
    game::config::Configuration config;
    config[intOption].set(20);
    config[boolOption].set(1);
    config[stringOption].set("fred");
    config[stringOption].setSource(ConfigurationOption::User);

    afl::test::Translator tx("<", ">");

    // Access
    TS_ASSERT_EQUALS(ed.getNumNodes(), 4U);

    TS_ASSERT(ed.getNodeByIndex(0) != 0);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getValue(config, tx), "");
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getLevel(), 0);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getName(), "divider 0");
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getType(), ConfigurationEditor::NoEditor);
    TS_ASSERT(ed.getNodeByIndex(0)->getFirstOption(config) == 0);

    TS_ASSERT(ed.getNodeByIndex(1) != 0);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(1)->getValue(config, tx), "20");
    TS_ASSERT_EQUALS(ed.getNodeByIndex(1)->getLevel(), 1);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(1)->getName(), "int 1");
    TS_ASSERT_EQUALS(ed.getNodeByIndex(1)->getType(), ConfigurationEditor::ToggleEditor);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(1)->getFirstOption(config), &config[intOption]);

    TS_ASSERT(ed.getNodeByIndex(2) != 0);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(2)->getValue(config, tx), "<Yes>");
    TS_ASSERT_EQUALS(ed.getNodeByIndex(2)->getLevel(), 1);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(2)->getName(), "bool 2");
    TS_ASSERT_EQUALS(ed.getNodeByIndex(2)->getType(), ConfigurationEditor::ToggleEditor);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(2)->getFirstOption(config), &config[boolOption]);

    TS_ASSERT(ed.getNodeByIndex(3) != 0);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(3)->getValue(config, tx), "value 3");
    TS_ASSERT_EQUALS(ed.getNodeByIndex(3)->getLevel(), 1);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(3)->getName(), "string 3");
    TS_ASSERT_EQUALS(ed.getNodeByIndex(3)->getType(), 77);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(3)->getFirstOption(config), &config[stringOption]);

    ConfigurationEditor::Info i = ed.getNodeByIndex(3)->describe(config, tx);
    TS_ASSERT_EQUALS(i.level,   1);
    TS_ASSERT_EQUALS(i.type,    77);
    TS_ASSERT_EQUALS(i.source,  ConfigurationEditor::User);
    TS_ASSERT_EQUALS(i.name,    "string 3");
    TS_ASSERT_EQUALS(i.value,   "value 3");
}

/** Test toggleValue().
    A: Create a ConfigurationEditor and a boolean option. Call toggleValue().
    E: Value changes as expected. */
void
TestGameConfigConfigurationEditor::testToggle()
{
    // Environment: a bool option
    static const IntegerOptionDescriptor boolOption = { "bool", &BooleanValueParser::instance };
    game::config::Configuration config;
    config[boolOption].set(1);

    ConfigurationEditor ed;
    ed.addToggle(0, "bool", boolOption);

    // Action
    TS_ASSERT(ed.getNodeByIndex(0) != 0);
    ed.getNodeByIndex(0)->toggleValue(config);

    // Verify result
    TS_ASSERT_EQUALS(config[boolOption](), 0);
    TS_ASSERT_EQUALS(config[boolOption].getSource(), ConfigurationOption::User);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::User);
}

/** Test setValue().
    A: Create a ConfigurationEditor and a generic option. Call setValue().
    E: Value of first option changes as expected. */
void
TestGameConfigConfigurationEditor::testSetValue()
{
    // Environment: an integer option
    static const IntegerOptionDescriptor intOption   = { "int",   &IntegerValueParser::instance };
    static const IntegerOptionDescriptor otherOption = { "other", &IntegerValueParser::instance };
    game::config::Configuration config;
    config[intOption].set(7);
    config[otherOption].set(3);

    ConfigurationEditor ed;
    ed.addGeneric(0, "gen", 77, "value")
        .addOption(intOption);

    // Action
    TS_ASSERT(ed.getNodeByIndex(0) != 0);
    ed.getNodeByIndex(0)->setValue(config, "9");

    // Verify result
    TS_ASSERT_EQUALS(config[intOption](), 9);
    TS_ASSERT_EQUALS(config[intOption].getSource(), ConfigurationOption::User);
    TS_ASSERT_EQUALS(config[otherOption](), 3);  // not affected
}

/** Test getSource() for single option.
    A: create a single option. Call getSource(), setSource().
    E: Correct value reported: same in option and ConfigurationEditor */
void
TestGameConfigConfigurationEditor::testSourceSingle()
{
    // Environment: a bool option
    static const IntegerOptionDescriptor boolOption = { "bool", &BooleanValueParser::instance };
    game::config::Configuration config;
    config[boolOption].set(1);
    config[boolOption].setSource(ConfigurationOption::System);

    ConfigurationEditor ed;
    ed.addToggle(0, "bool", boolOption);

    // Check
    TS_ASSERT(ed.getNodeByIndex(0) != 0);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::System);

    // Modify
    ed.getNodeByIndex(0)->setSource(config, ConfigurationOption::User);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::User);
    TS_ASSERT_EQUALS(config[boolOption].getSource(), ConfigurationOption::User);
}

/** Test getSource() for empty node.
    A: create a divider node. Call getSource(), setSource().
    E: Value NotStored reported, not changeable */
void
TestGameConfigConfigurationEditor::testSourceEmpty()
{
    // Environment: a divider
    game::config::Configuration config;
    ConfigurationEditor ed;
    ed.addDivider(0, "divi");

    // Check
    TS_ASSERT(ed.getNodeByIndex(0) != 0);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::NotStored);

    // Modify - has no effect
    ed.getNodeByIndex(0)->setSource(config, ConfigurationOption::User);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::NotStored);
}

/** Test getSource() for multiple options.
    A: create a generic option with multiple options. Call getSource(), setSource().
    E: Correct value reported: "Mixed" if appropriate, otherwise same in option and ConfigurationEditor */
void
TestGameConfigConfigurationEditor::testSourceMulti()
{
    // Environment: two bool options with different locations in one node
    static const IntegerOptionDescriptor boolOption  = { "bool",  &BooleanValueParser::instance };
    static const IntegerOptionDescriptor otherOption = { "other", &BooleanValueParser::instance };
    game::config::Configuration config;
    config[boolOption].set(1);
    config[boolOption].setSource(ConfigurationOption::System);
    config[otherOption].set(1);
    config[otherOption].setSource(ConfigurationOption::Game);

    ConfigurationEditor ed;
    ed.addGeneric(0, "multi", 1, "value")
        .addOption(boolOption)
        .addOption(otherOption);

    // Check
    TS_ASSERT(ed.getNodeByIndex(0) != 0);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::Mixed);

    // Modify
    ed.getNodeByIndex(0)->setSource(config, ConfigurationOption::User);
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::User);
    TS_ASSERT_EQUALS(config[boolOption].getSource(), ConfigurationOption::User);
    TS_ASSERT_EQUALS(config[otherOption].getSource(), ConfigurationOption::User);
}

/** Test change notification.
    A: create a ConfigurationEditor. Use loadValues(), updateValues() sequence. Modify properties of options.
    E: change correctly reported for value and source changes */
void
TestGameConfigConfigurationEditor::testChange()
{
    // Environment: two bool options
    static const IntegerOptionDescriptor boolOption  = { "bool",  &BooleanValueParser::instance };
    static const IntegerOptionDescriptor otherOption = { "other", &BooleanValueParser::instance };
    game::config::Configuration config;
    config[boolOption].set(1);
    config[boolOption].setSource(ConfigurationOption::Game);
    config[otherOption].set(1);
    config[otherOption].setSource(ConfigurationOption::Game);

    ConfigurationEditor ed;
    ed.addToggle(0, "a", boolOption);
    ed.addToggle(0, "b", otherOption);

    // Listener
    std::vector<size_t> responses;
    class Pusher : public afl::base::Closure<void(size_t)> {
     public:
        Pusher(std::vector<size_t>& vec)
            : m_vector(vec)
            { }
        virtual void call(size_t n)
            { m_vector.push_back(n); }
     private:
        std::vector<size_t>& m_vector;
    };
    ed.sig_change.addNewClosure(new Pusher(responses));
    responses.reserve(20);

    // Initialize
    afl::string::NullTranslator tx;
    ed.loadValues(config, tx);
    TS_ASSERT_EQUALS(responses.size(), 0U);

    // Check for changes - still no change
    ed.updateValues(config, tx);
    TS_ASSERT_EQUALS(responses.size(), 0U);

    // Modify otherOption value
    config[otherOption].set(0);
    ed.updateValues(config, tx);
    TS_ASSERT_EQUALS(responses.size(), 1U);
    TS_ASSERT_EQUALS(responses[0], 1U);

    // Modify boolOption location
    config[boolOption].setSource(ConfigurationOption::User);
    ed.updateValues(config, tx);
    TS_ASSERT_EQUALS(responses.size(), 2U);
    TS_ASSERT_EQUALS(responses[1], 0U);
}

/** Test alias handling.
    A: create a ConfigurationEditor and some alias options. Check operations.
    E: change correctly reported for value and source changes */
void
TestGameConfigConfigurationEditor::testAlias()
{
    static const IntegerOptionDescriptor boolOption = { "bool",  &BooleanValueParser::instance };
    static const AliasOptionDescriptor a1 = { "a1", "bool" };
    static const AliasOptionDescriptor a2 = { "a2", "deadlink" };
    game::config::Configuration config;
    config[boolOption].set(0);
    config[boolOption].setSource(ConfigurationOption::User);
    config[a1].setSource(ConfigurationOption::System);
    config[a2].setSource(ConfigurationOption::System);

    ConfigurationEditor ed;
    ed.addGeneric(0, "1", 77, "v1").addOption(a1); // cannot use addToggle here
    ed.addGeneric(0, "2", 77, "v2").addOption(a2);

    // Verify state
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::User);    // property of forwarded option
    TS_ASSERT_EQUALS(ed.getNodeByIndex(1)->getSource(config), ConfigurationEditor::System);  // property of dead link
    TS_ASSERT(ed.getNodeByIndex(0)->getFirstOption(config) == &config[boolOption]);
    TS_ASSERT(ed.getNodeByIndex(1)->getFirstOption(config) == &config[a2]);

    // Update
    ed.getNodeByIndex(0)->setSource(config, ConfigurationOption::Game);
    ed.getNodeByIndex(1)->setSource(config, ConfigurationOption::Game);
    ed.getNodeByIndex(0)->toggleValue(config);
    ed.getNodeByIndex(1)->toggleValue(config);

    // Verify state
    TS_ASSERT_EQUALS(ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::Game);    // property of forwarded option
    TS_ASSERT_EQUALS(ed.getNodeByIndex(1)->getSource(config), ConfigurationEditor::Game);    // property of dead link
    TS_ASSERT_EQUALS(config[boolOption](), 1);
}

/** Test addAll(). */
void
TestGameConfigConfigurationEditor::testAddAll()
{
    afl::string::NullTranslator tx;
    const int TYPE = 77;

    static const IntegerOptionDescriptor opt1  = { "v1",  &IntegerValueParser::instance };
    static const IntegerOptionDescriptor opt2  = { "v2",  &IntegerValueParser::instance };
    static const IntegerOptionDescriptor opt3  = { "v3",  &IntegerValueParser::instance };
    game::config::Configuration config;
    config[opt1].set(42);
    config[opt2].set(23);
    config[opt3].set(69);

    ConfigurationEditor ed;
    ed.addAll(0, TYPE, config);

    // Verify
    TS_ASSERT_EQUALS(ed.getNumNodes(), 3U);

    ConfigurationEditor::Node* n1 = ed.getNodeByIndex(0);
    TS_ASSERT(n1 != 0);
    TS_ASSERT_EQUALS(n1->getName(), "v1");
    TS_ASSERT_EQUALS(n1->getValue(config, tx), "42");
    TS_ASSERT_EQUALS(n1->getType(), TYPE);
    TS_ASSERT_EQUALS(n1->getFirstOption(config), &config[opt1]);

    ConfigurationEditor::Node* n2 = ed.getNodeByIndex(1);
    TS_ASSERT(n2 != 0);
    TS_ASSERT_EQUALS(n2->getName(), "v2");
    TS_ASSERT_EQUALS(n2->getValue(config, tx), "23");
    TS_ASSERT_EQUALS(n2->getType(), TYPE);
    TS_ASSERT_EQUALS(n2->getFirstOption(config), &config[opt2]);

    ConfigurationEditor::Node* n3 = ed.getNodeByIndex(2);
    TS_ASSERT(n3 != 0);
    TS_ASSERT_EQUALS(n3->getName(), "v3");
    TS_ASSERT_EQUALS(n3->getValue(config, tx), "69");
    TS_ASSERT_EQUALS(n3->getType(), TYPE);
    TS_ASSERT_EQUALS(n3->getFirstOption(config), &config[opt3]);

    // Apply the editor to a different config
    game::config::Configuration config2;
    config2[opt1].set(17);

    TS_ASSERT_EQUALS(n1->getValue(config2, tx), "17");
    TS_ASSERT_EQUALS(n2->getValue(config2, tx), "");
    TS_ASSERT_EQUALS(n3->getValue(config2, tx), "");

    TS_ASSERT_EQUALS(n1->getFirstOption(config2), &config2[opt1]);
    TS_ASSERT(n2->getFirstOption(config2) == 0);
    TS_ASSERT(n3->getFirstOption(config2) == 0);
}

