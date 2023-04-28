/**
  *  \file game/config/configurationeditor.cpp
  *  \brief Class game::config::ConfigurationEditor
  */

#include <memory>
#include "game/config/configurationeditor.hpp"
#include "game/config/aliasoption.hpp"
#include "util/translation.hpp"
#include "util/updater.hpp"

using game::config::AliasOption;
using game::config::ConfigurationEditor;
using game::config::ConfigurationOption;

namespace {
    /* Resolve an option alias */
    ConfigurationOption& resolveAlias(ConfigurationOption& opt)
    {
        // Do not use a loop. Aliases shall not nest.
        if (AliasOption* a = dynamic_cast<AliasOption*>(&opt)) {
            if (ConfigurationOption* forwarded = a->getForwardedOption()) {
                return *forwarded;
            }
        }
        return opt;
    }
}


/*
 *  Node
 */

// Constructor.
game::config::ConfigurationEditor::Node::Node(int level, const String_t& name)
    : m_level(level), m_name(name),
      m_lastReportedSource(), m_lastReportedValue()
{ }

// Get indentation level.
int
game::config::ConfigurationEditor::Node::getLevel() const
{
    return m_level;
}

// Get name.
String_t
game::config::ConfigurationEditor::Node::getName() const
{
    return m_name;
}

// Get first option.
game::config::ConfigurationOption*
game::config::ConfigurationEditor::Node::getFirstOption(Configuration& config)
{
    class Task : public afl::base::Closure<void(ConfigurationOption&)> {
     public:
        Task()
            : m_result()
            { }
        virtual void call(ConfigurationOption& opt)
            {
                if (m_result == 0) {
                    m_result = &resolveAlias(opt);
                }
            }
        ConfigurationOption* get() const
            { return m_result; }
     private:
        ConfigurationOption* m_result;
    };
    Task t;
    enumOptions(config, t);
    return t.get();
}

// Get source (storage location) of this option.
game::config::ConfigurationEditor::Source
game::config::ConfigurationEditor::Node::getSource(const Configuration& config)
{
    class Task : public afl::base::Closure<void(ConfigurationOption&)> {
     public:
        Task()
            : m_result(NotStored)
            { }
        virtual void call(ConfigurationOption& opt)
            {
                Source me = convertSource(resolveAlias(opt).getSource());
                if (me != m_result) {
                    if (m_result == NotStored) {
                        m_result = me;
                    } else {
                        m_result = Mixed;
                    }
                }
            }
        Source get() const
            { return m_result; }
     private:
        Source m_result;
    };
    Task t;
    enumOptions(const_cast<Configuration&>(config), t);
    return t.get();
}

// Set source (storage location) of this option.
void
game::config::ConfigurationEditor::Node::setSource(Configuration& config, ConfigurationOption::Source src)
{
    class Task : public afl::base::Closure<void(ConfigurationOption&)> {
     public:
        Task(ConfigurationOption::Source src)
            : m_source(src)
            { }
        virtual void call(ConfigurationOption& opt)
            { resolveAlias(opt).setSource(m_source); }
     private:
        ConfigurationOption::Source m_source;
    };
    Task t(src);
    enumOptions(config, t);
}

// Toggle value.
void
game::config::ConfigurationEditor::Node::toggleValue(Configuration& config)
{
    // ex WConfigBooleanEditor::edit
    if (IntegerOption* p = dynamic_cast<IntegerOption*>(getFirstOption(config))) {
        p->set(!(*p)());
        p->markUpdated(ConfigurationOption::User);
    }
}

// Set value.
void
game::config::ConfigurationEditor::Node::setValue(Configuration& config, String_t value)
{
    // ex WConfigOptionEditor::edit (part)
    if (ConfigurationOption* opt = getFirstOption(config)) {
        opt->setAndMarkUpdated(value, ConfigurationOption::User);
    }
}

// Describe.
game::config::ConfigurationEditor::Info
game::config::ConfigurationEditor::Node::describe(Configuration& config, afl::string::Translator& tx)
{
    Info result;
    result.level  = getLevel();
    result.type   = getType();
    result.source = getSource(config);
    result.name   = getName();
    result.value  = getValue(config, tx);
    return result;
}

// Update cached values.
bool
game::config::ConfigurationEditor::Node::update(Configuration& config, afl::string::Translator& tx)
{
    util::Updater u;
    u.set(m_lastReportedSource, getSource(config));
    u.set(m_lastReportedValue,  getValue(config, tx));
    return u;
}


/*
 *  GenericNode
 */

game::config::ConfigurationEditor::GenericNode::GenericNode(int level, const String_t& name, int type, const String_t& value)
    : Node(level, name),
      m_type(type),
      m_value(value),
      m_optionNames()
{ }

// Add option, given its name.
game::config::ConfigurationEditor::GenericNode&
game::config::ConfigurationEditor::GenericNode::addOptionByName(const String_t& name)
{
    m_optionNames.push_back(name);
    return *this;
}

int
game::config::ConfigurationEditor::GenericNode::getType()
{
    return m_type;
}

String_t
game::config::ConfigurationEditor::GenericNode::getValue(const Configuration& /*config*/, afl::string::Translator& /*tx*/)
{
    return m_value;
}

void
game::config::ConfigurationEditor::GenericNode::enumOptions(Configuration& config, afl::base::Closure<void(ConfigurationOption&)>& fcn)
{
    for (size_t i = 0, n = m_optionNames.size(); i < n; ++i) {
        if (ConfigurationOption* opt = config.getOptionByName(m_optionNames[i])) {
            fcn.call(*opt);
        }
    }
}


/*
 *  ConfigurationEditor
 */

// Constructor.
game::config::ConfigurationEditor::ConfigurationEditor()
    : m_nodes()
{ }

// Destructor.
game::config::ConfigurationEditor::~ConfigurationEditor()
{ }

// Add newly-constructed node.
void
game::config::ConfigurationEditor::addNewNode(Node* p)
{
    if (p != 0) {
        m_nodes.pushBackNew(p);
    }
}

// Add a divider node.
void
game::config::ConfigurationEditor::addDivider(int level, const String_t& name)
{
    class DividerNode : public Node {
     public:
        DividerNode(int level, const String_t& name)
            : Node(level, name)
            { }
        virtual int getType()
            { return NoEditor; }
        virtual String_t getValue(const Configuration& /*config*/, afl::string::Translator& /*tx*/)
            { return String_t(); }
        virtual void enumOptions(Configuration& /*config*/, afl::base::Closure<void(ConfigurationOption&)>& /*fcn*/)
            { }
    };
    addNewNode(new DividerNode(level, name));
}

// Add a boolean integer option node.
void
game::config::ConfigurationEditor::addToggle(int level, const String_t& name, const IntegerOptionDescriptor& opt)
{
    class ToggleNode : public Node {
     public:
        ToggleNode(int level, const String_t& name, const IntegerOptionDescriptor& opt)
            : Node(level, name), m_option(opt)
            { }
        virtual int getType()
            { return ToggleEditor; }
        virtual String_t getValue(const Configuration& config, afl::string::Translator& tx)
            {
                // ex WConfigBooleanEditor::toString
                // This is also used for options that have a different converter than Yes/No,
                // so we cannot just convert to Yes/No by hand.
                // But we can translate Yes/No if we get it.
                String_t result = config[m_option].toString();
                if (result == N_("Yes") || result == N_("No")) {
                    result = tx(result);
                }
                return result;
            }
        virtual void enumOptions(Configuration& config, afl::base::Closure<void(ConfigurationOption&)>& fcn)
            { fcn.call(config[m_option]); }
     private:
        const IntegerOptionDescriptor& m_option;
    };
    addNewNode(new ToggleNode(level, name, opt));
}

// Add a generic node for an option group typically edited by a dialog.
game::config::ConfigurationEditor::GenericNode&
game::config::ConfigurationEditor::addGeneric(int level, const String_t& name, int type, const String_t& value)
{
    return addNew(new GenericNode(level, name, type, value));
}

// Add generic nodes for all options from a given configuration.
void
game::config::ConfigurationEditor::addAll(int level, int type, const Configuration& config)
{
    class NamedNode : public Node {
     public:
        NamedNode(int level, int type, const String_t& name)
            : Node(level, name), m_type(type)
            { }
        virtual int getType()
            { return m_type; }
        virtual String_t getValue(const Configuration& config, afl::string::Translator& /*tx*/)
            {
                if (ConfigurationOption* opt = config.getOptionByName(getName())) {
                    return opt->toString();
                } else {
                    return String_t();
                }
            }
        virtual void enumOptions(Configuration& config, afl::base::Closure<void(ConfigurationOption&)>& fcn)
            {
                if (ConfigurationOption* opt = config.getOptionByName(getName())) {
                    fcn.call(*opt);
                }
            }
     private:
        const int m_type;
    };

    afl::base::Ref<Configuration::Enumerator_t> e = config.getOptions();
    Configuration::OptionInfo_t item;
    while (e->getNextElement(item)) {
        addNewNode(new NamedNode(level, type, item.first));
    }
}

// Get node, given an index.
game::config::ConfigurationEditor::Node*
game::config::ConfigurationEditor::getNodeByIndex(size_t index) const
{
    if (index < m_nodes.size()) {
        return m_nodes[index];
    } else {
        return 0;
    }
}

// Get number of nodes.
size_t
game::config::ConfigurationEditor::getNumNodes() const
{
    return m_nodes.size();
}

// Initialize change tracking.
void
game::config::ConfigurationEditor::loadValues(Configuration& config, afl::string::Translator& tx)
{
    for (size_t i = 0, n = m_nodes.size(); i < n; ++i) {
        m_nodes[i]->update(config, tx);
    }
}

// Check for changes.
void
game::config::ConfigurationEditor::updateValues(Configuration& config, afl::string::Translator& tx)
{
    for (size_t i = 0, n = m_nodes.size(); i < n; ++i) {
        if (m_nodes[i]->update(config, tx)) {
            sig_change.raise(i);
        }
    }
}

// Utility: convert ConfigurationOption::Source to ConfigurationEditor::Source.
game::config::ConfigurationEditor::Source
game::config::ConfigurationEditor::convertSource(ConfigurationOption::Source src)
{
    switch (src) {
     case ConfigurationOption::Default: return Default;
     case ConfigurationOption::System:  return System;
     case ConfigurationOption::User:    return User;
     case ConfigurationOption::Game:    return Game;
    }
    return NotStored;
}
