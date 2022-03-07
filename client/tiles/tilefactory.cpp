/**
  *  \file client/tiles/tilefactory.cpp
  */

#include "client/tiles/tilefactory.hpp"
#include "afl/string/format.hpp"
#include "client/si/genericwidgetvalue.hpp"
#include "client/si/userside.hpp"
#include "client/si/widgetcommand.hpp"
#include "client/si/widgetwrapper.hpp"
#include "client/tiles/basescreenheadertile.hpp"
#include "client/tiles/errortile.hpp"
#include "client/tiles/planetscreenheadertile.hpp"
#include "client/tiles/selectionheadertile.hpp"
#include "client/tiles/shipcargotile.hpp"
#include "client/tiles/shipmovementtile.hpp"
#include "client/tiles/shipscreenheadertile.hpp"
#include "client/tiles/starchartheadertile.hpp"
#include "client/widgets/commanddataview.hpp"
#include "client/widgets/standarddataview.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/typehint.hpp"
#include "ui/rich/documentview.hpp"
#include "util/translation.hpp"
#include "client/tiles/taskeditortile.hpp"

namespace {
    struct TileConfig {
        const char* name;       // name of tile type
        const char* title;      // may be null
        int type;
    };

    const ui::Widget::State DisabledState = ui::Widget::DisabledState;

    const TileConfig ship_screen[] = {
        { "SHIPHEADER",    0, 0 },
        { "SHIPEQUIPMENT", N_("Equipment & Crew:"), 0 },
        { "SHIPCARGO",     N_("Aboard:"), 0 },
        { "SHIPMISSION",   N_("Mission:"), 0 },
        { "COMMENT",       0, 0 },
        { "SHIPMOVEMENT",  N_("Movement:"), 0 },
        { 0, 0, 0 },
    };

    const TileConfig planet_screen[] = {
        { "PLANETHEADER",    0, 0 },
        { "PLANETECONOMY",   N_("Economy:"), 0 },
        { "PLANETNATIVES",   N_("Natives:"), 0 },
        { "PLANETCOLONISTS", N_("Colonists:"), 0 },
        { "PLANETFCODE",     0, 0 },
        { "COMMENT",         0, 0 },
        { "PLANETLINK",      0, 0 },
        { 0, 0, 0 },
    };

    const TileConfig base_screen[] = {
        { "BASEHEADER",      0, 0 },
        { "BASEMINERAL",     N_("Minerals & Funds:"), 0 },
        { "BASETECH",        N_("Technology & Defense:"), 0 },
        { "BASEORDER",       N_("Orders:"), 0 },
        { "PLANETFCODE",     0, 0 },
        { "COMMENT",         0, 0 },
        { "BASELINK",        0, 0 },
        { 0, 0, 0 },
    };

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

    static const TileConfig ship_lock[] = {
        { "NARROWHEADER",        0, 0 },
        { "NARROWSHIPEQUIPMENT", 0, 0 },
        { "NARROWSHIPCARGO",     0, 0 },
        { "NARROWSHIPMISSION",   0, 0 },
        { 0, 0, 0 },
    };

    static const TileConfig planet_lock[] = {
        { "NARROWHEADER",          0, 0 },
        { "NARROWPLANETMINERAL",   0, 0 },
        { "NARROWPLANETECONOMY",   0, 0 },
        { "NARROWPLANETCOLONISTS", 0, 0 },
        { "NARROWPLANETNATIVES",   0, 0 },
        { "NARROWPLANETFCODE",     0, 0 },
        { 0, 0, 0 },
    };

    static const TileConfig unknown_planet_lock[] = {
        { "NARROWHEADER", 0, 0 },
        { 0, 0, 0 },
    };

    static const TileConfig base_lock[] = {
        { "NARROWHEADER",        0, 0 },
        { "NARROWPLANETMINERAL", 0, 0 },
        { "NARROWBASETECH",      0, 0 },
        { "NARROWBASEORDER",     0, 0 },
        { 0, 0, 0 },
    };

    static const TileConfig shiptask_screen[] = {
        { "SHIPTASKHEADER",      0, 0 },
        { "SHIPTASKEDITOR",      N_("Auto Task:"), 0 },
        { "SHIPTASKCOMMAND",     0, 0 },
        { 0, 0, 0 },
    };

    static const TileConfig planettask_screen[] = {
        { "PLANETTASKHEADER",    0, 0 },
        { "PLANETTASKEDITOR",    N_("Auto Task:"), 0 },
        { "PLANETTASKCOMMAND",   0, 0 },
        { 0, 0, 0 },
    };

    static const TileConfig basetask_screen[] = {
        { "BASETASKHEADER",      0, 0 },
        { "BASETASKEDITOR",      N_("Auto Task:"), 0 },
        { "BASETASKCOMMAND",     0, 0 },
        { 0, 0, 0 },
    };

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
        if (name == "SHIPSCREEN") {
            return ship_screen;
        } else if (name == "PLANETSCREEN") {
            return planet_screen;
        } else if (name == "BASESCREEN") {
            return base_screen;
//     if (name == "HISTORYSCREEN")
//         return history_screen;
//     if (name == "FLEETSCREEN")
//         return fleet_screen;
        } else if (name == "PLANETLOCK") {
            return planet_lock;
        } else if (name == "SHIPLOCK") {
            return ship_lock;
        } else if (name == "BASELOCK") {
            return base_lock;
        } else if (name == "UNKNOWNPLANETLOCK") {
            return unknown_planet_lock;
        } else if (name == "SHIPTASKSCREEN") {
            return shiptask_screen;
        } else if (name == "PLANETTASKSCREEN") {
            return planettask_screen;
        } else if (name == "BASETASKSCREEN") {
            return basetask_screen;
        } else if (name == "SHIPSELECTIONDIALOG") {
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

    ui::Widget* createDocumentViewTile(const char* name,
                                       ui::Root& root,
                                       int width, int height,
                                       afl::base::Deleter& deleter,
                                       client::si::UserSide& user,
                                       game::proxy::ObjectObserver& oop)
    {
        static const interpreter::NameTable NAMES[] = {
            { "SETCONTENT", client::si::wicRichDocumentSetContent, client::si::WidgetCommandDomain, interpreter::thProcedure }
        };
        std::auto_ptr<ui::Widget> p(new ui::rich::DocumentView(root.provider().getFont(gfx::FontRequest().addWeight(1))->getCellSize().scaledBy(width, height),
                                                               0,
                                                               root.provider()));
        client::si::WidgetWrapper& wrap = deleter.addNew(new client::si::WidgetWrapper(user, p, NAMES));
        wrap.attach(oop, name);
        wrap.setState(DisabledState, true); // FIXME: disable so it doesn't get focus - should we have a FocusableState instead?
        return &wrap;
    }

    class DataViewFactory {
     public:
        virtual void configure(client::widgets::StandardDataView& dv, ui::Root& root) = 0;

        ui::Widget* run(ui::Root& root, client::widgets::KeymapWidget& keys, int wi, int he, const char* name, afl::base::Deleter& deleter, client::si::UserSide& user, game::proxy::ObjectObserver& oop);
    };

    ui::Widget* DataViewFactory::run(ui::Root& root,
                                     client::widgets::KeymapWidget& keys,
                                     int wi, int he,
                                     const char* name,
                                     afl::base::Deleter& deleter,
                                     client::si::UserSide& user,
                                     game::proxy::ObjectObserver& oop)
    {
        using client::widgets::StandardDataView;
        static const interpreter::NameTable NAMES[] = {
            { "SETBUTTON",  client::si::wicDataViewSetButton,  client::si::WidgetCommandDomain, interpreter::thProcedure },
            { "SETCONTENT", client::si::wicDataViewSetContent, client::si::WidgetCommandDomain, interpreter::thProcedure }
        };

        std::auto_ptr<StandardDataView> p(new StandardDataView(root, gfx::Point(wi, he), keys));
        configure(*p, root);
        client::si::WidgetWrapper& wrap = deleter.addNew(new client::si::WidgetWrapper(user, std::auto_ptr<ui::Widget>(p), NAMES));
        wrap.attach(oop, name);
        wrap.setState(DisabledState, true); // FIXME: disable so it doesn't get focus - should we have a FocusableState instead?
        return &wrap;
    }

    class CommandDataViewFactory {
     public:
        virtual void configure(client::widgets::CommandDataView& dv) = 0;

        ui::Widget* run(ui::Root& root, client::widgets::KeymapWidget& keys, client::widgets::CommandDataView::Mode align, const char* name, afl::base::Deleter& deleter, client::si::UserSide& user, game::proxy::ObjectObserver& oop);
    };

    ui::Widget* CommandDataViewFactory::run(ui::Root& root,
                                            client::widgets::KeymapWidget& keys,
                                            client::widgets::CommandDataView::Mode align,
                                            const char* name,
                                            afl::base::Deleter& deleter,
                                            client::si::UserSide& user,
                                            game::proxy::ObjectObserver& oop)
    {
        using client::widgets::CommandDataView;
        static const interpreter::NameTable NAMES[] = {
            { "SETBUTTON",    client::si::wicCommandViewSetButton,    client::si::WidgetCommandDomain, interpreter::thProcedure },
            { "SETLEFTTEXT",  client::si::wicCommandViewSetLeftText,  client::si::WidgetCommandDomain, interpreter::thProcedure },
            { "SETRIGHTTEXT", client::si::wicCommandViewSetRightText, client::si::WidgetCommandDomain, interpreter::thProcedure }
        };

        std::auto_ptr<CommandDataView> p(new CommandDataView(root, keys, align));
        configure(*p);
        client::si::WidgetWrapper& wrap = deleter.addNew(new client::si::WidgetWrapper(user, std::auto_ptr<ui::Widget>(p), NAMES));
        wrap.attach(oop, name);
        wrap.setState(DisabledState, true); // FIXME: disable so it doesn't get focus - should we have a FocusableState instead?
        return &wrap;
    }
}



client::tiles::TileFactory::TileFactory(ui::Root& root,
                                        client::si::UserSide& user,
                                        afl::string::Translator& tx,
                                        client::widgets::KeymapWidget& keys,
                                        game::proxy::ObjectObserver& observer)
    : m_root(root),
      m_userSide(user),
      m_translator(tx),
      m_keys(keys),
      m_observer(observer)
{ }

client::tiles::TileFactory::~TileFactory()
{ }

ui::Widget*
client::tiles::TileFactory::createTile(String_t name, afl::base::Deleter& deleter) const
{
    // ex client/tiles.h:createTile
    // Common factory for PlanetLink/BaseLink
    class LinkFactory : public CommandDataViewFactory {
     public:
        void configure(client::widgets::CommandDataView& dv)
            {
                // ex WBaseOrderTile::WBaseOrderTile
                dv.addButton("F5", util::Key_F5);
                dv.addButton("F8", util::Key_F8);
            }
    };

    class NullFactory : public DataViewFactory {
     public:
        void configure(client::widgets::StandardDataView& /*dv*/, ui::Root& /*root*/)
            { }
    };

    // Base
    if (name == "BASEHEADER") {
        BaseScreenHeaderTile& tile = deleter.addNew(new BaseScreenHeaderTile(m_root, m_keys, false));
        tile.attach(m_observer);
        return &tile;
    }
    if (name == "BASETASKHEADER") {
        BaseScreenHeaderTile& tile = deleter.addNew(new BaseScreenHeaderTile(m_root, m_keys, true));
        tile.attach(m_observer);
        return &tile;
    }
    if (name == "BASEMINERAL") {
        // ex WBaseMineralTile::WBaseMineralTile
        return NullFactory().run(m_root, m_keys, 30, 4, "Tile.BaseMineral", deleter, m_userSide, m_observer);
    }
    if (name == "BASETECH") {
        class Factory : public DataViewFactory {
         public:
            void configure(client::widgets::StandardDataView& dv, ui::Root& root)
                {
                    // ex WBaseTechTile::WBaseTechTile
                    dv.addNewButton(dv.Top, 0, 0, new ui::widgets::Button("T", 't', root));
                    dv.addNewButton(dv.Top, 0, 1, new ui::widgets::Button("D", 'd', root));
                    dv.addNewButton(dv.Top, 0, 2, new ui::widgets::Button("S", 's', root));
                }
        };
        return Factory().run(m_root, m_keys, 30, 4, "Tile.BaseTech", deleter, m_userSide, m_observer);
    }
    if (name == "BASEORDER") {
        class Factory : public CommandDataViewFactory {
         public:
            void configure(client::widgets::CommandDataView& dv)
                {
                    // ex WBaseOrderTile::WBaseOrderTile
                    dv.addButton("B", 'b');
                    dv.addButton("R", 'r');
                    dv.addButton("M", 'm');
                    dv.addButton("A", 'a');
                }
        };
        return Factory().run(m_root, m_keys, client::widgets::CommandDataView::ButtonsLeft, "Tile.BaseOrder", deleter, m_userSide, m_observer);
    }
    if (name == "BASELINK") {
        // ex WPlanetBaseTile::WPlanetBaseTile
        return LinkFactory().run(m_root, m_keys, client::widgets::CommandDataView::ButtonsRight, "Tile.BaseLink", deleter, m_userSide, m_observer);
    }
    if (name == "BASEOVERVIEW") {
        return createDocumentViewTile("Tile.BaseOverview", m_root, 30, 10, deleter, m_userSide, m_observer);
    }

    // Planet
    if (name == "PLANETHEADER") {
        PlanetScreenHeaderTile& tile = deleter.addNew(new PlanetScreenHeaderTile(m_root, m_keys, false));
        tile.attach(m_observer);
        return &tile;
    }
    if (name == "PLANETTASKHEADER") {
        PlanetScreenHeaderTile& tile = deleter.addNew(new PlanetScreenHeaderTile(m_root, m_keys, true));
        tile.attach(m_observer);
        return &tile;
    }
    if (name == "PLANETECONOMY") {
        class Factory : public DataViewFactory {
         public:
            void configure(client::widgets::StandardDataView& dv, ui::Root& root)
                {
                    // ex WPlanetEconomyTile::WPlanetEconomyTile
                    dv.addNewButton(dv.Top, 0, 0, new ui::widgets::Button("G", 'g', root));
                    dv.addNewButton(dv.Top, 1, 1, new ui::widgets::Button("B", 'b', root));
                    dv.addNewButton(dv.Top, 0, 1, new ui::widgets::Button("M", 'm', root));
                    dv.addNewButton(dv.Top, 1, 2, new ui::widgets::Button("S", 's', root));
                    dv.addNewButton(dv.Top, 0, 2, new ui::widgets::Button("D", 'd', root));
                    dv.addNewButton(dv.Top, 0, 3, new ui::widgets::Button("C", 'c', root));
                }
        };
        return Factory().run(m_root, m_keys, 30, 8, "Tile.PlanetEconomy", deleter, m_userSide, m_observer);
    }
    if (name == "PLANETNATIVES") {
        return NullFactory().run(m_root, m_keys, 30, 4, "Tile.PlanetNatives", deleter, m_userSide, m_observer);
    }
    if (name == "PLANETCOLONISTS") {
        class Factory : public DataViewFactory {
         public:
            void configure(client::widgets::StandardDataView& dv, ui::Root& root)
                {
                    dv.addNewButton(dv.Bottom, 0, 0, new ui::widgets::Button("T", 't', root));
                }
        };
        return Factory().run(m_root, m_keys, 30, 3, "Tile.PlanetColonists", deleter, m_userSide, m_observer);
    }
    if (name == "PLANETFCODE") {
        class Factory : public DataViewFactory {
         public:
            void configure(client::widgets::StandardDataView& dv, ui::Root& root)
                {
                    dv.addNewButton(dv.Bottom, 0, 0, new ui::widgets::Button("F", 'f', root));
                }
        };
        return Factory().run(m_root, m_keys, 30, 2, "Tile.PlanetFCode", deleter, m_userSide, m_observer);
    }
    if (name == "PLANETLINK") {
        // ex WPlanetBaseTile::WPlanetBaseTile
        return LinkFactory().run(m_root, m_keys, client::widgets::CommandDataView::ButtonsRight, "Tile.PlanetLink", deleter, m_userSide, m_observer);
    }
    if (name == "PLANETOVERVIEW") {
        return createDocumentViewTile("Tile.PlanetOverview", m_root, 30, 10, deleter, m_userSide, m_observer);
    }

    // Ship
    if (name == "SHIPHEADER") {
        ShipScreenHeaderTile& tile = deleter.addNew(new ShipScreenHeaderTile(m_root, m_keys, ShipScreenHeaderTile::ShipScreen));
        tile.attach(m_observer);
        return &tile;
    }
    if (name == "SHIPTASKHEADER") {
        ShipScreenHeaderTile& tile = deleter.addNew(new ShipScreenHeaderTile(m_root, m_keys, ShipScreenHeaderTile::ShipTaskScreen));
        tile.attach(m_observer);
        return &tile;
    }
    if (name == "SHIPEQUIPMENT") {
        class Factory : public DataViewFactory {
         public:
            void configure(client::widgets::StandardDataView& dv, ui::Root& root)
                {
                    // ex WShipEquipmentTile::WShipEquipmentTile
                    dv.addNewButton(dv.Top, 0, 0, new ui::widgets::Button("S", 's', root));
                    dv.addNewButton(dv.Top, 1, 0, new ui::widgets::Button("G", 'g', root));
                    dv.addNewButton(dv.Top, 2, 0, new ui::widgets::Button("R", 'r', root));
                    dv.addNewButton(dv.Top, 0, 1, new ui::widgets::Button("C", 'c', root));
                }
        };
        return Factory().run(m_root, m_keys, 30, 6, "Tile.ShipEquipment", deleter, m_userSide, m_observer);
    }
    if (name == "SHIPCARGO") {
        ShipCargoTile& tile = deleter.addNew(new ShipCargoTile(m_root, m_translator, m_keys));
        tile.attach(m_observer);
        tile.setState(DisabledState, true); // FIXME: disable so it doesn't get focus - should we have a FocusableState instead?
        return &tile;
    }
    if (name == "SHIPMISSION") {
        class Factory : public DataViewFactory {
         public:
            void configure(client::widgets::StandardDataView& dv, ui::Root& root)
                {
                    // ex WShipMissionTile::WShipMissionTile
                    dv.addNewButton(dv.Top, 0, 0, new ui::widgets::Button("M", 'm', root));
                    dv.addNewButton(dv.Top, 0, 1, new ui::widgets::Button("E", 'e', root));
                    dv.addNewButton(dv.Top, 0, 2, new ui::widgets::Button("F", 'f', root));
                    dv.addNewButton(dv.Top, 1, 2, new ui::widgets::Button("B", 'b', root));
                }
        };
        return Factory().run(m_root, m_keys, 30, 3, "Tile.ShipMission", deleter, m_userSide, m_observer);
    }
    if (name == "SHIPMOVEMENT") {
        ShipMovementTile& tile = deleter.addNew(new ShipMovementTile(m_root, m_translator, m_keys));
        tile.attach(m_observer);
        tile.setState(DisabledState, true); // FIXME: disable so it doesn't get focus - should we have a FocusableState instead?
        return &tile;
    }
    if (name == "SHIPOVERVIEW") {
        return createDocumentViewTile("Tile.ShipOverview", m_root, 30, 12, deleter, m_userSide, m_observer);
    }

    // History
    if (name == "HISTORYHEADER") {
        ShipScreenHeaderTile& tile = deleter.addNew(new ShipScreenHeaderTile(m_root, m_keys, ShipScreenHeaderTile::HistoryScreen));
        tile.attach(m_observer);
        return &tile;
    }
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
    if (name == "COMMENT") {
        class Factory : public CommandDataViewFactory {
         public:
            void configure(client::widgets::CommandDataView& dv)
                {
                    // ex WBaseOrderTile::WBaseOrderTile
                    dv.addButton("F9", util::Key_F9);
                }
        };
        return Factory().run(m_root, m_keys, client::widgets::CommandDataView::ButtonsRight, "Tile.Comment", deleter, m_userSide, m_observer);
    }

    // Tasks
    // @change: we need to distinguish between different task types because the tile selects the task
//     if (name == "SHIPTASKCOMMAND")
//         return new WShipAutoTaskCommandTile(selection);
//     if (name == "PLANETTASKCOMMAND")
//         return new WPlanetAutoTaskCommandTile(selection);
//     if (name == "BASETASKCOMMAND")
//         return new WBaseAutoTaskCommandTile(selection);
    if (name == "SHIPTASKEDITOR") {
        TaskEditorTile& tile = deleter.addNew(new TaskEditorTile(m_root, m_userSide, interpreter::Process::pkShipTask));
        tile.attach(m_observer);
        return &tile;
    }
    if (name == "PLANETTASKEDITOR") {
        TaskEditorTile& tile = deleter.addNew(new TaskEditorTile(m_root, m_userSide, interpreter::Process::pkPlanetTask));
        tile.attach(m_observer);
        return &tile;
    }
    if (name == "BASETASKEDITOR") {
        TaskEditorTile& tile = deleter.addNew(new TaskEditorTile(m_root, m_userSide, interpreter::Process::pkBaseTask));
        tile.attach(m_observer);
        return &tile;
    }

    // Narrow
    if (name == "NARROWHEADER") {
        StarchartHeaderTile& tile = deleter.addNew(new StarchartHeaderTile(m_root));
        tile.attach(m_observer);
        tile.setState(DisabledState, true); // FIXME: disable so it doesn't get focus - should we have a FocusableState instead?
        return &tile;
    }
    if (name == "NARROWSHIPEQUIPMENT") {
        return NullFactory().run(m_root, m_keys, 25, 5, "Tile.NarrowShipEquipment", deleter, m_userSide, m_observer);
    }
    if (name == "NARROWSHIPCARGO") {
        return NullFactory().run(m_root, m_keys, 25, 4, "Tile.NarrowShipCargo", deleter, m_userSide, m_observer);
    }
    if (name == "NARROWSHIPMISSION") {
        return NullFactory().run(m_root, m_keys, 25, 6, "Tile.NarrowShipMission", deleter, m_userSide, m_observer);
    }
    if (name == "NARROWPLANETMINERAL") {
        return NullFactory().run(m_root, m_keys, 25, 5, "Tile.NarrowPlanetMinerals", deleter, m_userSide, m_observer);
    }
    if (name == "NARROWPLANETECONOMY") {
        return NullFactory().run(m_root, m_keys, 25, 3, "Tile.NarrowPlanetEconomy", deleter, m_userSide, m_observer);
    }
    if (name == "NARROWPLANETCOLONISTS") {
        return NullFactory().run(m_root, m_keys, 25, 3, "Tile.NarrowPlanetColonists", deleter, m_userSide, m_observer);
    }
    if (name == "NARROWPLANETNATIVES") {
        return NullFactory().run(m_root, m_keys, 25, 4, "Tile.NarrowPlanetNatives", deleter, m_userSide, m_observer);
    }
    if (name == "NARROWPLANETFCODE") {
        return NullFactory().run(m_root, m_keys, 25, 2, "Tile.NarrowPlanetFCode", deleter, m_userSide, m_observer);
    }
    if (name == "NARROWBASETECH") {
        return NullFactory().run(m_root, m_keys, 25, 4, "Tile.NarrowBaseTech", deleter, m_userSide, m_observer);
    }
    if (name == "NARROWBASEORDER") {
        return NullFactory().run(m_root, m_keys, 25, 4, "Tile.NarrowBaseOrder", deleter, m_userSide, m_observer);
    }
    return 0;
}

void
client::tiles::TileFactory::createLayout(ui::LayoutableGroup& group, String_t layoutName, afl::base::Deleter& deleter) const
{
    // ex client/tiles.h:createTileLayout

    const TileConfig* cfg = getTileLayout(layoutName);
    if (!cfg) {
        group.add(deleter.addNew(new ErrorTile(afl::string::Format(m_translator("Error: unknown layout \"%s\"").c_str(), layoutName), m_root)));
    } else {
        while (cfg->name != 0) {
            // FIXME: handle 'type' field to select a custom selection. Feature is in PCC2, but not used.
            ui::Widget* p = createTile(cfg->name, deleter);
            if (!p) {
                group.add(deleter.addNew(new ErrorTile(afl::string::Format(m_translator("Error: unknown tile \"%s\"").c_str(), cfg->name), m_root)));
            } else {
                group.add(*p);

                // Configure the widget
                if (client::si::WidgetWrapper* wrap = dynamic_cast<client::si::WidgetWrapper*>(p)) {
                    p = wrap->getFirstChild();
                }
                if (client::widgets::CollapsibleDataView* dv = dynamic_cast<client::widgets::CollapsibleDataView*>(p)) {
                    if (cfg->title != 0) {
                        dv->setTitle(m_translator(cfg->title));
                        dv->setViewState(client::widgets::CollapsibleDataView::Complete);
                    } else {
                        dv->setViewState(client::widgets::CollapsibleDataView::DataOnly);
                    }
                }
            }
            ++cfg;
        }
    }
}
