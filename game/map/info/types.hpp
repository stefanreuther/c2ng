/**
  *  \file game/map/info/types.hpp
  *  \brief Types for game::map::info
  */
#ifndef C2NG_GAME_MAP_INFO_TYPES_HPP
#define C2NG_GAME_MAP_INFO_TYPES_HPP

#include "afl/base/types.hpp"
#include "afl/io/xml/node.hpp"
#include "afl/io/xml/tagnode.hpp"

namespace game { namespace map { namespace info {

    typedef afl::io::xml::Nodes_t Nodes_t;
    using afl::io::xml::TagNode;

    /* Pages */
    enum Page {
        TotalsPage,
        MineralsPage,
        PlanetsPage,
        ColonyPage,
        StarbasePage,
        StarshipPage,
        CapitalPage,
        StarchartPage,
        WeaponsPage
    };

    // ex LastPage
    static const size_t NUM_PAGES = static_cast<size_t>(WeaponsPage+1);

    /*
     *  Page Options
     */

    typedef uint8_t PageOptions_t;

    // MineralsPage
    const uint8_t Minerals_SortMask = 0x0F;
    const uint8_t Minerals_SortByTotal = 0;         // ex sort_by_total
    const uint8_t Minerals_SortByMined = 1;         // ex sort_by_mined

    const uint8_t Minerals_ShowMask = 0xF0;
    const uint8_t Minerals_ShowOnlyN = 0x10;        // ex show_only_N
    const uint8_t Minerals_ShowOnlyT = 0x20;        // ex show_only_T
    const uint8_t Minerals_ShowOnlyD = 0x30;        // ex show_only_D
    const uint8_t Minerals_ShowOnlyM = 0x40;        // ex show_only_M

    // StarshipPage/CapitalPage/StarbasePage
    const uint8_t Ships_SortMask = 0x0F;            // ex sort_mask
    const uint8_t Ships_SortByAmount    = 0;        // ex sort_by_amount
    const uint8_t Ships_SortByName      = 1;        // ex sort_by_name
    const uint8_t Ships_SortById        = 2;        // ex sort_by_id
    const uint8_t Ships_SortByMass      = 3;        // ex sort_by_mass
    const uint8_t Ships_SortByTech      = 4;        // ex sort_by_tech
    const uint8_t Ships_SortByCargo     = 5;        // ex sort_by_cargo
    const uint8_t Ships_SortByEngines   = 6;        // ex sort_by_engines
    const uint8_t Ships_SortByBeams     = 7;        // ex sort_by_beams
    const uint8_t Ships_SortByLaunchers = 8;        // ex sort_by_tubes
    const uint8_t Ships_SortByBays      = 9;        // ex sort_by_bays

    const uint8_t Ships_HideTop = 0x10;             // ex hide_top

    // ColonyPage
    const uint8_t Colony_ShowMask = 0xF0;
    const uint8_t Colony_ShowOnlyColonists = 0x10;  // ex show_only_Colonists
    const uint8_t Colony_ShowOnlySupplies  = 0x20;  // ex show_only_Supplies
    const uint8_t Colony_ShowOnlyMoney     = 0x30;  // ex show_only_Money

    // WeaponsPage
    const uint8_t Weapons_ShowEverything    = 0;    // ex show_everything
    const uint8_t Weapons_ShowOnlyBeams     = 1;    // ex show_only_Beams
    const uint8_t Weapons_ShowOnlyTorpedoes = 2;    // ex show_only_Torps
    const uint8_t Weapons_ShowOnlyRest      = 3;    // ex show_only_Rest

    // PlanetsPage
    const uint8_t Planets_SortByNumber   = 0;       // ex sort_P_by_NumPlanets
    const uint8_t Planets_SortByTotalPop = 1;       // ex sort_P_by_TotalPop
    const uint8_t Planets_SortByRace     = 2;       // ex sort_P_by_Race

} } }

#endif
