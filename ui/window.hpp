/**
  *  \file ui/window.hpp
  */
#ifndef C2NG_UI_WINDOW_HPP
#define C2NG_UI_WINDOW_HPP

#include "ui/layoutablegroup.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/draw.hpp"
#include "ui/layout/manager.hpp"
#include "afl/base/signalconnection.hpp"

namespace ui {

    class ColorScheme;

    class Window : public LayoutableGroup {
     public:
        Window(String_t title, gfx::ResourceProvider& provider, ColorScheme& uiColorScheme, const WindowStyle& style, const ui::layout::Manager& manager);
        ~Window();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        class WindowColorScheme : public gfx::ColorScheme<SkinColor::Color> {
         public:
            WindowColorScheme(Window& parent);
            virtual gfx::Color_t getColor(SkinColor::Color index);
            virtual void drawBackground(gfx::Canvas& can, const gfx::Rectangle& area);
         private:
            Window& m_parent;
        };
        friend class WindowColorScheme;

        String_t m_title;
        gfx::ResourceProvider& m_resourceProvider;
        const WindowStyle& m_style;
        int m_border;
        ColorScheme& m_uiColorScheme;
        WindowColorScheme m_colorScheme;
        afl::base::SignalConnection conn_providerImageChange;
    };

}

#endif
