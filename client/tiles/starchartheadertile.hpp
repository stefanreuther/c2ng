/**
  *  \file client/tiles/starchartheadertile.hpp
  */
#ifndef C2NG_CLIENT_TILES_STARCHARTHEADERTILE_HPP
#define C2NG_CLIENT_TILES_STARCHARTHEADERTILE_HPP

#include "afl/base/signalconnection.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class StarchartHeaderTile : public ui::SimpleWidget {
     public:
        enum Line {
            Name,
            Id,
            Type,
            Owner,
            Level,
            Mass
        };
        static const size_t NUM_LINES = Mass+1;

        struct Content {
            String_t text[NUM_LINES];
            String_t image;
        };

        StarchartHeaderTile(ui::Root& root);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        void setContent(const Content& content);
        void onImageChange();

        void attach(game::proxy::ObjectObserver& oop);

     private:
        ui::Root& m_root;
        Content m_content;
        util::RequestReceiver<StarchartHeaderTile> m_reply;
        afl::base::SignalConnection conn_imageChange;
        bool m_isMissingImage;
    };

} }

#endif
