/**
  *  \file ui/reshack/colorselector.hpp
  *  \brief Class ui::reshack::ColorSelector
  */
#ifndef C2NG_UI_RESHACK_COLORSELECTOR_HPP
#define C2NG_UI_RESHACK_COLORSELECTOR_HPP

#include "afl/base/signalconnection.hpp"
#include "ui/simplewidget.hpp"

namespace ui { namespace reshack {

    class Painter;
    class Session;

    /** Color seclector for pictures.
        The 256-color palette is divided into 16 pages.
        This widget displays one page of colors, and controls to change the page or edit a color.
        It also allows to select foreground and background colors of a Painter, and displays those. */
    class ColorSelector : public SimpleWidget {
     public:
        /** Constructor.
            @param session Session (for Root, Translator)
            @param painter Painter whose colors to control; also used as Root::PaletteHandler to reflect palette changes */
        ColorSelector(Session& session, Painter& painter);
        ~ColorSelector();

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        class PalettePreview;
        class PaletteEditorDialog;

        Session& m_session;
        Painter& m_painter;
        uint8_t m_page;
        bool m_mouseDown;
        afl::base::SignalConnection conn_colorChange;

        void onNextPage();
        void onPreviousPage();
        void editColor(uint8_t color);
    };

} }

#endif
