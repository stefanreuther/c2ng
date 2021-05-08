/**
  *  \file client/widgets/consoleview.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_CONSOLEVIEW_HPP
#define C2NG_CLIENT_WIDGETS_CONSOLEVIEW_HPP

#include "ui/simplewidget.hpp"
#include "gfx/resourceprovider.hpp"
#include "util/skincolor.hpp"
#include "afl/container/ptrvector.hpp"

namespace client { namespace widgets {

    class ConsoleView : public ui::SimpleWidget {
     public:
        ConsoleView(gfx::ResourceProvider& provider, gfx::Point sizeCells);
        ~ConsoleView();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        void addLine(int nr, String_t text, gfx::HorizontalAlignment align, int bold, util::SkinColor::Color color);
        void clear();
        void setScrollbackIndicator(int n);

     private:
        gfx::ResourceProvider& m_provider;
        gfx::Point m_sizeCells;

        struct Line {
            String_t text;
            gfx::HorizontalAlignment align;
            int bold;
            util::SkinColor::Color color;
        };
        afl::container::PtrVector<Line> m_lines;
        int m_scrollback;
    };

} }

#endif
