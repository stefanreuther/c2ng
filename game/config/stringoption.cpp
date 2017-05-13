/**
  *  \file game/config/stringoption.cpp
  *  \brief Class game::config::StringOption
  */

#include "game/config/stringoption.hpp"

game::config::StringOption::StringOption(const char* initialValue)
    : m_value(initialValue)
{ }

game::config::StringOption::~StringOption()
{ }

void
game::config::StringOption::set(String_t value)
{
    if (value != m_value) {
        m_value = value;
        markChanged();
    }
}

void
game::config::StringOption::set(const char* value)
{
    if (value != m_value) {
        m_value = value;
        markChanged();
    }
}

String_t
game::config::StringOption::toString() const
{
    return m_value;
}

String_t
game::config::StringOption::operator()() const
{
    return m_value;
}
