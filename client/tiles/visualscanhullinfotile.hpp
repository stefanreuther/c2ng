/**
  *  \file client/tiles/visualscanhullinfotile.hpp
  */
#ifndef C2NG_CLIENT_TILES_VISUALSCANHULLINFOTILE_HPP
#define C2NG_CLIENT_TILES_VISUALSCANHULLINFOTILE_HPP

#include "afl/string/string.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class VisualScanHullInfoTile : public ui::SimpleWidget {
     public:
        enum Line {
            HullMass,
            Cargo,
            Fuel,
            Beams,
            Secondary,
            FriendlyCode
        };
        static const int NUM_LINES = 6;
        struct Content {
            String_t text[NUM_LINES];
        };

        explicit VisualScanHullInfoTile(ui::Root& root);
        ~VisualScanHullInfoTile();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        void setContent(const Content& content);

        void attach(game::proxy::ObjectObserver& oop);

     private:
        ui::Root& m_root;
        Content m_content;
        util::RequestReceiver<VisualScanHullInfoTile> m_reply;
    };

} }

#endif
