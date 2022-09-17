/**
  *  \file game/config/configurationoption.cpp
  *  \brief Class game::config::ConfigurationOption
  */

#include "game/config/configurationoption.hpp"

// Constructor.
game::config::ConfigurationOption::ConfigurationOption()
    : m_source(Default),
      m_changed(false)
{ }

// Get source of this option (place where it was set).
game::config::ConfigurationOption::Source
game::config::ConfigurationOption::getSource() const
{
    // ex ConfigOption::getSource
    return m_source;
}

// Set source of this option.
void
game::config::ConfigurationOption::setSource(Source source)
{
    // ex ConfigOption::setSource
    // @change A change of m_source counts as modification because configuration editors will show it
    if (m_source != source) {
        m_source = source;
        markChanged();
    }
}

// Check whether option was set.
bool
game::config::ConfigurationOption::wasSet() const
{
    // ex ConfigOption::wasSet
    return m_source != Default;
}

// Mark this option updated.
void
game::config::ConfigurationOption::markUpdated(Source source)
{
    // ex ConfigOption::markUpdated
    if (m_source < source) {
        m_source = source;
        markChanged();
    }
}

// Set this option and mark it updated.
void
game::config::ConfigurationOption::setAndMarkUpdated(String_t value, Source source)
{
    // ex ConfigOption::setAndMarkUpdated
    if (value != toString()) {
        set(value);
        markUpdated(source);
        markChanged();
    }
}

// Mark this option changed.
void
game::config::ConfigurationOption::markChanged(bool state)
{
    m_changed = state;
}

// Check whether this option was changed.
bool
game::config::ConfigurationOption::isChanged() const
{
    return m_changed;
}

// Remove comment from an option value.
void
game::config::ConfigurationOption::removeComment(String_t& str)
{
    // ex conf.cc:removeComment
    String_t::size_type n = str.find('#');
    if (n != str.npos) {
        str.erase(n);
    }
}
