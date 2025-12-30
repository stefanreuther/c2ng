/**
  *  \file ui/reshack/fontcolorselector.hpp
  *  \brief Class ui::reshack::FontColorSelector
  */
#ifndef C2NG_UI_RESHACK_FONTCOLORSELECTOR_HPP
#define C2NG_UI_RESHACK_FONTCOLORSELECTOR_HPP

#include "ui/simplewidget.hpp"
#include "afl/base/signalconnection.hpp"

namespace ui { namespace reshack {

    class Painter;
    class Session;

    /** Color selector for fonts.
        Simple version that produces black/gray/white,
        no background color, no pages, no palette. */
    class FontColorSelector : public SimpleWidget {
     public:
        /** Constructor.
            @param session Session (for Root, Translator)
            @param painter Painter whose colors to control */
        FontColorSelector(Session& session, Painter& painter);
        ~FontColorSelector();

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        Session& m_session;
        Painter& m_painter;

        afl::base::SignalConnection conn_colorChange;
    };

} }

#endif
