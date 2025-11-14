/**
  *  \file test/game/config/configurationeditortest.cpp
  *  \brief Test for game::config::ConfigurationEditor
  */

#include "game/config/configurationeditor.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "afl/test/translator.hpp"
#include "game/config/aliasoption.hpp"
#include "game/config/booleanvalueparser.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/stringoption.hpp"

using game::config::AliasOptionDescriptor;
using game::config::BooleanValueParser;
using game::config::ConfigurationEditor;
using game::config::ConfigurationOption;
using game::config::IntegerOptionDescriptor;
using game::config::IntegerValueParser;

/** General functionality test.
    A: create a ConfigurationEditor. Add some options. Verify content.
    E: correct content reported */
AFL_TEST("game.config.ConfigurationEditor:basic", a)
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
    afl::base::Ref<game::config::Configuration> rconfig = game::config::Configuration::create();
    game::config::Configuration& config = *rconfig;
    config[intOption].set(20);
    config[boolOption].set(1);
    config[stringOption].set("fred");
    config[stringOption].setSource(ConfigurationOption::User);

    afl::test::Translator tx("<", ">");

    // Access
    a.checkEqual("01. getNumNodes", ed.getNumNodes(), 4U);

    a.checkNonNull("11. getNodeByIndex", ed.getNodeByIndex(0));
    a.checkEqual("12. getValue", ed.getNodeByIndex(0)->getValue(config, tx), "");
    a.checkEqual("13. getLevel", ed.getNodeByIndex(0)->getLevel(), 0);
    a.checkEqual("14. getName", ed.getNodeByIndex(0)->getName(), "divider 0");
    a.checkEqual("15. getType", ed.getNodeByIndex(0)->getType(), ConfigurationEditor::NoEditor);
    a.checkNull("16. getFirstOption", ed.getNodeByIndex(0)->getFirstOption(config));

    a.checkNonNull("21. getNodeByIndex", ed.getNodeByIndex(1));
    a.checkEqual("22. getValue", ed.getNodeByIndex(1)->getValue(config, tx), "20");
    a.checkEqual("23. getLevel", ed.getNodeByIndex(1)->getLevel(), 1);
    a.checkEqual("24. getName", ed.getNodeByIndex(1)->getName(), "int 1");
    a.checkEqual("25. getType", ed.getNodeByIndex(1)->getType(), ConfigurationEditor::ToggleEditor);
    a.checkEqual("26. getFirstOption", ed.getNodeByIndex(1)->getFirstOption(config), &config[intOption]);

    a.checkNonNull("31. getNodeByIndex", ed.getNodeByIndex(2));
    a.checkEqual("32. getValue", ed.getNodeByIndex(2)->getValue(config, tx), "<Yes>");
    a.checkEqual("33. getLevel", ed.getNodeByIndex(2)->getLevel(), 1);
    a.checkEqual("34. getName", ed.getNodeByIndex(2)->getName(), "bool 2");
    a.checkEqual("35. getType", ed.getNodeByIndex(2)->getType(), ConfigurationEditor::ToggleEditor);
    a.checkEqual("36. getFirstOption", ed.getNodeByIndex(2)->getFirstOption(config), &config[boolOption]);

    a.checkNonNull("41. getNodeByIndex", ed.getNodeByIndex(3));
    a.checkEqual("42. getValue", ed.getNodeByIndex(3)->getValue(config, tx), "value 3");
    a.checkEqual("43. getLevel", ed.getNodeByIndex(3)->getLevel(), 1);
    a.checkEqual("44. getName", ed.getNodeByIndex(3)->getName(), "string 3");
    a.checkEqual("45. getType", ed.getNodeByIndex(3)->getType(), 77);
    a.checkEqual("46. getFirstOption", ed.getNodeByIndex(3)->getFirstOption(config), &config[stringOption]);

    ConfigurationEditor::Info i = ed.getNodeByIndex(3)->describe(config, tx);
    a.checkEqual("51. level",  i.level,   1);
    a.checkEqual("52. type",   i.type,    77);
    a.checkEqual("53. source", i.source,  ConfigurationEditor::User);
    a.checkEqual("54. name",   i.name,    "string 3");
    a.checkEqual("55. value",  i.value,   "value 3");
}

/** Test toggleValue().
    A: Create a ConfigurationEditor and a boolean option. Call toggleValue().
    E: Value changes as expected. */
AFL_TEST("game.config.ConfigurationEditor:toggleValue", a)
{
    // Environment: a bool option
    static const IntegerOptionDescriptor boolOption = { "bool", &BooleanValueParser::instance };
    afl::base::Ref<game::config::Configuration> rconfig = game::config::Configuration::create();
    game::config::Configuration& config = *rconfig;
    config[boolOption].set(1);

    ConfigurationEditor ed;
    ed.addToggle(0, "bool", boolOption);

    // Action
    a.checkNonNull("01. getNodeByIndex", ed.getNodeByIndex(0));
    ed.getNodeByIndex(0)->toggleValue(config);

    // Verify result
    a.checkEqual("11. option value",  config[boolOption](), 0);
    a.checkEqual("12. option source", config[boolOption].getSource(), ConfigurationOption::User);
    a.checkEqual("13. getSource", ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::User);
}

/** Test setValue().
    A: Create a ConfigurationEditor and a generic option. Call setValue().
    E: Value of first option changes as expected. */
AFL_TEST("game.config.ConfigurationEditor:setValue", a)
{
    // Environment: an integer option
    static const IntegerOptionDescriptor intOption   = { "int",   &IntegerValueParser::instance };
    static const IntegerOptionDescriptor otherOption = { "other", &IntegerValueParser::instance };
    afl::base::Ref<game::config::Configuration> rconfig = game::config::Configuration::create();
    game::config::Configuration& config = *rconfig;
    config[intOption].set(7);
    config[otherOption].set(3);

    ConfigurationEditor ed;
    ed.addGeneric(0, "gen", 77, "value")
        .addOption(intOption);

    // Action
    a.checkNonNull("01. getNodeByIndex", ed.getNodeByIndex(0));
    ed.getNodeByIndex(0)->setValue(config, "9");

    // Verify result
    a.checkEqual("11. option value", config[intOption](), 9);
    a.checkEqual("12. option source", config[intOption].getSource(), ConfigurationOption::User);
    a.checkEqual("13. other value", config[otherOption](), 3);  // not affected
}

/** Test getSource() for single option.
    A: create a single option. Call getSource(), setSource().
    E: Correct value reported: same in option and ConfigurationEditor */
AFL_TEST("game.config.ConfigurationEditor:getSource:single", a)
{
    // Environment: a bool option
    static const IntegerOptionDescriptor boolOption = { "bool", &BooleanValueParser::instance };
    afl::base::Ref<game::config::Configuration> rconfig = game::config::Configuration::create();
    game::config::Configuration& config = *rconfig;
    config[boolOption].set(1);
    config[boolOption].setSource(ConfigurationOption::System);

    ConfigurationEditor ed;
    ed.addToggle(0, "bool", boolOption);

    // Check
    a.checkNonNull("01. getNodeByIndex", ed.getNodeByIndex(0));
    a.checkEqual("02. getSource", ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::System);

    // Modify
    ed.getNodeByIndex(0)->setSource(config, ConfigurationOption::User);
    a.checkEqual("11. getSource", ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::User);
    a.checkEqual("12. option source", config[boolOption].getSource(), ConfigurationOption::User);
}

/** Test getSource() for empty node.
    A: create a divider node. Call getSource(), setSource().
    E: Value NotStored reported, not changeable */
AFL_TEST("game.config.ConfigurationEditor:getSource:empty", a)
{
    // Environment: a divider
    afl::base::Ref<game::config::Configuration> rconfig = game::config::Configuration::create();
    game::config::Configuration& config = *rconfig;
    ConfigurationEditor ed;
    ed.addDivider(0, "divi");

    // Check
    a.checkNonNull("01. getNodeByIndex", ed.getNodeByIndex(0));
    a.checkEqual("02. getSource", ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::NotStored);

    // Modify - has no effect
    ed.getNodeByIndex(0)->setSource(config, ConfigurationOption::User);
    a.checkEqual("11. getSource", ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::NotStored);
}

/** Test getSource() for multiple options.
    A: create a generic option with multiple options. Call getSource(), setSource().
    E: Correct value reported: "Mixed" if appropriate, otherwise same in option and ConfigurationEditor */
AFL_TEST("game.config.ConfigurationEditor:getSource:mixed", a)
{
    // Environment: two bool options with different locations in one node
    static const IntegerOptionDescriptor boolOption  = { "bool",  &BooleanValueParser::instance };
    static const IntegerOptionDescriptor otherOption = { "other", &BooleanValueParser::instance };
    afl::base::Ref<game::config::Configuration> rconfig = game::config::Configuration::create();
    game::config::Configuration& config = *rconfig;
    config[boolOption].set(1);
    config[boolOption].setSource(ConfigurationOption::System);
    config[otherOption].set(1);
    config[otherOption].setSource(ConfigurationOption::Game);

    ConfigurationEditor ed;
    ed.addGeneric(0, "multi", 1, "value")
        .addOption(boolOption)
        .addOption(otherOption);

    // Check
    a.checkNonNull("01. getNodeByIndex", ed.getNodeByIndex(0));
    a.checkEqual("02. getSource", ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::Mixed);

    // Modify
    ed.getNodeByIndex(0)->setSource(config, ConfigurationOption::User);
    a.checkEqual("11. getSource", ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::User);
    a.checkEqual("12. option source", config[boolOption].getSource(), ConfigurationOption::User);
    a.checkEqual("13. option source", config[otherOption].getSource(), ConfigurationOption::User);
}

/** Test change notification.
    A: create a ConfigurationEditor. Use loadValues(), updateValues() sequence. Modify properties of options.
    E: change correctly reported for value and source changes */
AFL_TEST("game.config.ConfigurationEditor:change-notification", a)
{
    // Environment: two bool options
    static const IntegerOptionDescriptor boolOption  = { "bool",  &BooleanValueParser::instance };
    static const IntegerOptionDescriptor otherOption = { "other", &BooleanValueParser::instance };
    afl::base::Ref<game::config::Configuration> rconfig = game::config::Configuration::create();
    game::config::Configuration& config = *rconfig;
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
    a.checkEqual("01. responses size", responses.size(), 0U);

    // Check for changes - still no change
    ed.updateValues(config, tx);
    a.checkEqual("11. responses size", responses.size(), 0U);

    // Modify otherOption value
    config[otherOption].set(0);
    ed.updateValues(config, tx);
    a.checkEqual("21. responses size", responses.size(), 1U);
    a.checkEqual("22. response", responses[0], 1U);

    // Modify boolOption location
    config[boolOption].setSource(ConfigurationOption::User);
    ed.updateValues(config, tx);
    a.checkEqual("31. responses size", responses.size(), 2U);
    a.checkEqual("32. response", responses[1], 0U);
}

/** Test alias handling.
    A: create a ConfigurationEditor and some alias options. Check operations.
    E: change correctly reported for value and source changes */
AFL_TEST("game.config.ConfigurationEditor:alias", a)
{
    static const IntegerOptionDescriptor boolOption = { "bool",  &BooleanValueParser::instance };
    static const AliasOptionDescriptor a1 = { "a1", "bool" };
    static const AliasOptionDescriptor a2 = { "a2", "deadlink" };
    afl::base::Ref<game::config::Configuration> rconfig = game::config::Configuration::create();
    game::config::Configuration& config = *rconfig;
    config[boolOption].set(0);
    config[boolOption].setSource(ConfigurationOption::User);
    config[a1].setSource(ConfigurationOption::System);
    config[a2].setSource(ConfigurationOption::System);

    ConfigurationEditor ed;
    ed.addGeneric(0, "1", 77, "v1").addOption(a1); // cannot use addToggle here
    ed.addGeneric(0, "2", 77, "v2").addOption(a2);

    // Verify state
    a.checkEqual("01. getSource", ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::User);    // property of forwarded option
    a.checkEqual("02. getSource", ed.getNodeByIndex(1)->getSource(config), ConfigurationEditor::System);  // property of dead link
    a.check("03. getFirstOption", ed.getNodeByIndex(0)->getFirstOption(config) == &config[boolOption]);
    a.check("04. getFirstOption", ed.getNodeByIndex(1)->getFirstOption(config) == &config[a2]);

    // Update
    ed.getNodeByIndex(0)->setSource(config, ConfigurationOption::Game);
    ed.getNodeByIndex(1)->setSource(config, ConfigurationOption::Game);
    ed.getNodeByIndex(0)->toggleValue(config);
    ed.getNodeByIndex(1)->toggleValue(config);

    // Verify state
    a.checkEqual("11. getSource", ed.getNodeByIndex(0)->getSource(config), ConfigurationEditor::Game);    // property of forwarded option
    a.checkEqual("12. getSource", ed.getNodeByIndex(1)->getSource(config), ConfigurationEditor::Game);    // property of dead link
    a.checkEqual("13. option value", config[boolOption](), 1);
}

/** Test addAll(). */
AFL_TEST("game.config.ConfigurationEditor:addAll", a)
{
    afl::string::NullTranslator tx;
    const int TYPE = 77;

    static const IntegerOptionDescriptor opt1  = { "v1",  &IntegerValueParser::instance };
    static const IntegerOptionDescriptor opt2  = { "v2",  &IntegerValueParser::instance };
    static const IntegerOptionDescriptor opt3  = { "v3",  &IntegerValueParser::instance };
    afl::base::Ref<game::config::Configuration> rconfig = game::config::Configuration::create();
    game::config::Configuration& config = *rconfig;
    config[opt1].set(42);
    config[opt2].set(23);
    config[opt3].set(69);

    ConfigurationEditor ed;
    ed.addAll(0, TYPE, config);

    // Verify
    a.checkEqual("01. getNumNodes", ed.getNumNodes(), 3U);

    ConfigurationEditor::Node* n1 = ed.getNodeByIndex(0);
    a.checkNonNull("11. getNodeByIndex", n1);
    a.checkEqual("12. getName", n1->getName(), "v1");
    a.checkEqual("13. getValue", n1->getValue(config, tx), "42");
    a.checkEqual("14. getType", n1->getType(), TYPE);
    a.checkEqual("15. getFirstOption", n1->getFirstOption(config), &config[opt1]);

    ConfigurationEditor::Node* n2 = ed.getNodeByIndex(1);
    a.checkNonNull("21. getNodeByIndex", n2);
    a.checkEqual("22. getName", n2->getName(), "v2");
    a.checkEqual("23. getValue", n2->getValue(config, tx), "23");
    a.checkEqual("24. getType", n2->getType(), TYPE);
    a.checkEqual("25. getFirstOption", n2->getFirstOption(config), &config[opt2]);

    ConfigurationEditor::Node* n3 = ed.getNodeByIndex(2);
    a.checkNonNull("31. getNodeByIndex", n3);
    a.checkEqual("32. getName", n3->getName(), "v3");
    a.checkEqual("33. getValue", n3->getValue(config, tx), "69");
    a.checkEqual("34. getType", n3->getType(), TYPE);
    a.checkEqual("35. getFirstOption", n3->getFirstOption(config), &config[opt3]);

    // Apply the editor to a different config
    afl::base::Ref<game::config::Configuration> rconfig2 = game::config::Configuration::create();
    game::config::Configuration& config2 = *rconfig2;
    config2[opt1].set(17);

    a.checkEqual("41. getValue", n1->getValue(config2, tx), "17");
    a.checkEqual("42. getValue", n2->getValue(config2, tx), "");
    a.checkEqual("43. getValue", n3->getValue(config2, tx), "");

    a.checkEqual("51. getFirstOption", n1->getFirstOption(config2), &config2[opt1]);
    a.checkNull("52. getFirstOption", n2->getFirstOption(config2));
    a.checkNull("53. getFirstOption", n3->getFirstOption(config2));
}
