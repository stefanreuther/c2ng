/**
  *  \file game/config/costarrayoption.cpp
  *  \brief Class game::config::CostArrayOption
  */

#include "game/config/costarrayoption.hpp"
#include "afl/base/staticassert.hpp"

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

    // The last value of a list is repeated when we have fewer elements than expected.
    // We try to truncate the list from the end to find when we can stop.
    // Since MAX_PLAYERS=31 in c2ng, one-or-everything as in PCC2 would be a bad choice.
    // Most users expect one-or-11.
    // If we find the list can be truncated to more than one, but fewer than 11, we expand it to 11.
    const int MIN_PLAYERS = 11;
    static_assert(MIN_PLAYERS <= MAX_PLAYERS, "MIN_PLAYERS");

    // Find truncation point such that all elements after limit are identical to it.
    int limit = MAX_PLAYERS-1;
    while (limit > 0 && m_data[limit-1] == m_data[MAX_PLAYERS-1]) {
        --limit;
    }

    // Grow to 11.
    if (limit > 0 && limit < MIN_PLAYERS-1) {
        limit = MIN_PLAYERS-1;
    }

    // Format. All below limit are possibly different, limit is the repeating one.
    String_t result;
    for (int i = 0; i < limit; ++i) {
        result += m_data[i].toPHostString();
        result += ",";
    }
    result += m_data[limit].toPHostString();

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
