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
