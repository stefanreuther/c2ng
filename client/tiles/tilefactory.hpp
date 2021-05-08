/**
  *  \file client/tiles/tilefactory.hpp
  */
#ifndef C2NG_CLIENT_TILES_TILEFACTORY_HPP
#define C2NG_CLIENT_TILES_TILEFACTORY_HPP

#include "ui/root.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/string.hpp"
#include "client/si/userside.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/layoutablegroup.hpp"
#include "ui/widget.hpp"

namespace client { namespace tiles {

    class TileFactory {
     public:
        TileFactory(ui::Root& root,
                    client::si::UserSide& user,
                    afl::string::Translator& tx,
                    client::widgets::KeymapWidget& keys,
                    game::proxy::ObjectObserver& observer);
        ~TileFactory();

        ui::Widget* createTile(String_t name, afl::base::Deleter& deleter) const;

        void createLayout(ui::LayoutableGroup& group, String_t layoutName, afl::base::Deleter& deleter) const;

     private:
        ui::Root& m_root;
        client::si::UserSide& m_userSide;
        afl::string::Translator& m_translator;
        client::widgets::KeymapWidget& m_keys;
        game::proxy::ObjectObserver& m_observer;
    };

} }

#endif
