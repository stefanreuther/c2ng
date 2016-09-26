/**
  *  \file client/tiles/tilefactory.cpp
  */

#include "client/tiles/tilefactory.hpp"
#include "client/tiles/selectionheadertile.hpp"
#include "client/tiles/errortile.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"
#include "client/tiles/shipoverviewtile.hpp"

namespace {
    struct TileConfig {
        const char* name;       // name of tile type
        const char* title;      // may be null
        int type;
    };

// static const TileConfig ship_screen[] = {
//     { "SHIPHEADER",    0, 0 },
//     { "SHIPEQUIPMENT", N_("Equipment & Crew:"), 0 },
//     { "SHIPCARGO",     N_("Aboard:"), 0 },
//     { "SHIPMISSION",   N_("Mission:"), 0 },
//     { "COMMENT",       0, 0 },
//     { "SHIPMOVEMENT",  N_("Movement:"), 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig planet_screen[] = {
//     { "PLANETHEADER",    0, 0 },
//     { "PLANETECONOMY",   N_("Economy:"), 0 },
//     { "PLANETNATIVES",   N_("Natives:"), 0 },
//     { "PLANETCOLONISTS", N_("Colonists:"), 0 },
//     { "PLANETFCODE",     0, 0 },
//     { "COMMENT",         0, 0 },
//     { "PLANETLINK",      0, 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig base_screen[] = {
//     { "BASEHEADER",      0, 0 },
//     { "BASEMINERAL",     N_("Minerals & Funds:"), 0 },
//     { "BASETECH",        N_("Technology & Defense:"), 0 },
//     { "BASEORDER",       N_("Orders:"), 0 },
//     { "PLANETFCODE",     0, 0 },
//     { "COMMENT",         0, 0 },
//     { "BASELINK",        0, 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig history_screen[] = {
//     { "HISTORYHEADER",    0, 0 },
//     { "HISTORYEQUIPMENT", N_("Equipment & Crew:"), 0 },
//     { "HISTORYPOSITION",  0, 0 },
//     { "HISTORYMOVEMENT",  N_("Travelled this turn:"), 0 },
//     { "COMMENT",          0, 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig fleet_screen[] = {
//     { "FLEETHEADER",      0, 1 },
//     { "SHIPEQUIPMENT",    N_("Equipment & Crew:"), 1 },
//     { "FLEETMEMBERS",     0, 0 },
//     { "FLEETWAYPOINT",    0, 1 },
//     { 0, 0, 0 }
// };

// static const TileConfig ship_lock[] = {
//     { "NARROWHEADER",        0, 0 },
//     { "NARROWSHIPEQUIPMENT", 0, 0 },
//     { "NARROWSHIPCARGO",     0, 0 },
//     { "NARROWSHIPMISSION",   0, 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig planet_lock[] = {
//     { "NARROWHEADER",          0, 0 },
//     { "NARROWPLANETMINERAL",   0, 0 },
//     { "NARROWPLANETECONOMY",   0, 0 },
//     { "NARROWPLANETCOLONISTS", 0, 0 },
//     { "NARROWPLANETNATIVES",   0, 0 },
//     { "NARROWPLANETFCODE",     0, 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig unknown_planet_lock[] = {
//     { "NARROWHEADER", 0, 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig base_lock[] = {
//     { "NARROWHEADER",        0, 0 },
//     { "NARROWPLANETMINERAL", 0, 0 },
//     { "NARROWBASETECH",      0, 0 },
//     { "NARROWBASEORDER",     0, 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig shiptask_screen[] = {
//     { "SHIPTASKHEADER",      0, 0 },
//     { "TASKEDITOR",          N_("Auto Task:"), 0 },
//     { "SHIPTASKCOMMAND",     0, 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig planettask_screen[] = {
//     { "PLANETTASKHEADER",    0, 0 },
//     { "TASKEDITOR",          N_("Auto Task:"), 0 },
//     { "PLANETTASKCOMMAND",   0, 0 },
//     { 0, 0, 0 },
// };

// static const TileConfig basetask_screen[] = {
//     { "BASETASKHEADER",      0, 0 },
//     { "TASKEDITOR",          N_("Auto Task:"), 0 },
//     { "BASETASKCOMMAND",     0, 0 },
//     { 0, 0, 0 },
// };

    const TileConfig shipsel_dialog[] = {
        { "OBJHEADER",          0, 0 },
        { "SHIPOVERVIEW",       0, 0 },
        { 0, 0, 0 },
    };

    const TileConfig planetsel_dialog[] = {
        { "OBJHEADER",          0, 0 },
        { "PLANETOVERVIEW",     0, 0 },
        { 0, 0, 0 },
    };
    const TileConfig basesel_dialog[] = {
        { "OBJHEADER",          0, 0 },
        { "BASEOVERVIEW",       0, 0 },
        { 0, 0, 0 },
    };

    const TileConfig blank[] = {
        { 0, 0, 0 },
    };

    const TileConfig* getTileLayout(const String_t& name)
        {
//     if (name == "SHIPSCREEN")
//         return ship_screen;
//     if (name == "PLANETSCREEN")
//         return planet_screen;
//     if (name == "BASESCREEN")
//         return base_screen;
//     if (name == "HISTORYSCREEN")
//         return history_screen;
//     if (name == "FLEETSCREEN")
//         return fleet_screen;
//     if (name == "PLANETLOCK")
//         return planet_lock;
//     if (name == "SHIPLOCK")
//         return ship_lock;
//     if (name == "BASELOCK")
//         return base_lock;
//     if (name == "UNKNOWNPLANETLOCK")
//         return unknown_planet_lock;
//     if (name == "SHIPTASKSCREEN")
//         return shiptask_screen;
//     if (name == "PLANETTASKSCREEN")
//         return planettask_screen;
//     if (name == "BASETASKSCREEN")
//         return basetask_screen;
            if (name == "SHIPSELECTIONDIALOG") {
                return shipsel_dialog;
            } else if (name == "PLANETSELECTIONDIALOG") {
                return planetsel_dialog;
            } else if (name == "BASESELECTIONDIALOG") {
                return basesel_dialog;
            } else if (name == "") {
                return blank;
            }
            return 0;
        }
}



client::tiles::TileFactory::TileFactory(ui::Root& root,
                                        client::widgets::KeymapWidget& keys,
                                        ObjectObserverProxy& observer)
    : m_root(root),
      m_keys(keys),
      m_observer(observer)
{ }

client::tiles::TileFactory::~TileFactory()
{ }

ui::Widget*
client::tiles::TileFactory::createTile(String_t name, afl::base::Deleter& deleter) const
{
    // ex client/tiles.h:createTile

//     // Base
//     if (name == "BASEHEADER")
//         return new WBaseScreenHeaderTile(selection, false);
//     if (name == "BASEMINERAL")
//         return new WBaseMineralTile(selection);
//     if (name == "BASETECH")
//         return new WBaseTechTile(selection);
//     if (name == "BASEORDER")
//         return new WBaseOrderTile(selection);
//     if (name == "BASELINK")
//         return new WPlanetBaseTile(selection, true);
//     if (name == "BASEOVERVIEW")
//         return new WBaseOverviewTile(selection);

//     // Planet
//     if (name == "PLANETHEADER")
//         return new WPlanetScreenHeaderTile(selection, false);
//     if (name == "PLANETECONOMY")
//         return new WPlanetEconomyTile(selection);
//     if (name == "PLANETNATIVES")
//         return new WPlanetNativeTile(selection);
//     if (name == "PLANETCOLONISTS")
//         return new WPlanetColonistTile(selection);
//     if (name == "PLANETFCODE")
//         return new WPlanetFCodeTile(selection);
//     if (name == "PLANETLINK")
//         return new WPlanetBaseTile(selection, false);
//     if (name == "PLANETOVERVIEW")
//         return new WPlanetOverviewTile(selection);

//     // Ship
//     if (name == "SHIPHEADER")
//         return new WShipScreenHeaderTile(selection, WShipScreenHeaderTile::ShipScreen);
//     if (name == "SHIPEQUIPMENT")
//         return new WShipEquipmentTile(selection, false);
//     if (name == "SHIPCARGO")
//         return new WShipCargoTile(selection);
//     if (name == "SHIPMISSION")
//         return new WShipMissionTile(selection);
//     if (name == "SHIPMOVEMENT")
//         return new WShipMovementTile(selection);
    if (name == "SHIPOVERVIEW") {
        ShipOverviewTile& tile = deleter.addNew(new ShipOverviewTile(m_root));
        tile.attach(m_observer);
        return &tile;
    }

//     // History
//     if (name == "HISTORYHEADER")
//         return new WShipScreenHeaderTile(selection, WShipScreenHeaderTile::HistoryScreen);
//     if (name == "HISTORYPOSITION")
//         return new WHistoryShipPositionTile(selection);
//     if (name == "HISTORYEQUIPMENT")
//         return new WShipEquipmentTile(selection, true);
//     if (name == "HISTORYMOVEMENT")
//         return new WHistoryShipMovementTile(selection);

//     // Fleets
//     if (name == "FLEETHEADER")
//         return new WFleetScreenHeaderTile(selection);
//     if (name == "FLEETMEMBERS")
//         return new WFleetMemberTile(selection);
//     if (name == "FLEETWAYPOINT")
//         return new WFleetWaypointTile(selection);

//     // Misc
    if (name == "OBJHEADER") {
        SelectionHeaderTile& tile = deleter.addNew(new SelectionHeaderTile(m_root, m_keys));
        tile.attach(m_observer);
        return &tile;
    }
//     if (name == "OBJHEADER")
//         return new WObjectSelectionHeaderTile(selection);
//     if (name == "COMMENT")
//         return new WCommentTile(selection);

//     // Tasks
//     if (name == "SHIPTASKHEADER")
//         return new WShipScreenHeaderTile(selection, WShipScreenHeaderTile::ShipTaskScreen);
//     if (name == "SHIPTASKCOMMAND")
//         return new WShipAutoTaskCommandTile(selection);
//     if (name == "PLANETTASKHEADER")
//         return new WPlanetScreenHeaderTile(selection, true);
//     if (name == "PLANETTASKCOMMAND")
//         return new WPlanetAutoTaskCommandTile(selection);
//     if (name == "BASETASKHEADER")
//         return new WBaseScreenHeaderTile(selection, true);
//     if (name == "BASETASKCOMMAND")
//         return new WBaseAutoTaskCommandTile(selection);
//     if (name == "TASKEDITOR")
//         return new WAutoTaskEditorTile(selection);

//     // Narrow
//     if (name == "NARROWHEADER")
//         return new WNarrowHeaderTile(selection);
//     if (name == "NARROWSHIPEQUIPMENT")
//         return new WNarrowShipEquipmentTile(selection);
//     if (name == "NARROWSHIPCARGO")
//         return new WNarrowShipCargoTile(selection);
//     if (name == "NARROWSHIPMISSION")
//         return new WNarrowShipMissionTile(selection);
//     if (name == "NARROWPLANETMINERAL")
//         return new WNarrowPlanetMineralTile(selection);
//     if (name == "NARROWPLANETECONOMY")
//         return new WNarrowPlanetEconomyTile(selection);
//     if (name == "NARROWPLANETCOLONISTS")
//         return new WNarrowPlanetColonistTile(selection);
//     if (name == "NARROWPLANETNATIVES")
//         return new WNarrowPlanetNativeTile(selection);
//     if (name == "NARROWPLANETFCODE")
//         return new WNarrowPlanetFCodeTile(selection);
//     if (name == "NARROWBASETECH")
//         return new WNarrowBaseTechTile(selection);
//     if (name == "NARROWBASEORDER")
//         return new WNarrowBaseOrderTile(selection);

    return 0;
}

void
client::tiles::TileFactory::createLayout(ui::LayoutableGroup& group, String_t layoutName, afl::base::Deleter& deleter) const
{
    // ex client/tiles.h:createTileLayout

    const TileConfig* cfg = getTileLayout(layoutName);
    if (!cfg) {
        group.add(deleter.addNew(new ErrorTile(afl::string::Format(_("Error: unknown layout \"%s\"").c_str(), layoutName), m_root)));
    } else {
        while (cfg->name != 0) {
            // FIXME: port this: Figure out selection
            //         GObjectSelection* tileSelection;
            //         if (cfg->type == 0)
            //             tileSelection = &selection;
            //         else
            //             tileSelection = getObjectSelectionFromIteratorId(cfg->type);
            //         if (tileSelection == 0) {
            //             container.addNewTile(new WErrorTile(format(_("Error: unknown selection #%d"), cfg->type)));
            //         } else {
            ui::Widget* p = createTile(cfg->name, deleter);
            if (!p) {
                group.add(deleter.addNew(new ErrorTile(afl::string::Format(_("Error: unknown tile \"%s\"").c_str(), cfg->name), m_root)));
            } else {
                // FIXME: port this: tile config
                //                 if (cfg->title) {
                //                     t->setHeading(_(cfg->title));
                //                     t->setDisplayState(WTile::sNormal);
                //                 }
                group.add(*p);
            }
            //         }
            ++cfg;
        }
    }
}
