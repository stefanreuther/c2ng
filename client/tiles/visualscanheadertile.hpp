/**
  *  \file client/tiles/visualscanheadertile.hpp
  */
#ifndef C2NG_CLIENT_TILES_VISUALSCANHEADERTILE_HPP
#define C2NG_CLIENT_TILES_VISUALSCANHEADERTILE_HPP

#include "afl/string/string.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "util/requestreceiver.hpp"
#include "util/skincolor.hpp"

namespace client { namespace tiles {

    class VisualScanHeaderTile : public ui::SimpleWidget {
     public:
        struct Content {
            String_t title;
            String_t subtitle;
            String_t type;
            util::SkinColor::Color subtitleColor;
            bool hasMessages;
            Content()
                : title(), subtitle(), type(), subtitleColor(util::SkinColor::Static),
                  hasMessages(false)
                { }
        };

        explicit VisualScanHeaderTile(ui::Root& root);
        ~VisualScanHeaderTile();

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
        util::RequestReceiver<VisualScanHeaderTile> m_reply;
    };

} }

#endif
