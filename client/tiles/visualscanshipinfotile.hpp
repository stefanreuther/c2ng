/**
  *  \file client/tiles/visualscanshipinfotile.hpp
  */
#ifndef C2NG_CLIENT_TILES_VISUALSCANSHIPINFOTILE_HPP
#define C2NG_CLIENT_TILES_VISUALSCANSHIPINFOTILE_HPP

#include "afl/string/string.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class VisualScanShipInfoTile : public ui::SimpleWidget {
     public:
        enum Line {
            ShipMass,
            Waypoint,
            NextPosition,
            Damage,

            Speed
        };
        static const int NUM_LINES = 4;
        static const int NUM_ELEMENTS = 5;
        struct Content {
            String_t text[NUM_ELEMENTS];
        };

        explicit VisualScanShipInfoTile(ui::Root& root);
        ~VisualScanShipInfoTile();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        void setContent(const Content& content);

        void attach(game::proxy::ObjectObserver& oop);

     private:
        ui::Root& m_root;
        Content m_content;
        util::RequestReceiver<VisualScanShipInfoTile> m_reply;
    };

} }

#endif
