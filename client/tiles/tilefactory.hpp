/**
  *  \file client/tiles/tilefactory.hpp
  *  \brief Class client::tiles::TileFactory
  */
#ifndef C2NG_CLIENT_TILES_TILEFACTORY_HPP
#define C2NG_CLIENT_TILES_TILEFACTORY_HPP

#include "afl/base/deleter.hpp"
#include "afl/string/string.hpp"
#include "client/si/userside.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "game/proxy/fleetproxy.hpp"
#include "game/proxy/objectobserver.hpp"
#include "game/proxy/taskeditorproxy.hpp"
#include "ui/layoutablegroup.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"

namespace client { namespace tiles {

    class HistoryAdaptor;

    /** Factory for "tile" widgets.
        Tiles are displayed on control screens.
        They can listen to a number of objects providing the control screen's state,
        most prominently, an ObjectObserver proxy. */
    class TileFactory {
     public:
        /** Constructor.
            @param user      User side
            @param keys      KeymapWidget managing the control screen's keymap.
                             Tiles will dispatch their keys here.
            @param observer  ObjectObserver managing the control screen's object */
        TileFactory(client::si::UserSide& user,
                    gfx::KeyEventConsumer& keys,
                    game::proxy::ObjectObserver& observer);
        ~TileFactory();

        /** Add a TaskEditorProxy.
            This will be made available to tiles that can use it.
            @param p Pointer to TaskEditorProxy, owned by caller, living sufficiently long for tiles.
                     Can be null.
            @return this */
        TileFactory& withTaskEditorProxy(game::proxy::TaskEditorProxy* p);

        /** Add a FleetProxy.
            This will be made available to tiles that can use it.
            @param p Pointer to FleetProxy, owned by caller, living sufficiently long for tiles.
                     Can be null.
            @return this */
        TileFactory& withFleetProxy(game::proxy::FleetProxy* p);

        /** Add a HistoryAdaptor.
            This will be made available to tiles that can use it.
            @param p Pointer to HistoryAdaptor, owned by caller, living sufficiently long for tiles.
                     Can be null.
            @return this */
        TileFactory& withHistoryAdaptor(HistoryAdaptor* p);

        /** Create a single tile by name.
            @param name      Name
            @param deleter   Deleter; will own the tile and its sub-objects, of any.
            @return Newly-allocated tile managed by Deleter. Null if name is not recognized. */
        ui::Widget* createTile(String_t name, afl::base::Deleter& deleter) const;

        /** Create a layout by name.
            If the layout is not known or refers to an unknown tile, will create an ErrorTile.
            @param group      Widget group, will receive tiles.
            @param layoutName Name of layout to create
            @param deleter    Deleter; will own the tiles and their sub-objects, if any. */
        void createLayout(ui::LayoutableGroup& group, String_t layoutName, afl::base::Deleter& deleter) const;

     private:
        client::si::UserSide& m_userSide;
        gfx::KeyEventConsumer& m_keys;
        game::proxy::ObjectObserver& m_observer;
        game::proxy::TaskEditorProxy* m_pTaskEditor;
        game::proxy::FleetProxy* m_pFleetProxy;
        HistoryAdaptor* m_pHistoryAdaptor;
    };

} }

#endif
