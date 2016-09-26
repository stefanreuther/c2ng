/**
  *  \file game/config/costarrayoption.cpp
  */

#include "game/config/costarrayoption.hpp"

game::config::CostArrayOption::CostArrayOption()
    : ConfigurationOption()
{ }

game::config::CostArrayOption::~CostArrayOption()
{ }

void
game::config::CostArrayOption::set(String_t value)
{
    // ex ConfigCostOption::set
    int count = 0;

    game::spec::Cost lastValue;
    String_t::size_type n;
    while (count < MAX_PLAYERS-1 && (n = value.find(',')) != String_t::npos) {
        lastValue = game::spec::Cost::fromString(value.substr(0, n));
        value.erase(0, n+1);
        set(++count, lastValue);
    }

    lastValue = game::spec::Cost::fromString(value);
    while (count < MAX_PLAYERS) {
        set(++count, lastValue);
    }
}

String_t
game::config::CostArrayOption::toString() const
{
    // ex ConfigCostOption::toString
    String_t result = m_data[0].toPHostString();

    bool different = false;
    for (int i = 1; i < MAX_PLAYERS; ++i) {
        if (m_data[i] != m_data[0]) {
            different = true;
            break;
        }
    }
    if (different) {
        for (int i = 1; i < MAX_PLAYERS; ++i) {
            result += ",";
            result += m_data[i].toPHostString();
        }
    }
    return result;
}

void
game::config::CostArrayOption::set(int index, const game::spec::Cost& cost)
{
    // ex ConfigCostOption::set
    if (index > 0 && index <= MAX_PLAYERS && cost != m_data[index-1]) {
        m_data[index-1] = cost;
        markChanged();
    }
}

game::spec::Cost
game::config::CostArrayOption::operator()(int index) const
{
    // ex ConfigCostOption::operator()
    if (index > 0 && index <= MAX_PLAYERS) {
        return m_data[index-1];
    } else {
        return m_data[MAX_PLAYERS-1];
    }
}
