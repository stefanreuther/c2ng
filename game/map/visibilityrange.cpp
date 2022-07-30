/**
  *  \file game/map/visibilityrange.cpp
  *  \brief Visibility Range Computation
  */

#include "game/map/visibilityrange.hpp"
#include "game/config/booleanvalueparser.hpp"
#include "game/config/enumvalueparser.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/rangeset.hpp"
#include "game/map/universe.hpp"

using game::config::HostConfiguration;

namespace {
    game::config::EnumValueParser vep_mode("Own,Ships,Planets,Marked");
    const game::config::IntegerOptionDescriptor opt_chart_range_mode     = { "Chart.Range.Mode",     &vep_mode };
    const game::config::IntegerOptionDescriptor opt_chart_range_team     = { "Chart.Range.Team",     &game::config::BooleanValueParser::instance };
    const game::config::IntegerOptionDescriptor opt_chart_range_distance = { "Chart.Range.Distance", &game::config::IntegerValueParser::instance };
}


game::map::VisSettings_t
game::map::getVisibilityRangeSettings(const game::config::HostConfiguration& config, int viewpointPlayer, afl::string::Translator& tx)
{
    VisSettings_t result;
    result.push_back(VisSetting(tx("Sensor range"),       VisModeShips,  config[HostConfiguration::SensorRange](viewpointPlayer)));
    result.push_back(VisSetting(tx("Visibility range"),   VisModeOwn,    config[HostConfiguration::ScanRange](viewpointPlayer)));
    result.push_back(VisSetting(tx("Minefields visible"), VisModeShips,  config[HostConfiguration::MineScanRange](viewpointPlayer)));
    result.push_back(VisSetting(tx("Dark sense"),         VisModeShips,  config[HostConfiguration::DarkSenseRange](viewpointPlayer)));
    return result;
}

void
game::map::buildVisibilityRange(RangeSet& out, const Universe& univ, const VisConfig& vc, const TeamSettings& team)
{
    // ex doEditRanges (part)
    // Discard old ranges
    out.clear();

    // Initial player set
    PlayerSet_t players(team.getViewpointPlayer());
    if (vc.useTeam) {
        players += team.getTeamPlayers(team.getPlayerTeam(team.getViewpointPlayer()));
    }

    // Build new ranges
    AnyShipType& tyAnyShips(const_cast<Universe&>(univ).allShips());
    AnyPlanetType& tyAnyPlanets(const_cast<Universe&>(univ).allPlanets());
    switch (vc.mode) {
     case VisModeOwn:
        out.addObjectType(tyAnyShips,   players, false, vc.range);
        out.addObjectType(tyAnyPlanets, players, false, vc.range);
        break;
     case VisModeShips:
        out.addObjectType(tyAnyShips,   players, false, vc.range);
        break;
     case VisModePlanets:
        out.addObjectType(tyAnyPlanets, players, false, vc.range);
        break;
     case VisModeMarked:
        out.addObjectType(tyAnyShips,   PlayerSet_t::allUpTo(MAX_PLAYERS), true, vc.range);
        out.addObjectType(tyAnyPlanets, PlayerSet_t::allUpTo(MAX_PLAYERS), true, vc.range);
        break;
    }
}

void
game::map::saveVisibilityConfiguration(game::config::UserConfiguration& pref, const VisConfig& vc)
{
    // ex WRangeDialog::save() const
    pref[opt_chart_range_team].set(vc.useTeam);
    pref[opt_chart_range_mode].set(vc.mode);
    pref[opt_chart_range_distance].set(vc.range);
}

game::map::VisConfig
game::map::loadVisibilityConfiguration(const game::config::UserConfiguration& pref)
{
    // ex WRangeDialog::init() (part)
    VisConfig result;
    result.useTeam = pref[opt_chart_range_team]() != 0;

    int mode = pref[opt_chart_range_mode]();
    if (mode >= 0 && mode <= VisModeMarked) {
        result.mode = static_cast<VisMode>(mode);
    }

    int dist = pref[opt_chart_range_distance]();
    if (dist >= 0 && dist <= 1000) {
        result.range = dist;
    }

    return result;
}

String_t
game::map::toString(VisMode mode, afl::string::Translator& tx)
{
    switch (mode) {
     case VisModeOwn:     return tx("own units");
     case VisModeShips:   return tx("own ships");
     case VisModePlanets: return tx("own planets");
     case VisModeMarked:  return tx("marked units");
    }
    return String_t();
}
