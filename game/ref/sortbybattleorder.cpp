/**
  *  \file game/ref/sortbybattleorder.cpp
  */

#include "game/ref/sortbybattleorder.hpp"
#include "afl/string/format.hpp"
#include "util/unicodechars.hpp"
#include "game/map/universe.hpp"

game::ref::SortByBattleOrder::SortByBattleOrder(const game::map::Universe& univ, HostVersion host, afl::string::Translator& tx)
    : m_universe(univ),
      m_rule(host),
      m_translator(tx)
{ }

int
game::ref::SortByBattleOrder::compare(const Reference& a, const Reference& b) const
{
    return getBattleOrderValue(a) - getBattleOrderValue(b);
}

String_t
game::ref::SortByBattleOrder::getClass(const Reference& a) const
{
    // ex diviBattleOrder
    int n = getBattleOrderValue(a);
    if (n < 0) {
        return "< 0";
    } else if (n < 1000) {
        int level = n/100;
        return afl::string::Format("%d .. %d", 100*level, 100*level+99);
    } else if (n < BattleOrderRule::UNKNOWN) {
        return UTF_GEQ " 1000";
    } else {
        return m_translator.translateString("unknown");
    }

}

int
game::ref::SortByBattleOrder::getBattleOrderValue(const Reference& a) const
{
    // ex sortByBattleOrder
    if (const game::map::Object* obj = m_universe.getObject(a)) {
        return m_rule.get(*obj);
    } else {
        return BattleOrderRule::UNKNOWN;
    }
}
