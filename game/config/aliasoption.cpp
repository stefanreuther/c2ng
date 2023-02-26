/**
  *  \file game/config/aliasoption.cpp
  *  \brief Class game::config::AliasOption
  */

#include "game/config/aliasoption.hpp"
#include "game/config/configuration.hpp"

game::config::AliasOption::AliasOption(Configuration& container, String_t forwardedOptionName)
    : m_container(container),
      m_forwardedOptionName(forwardedOptionName)
{ }

game::config::AliasOption::~AliasOption()
{ }

void
game::config::AliasOption::set(String_t value)
{
    if (ConfigurationOption* opt = getForwardedOption()) {
        opt->set(value);
    }
}

String_t
game::config::AliasOption::toString() const
{
    if (ConfigurationOption* opt = getForwardedOption()) {
        return opt->toString();
    } else {
        return String_t();
    }
}

game::config::ConfigurationOption*
game::config::AliasOption::getForwardedOption() const
{
    return m_container.getOptionByName(m_forwardedOptionName);
}
