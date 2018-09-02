/**
  *  \file client/tiles/tilefactory.hpp
  */
#ifndef C2NG_CLIENT_TILES_TILEFACTORY_HPP
#define C2NG_CLIENT_TILES_TILEFACTORY_HPP

#include "ui/root.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "afl/base/deleter.hpp"
#include "client/proxy/objectobserver.hpp"
#include "afl/string/string.hpp"
#include "ui/layoutablegroup.hpp"
#include "ui/widget.hpp"
#include "client/si/userside.hpp"

namespace client { namespace tiles {

    class TileFactory {
     public:
        TileFactory(ui::Root& root,
                    client::si::UserSide& user,
                    client::widgets::KeymapWidget& keys,
                    client::proxy::ObjectObserver& observer);
        ~TileFactory();

        ui::Widget* createTile(String_t name, afl::base::Deleter& deleter) const;

        void createLayout(ui::LayoutableGroup& group, String_t layoutName, afl::base::Deleter& deleter) const;

     private:
        ui::Root& m_root;
        client::si::UserSide& m_userSide;
        client::widgets::KeymapWidget& m_keys;
        client::proxy::ObjectObserver& m_observer;
    };

} }

#endif
