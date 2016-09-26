/**
  *  \file game/config/integeroption.cpp
  */

#include "game/config/integeroption.hpp"
#include "game/config/valueparser.hpp"


game::config::IntegerOption::IntegerOption(const ValueParser& parser, int32_t initialValue)
    : ConfigurationOption(),
      m_parser(parser),
      m_value(initialValue)
{ }

game::config::IntegerOption::~IntegerOption()
{ }

void
game::config::IntegerOption::set(String_t value)
{
    // ex ConfigValueOption<T>::set
    removeComment(value);
    set(m_parser.parse(value));
}

String_t
game::config::IntegerOption::toString() const
{
    // ex ConfigValueOption<T>::toString
    return m_parser.toString(m_value);
}

void
game::config::IntegerOption::set(int32_t newValue)
{
    if (m_value != newValue) {
        m_value = newValue;
        markChanged();
    }
}

int32_t
game::config::IntegerOption::operator()() const
{
    return m_value;
}

void
game::config::IntegerOption::copyFrom(const IntegerOption& other)
{
    set(other.m_value);
}
