/**
  *  \file ui/widgets/transparentwindow.hpp
  *  \brief Class ui::widgets::TransparentWindow
  */
#ifndef C2NG_UI_WIDGETS_TRANSPARENTWINDOW_HPP
#define C2NG_UI_WIDGETS_TRANSPARENTWINDOW_HPP

#include "afl/base/ptr.hpp"
#include "ui/layoutablegroup.hpp"
#include "ui/layout/manager.hpp"
#include "gfx/dimbackgroundcolorscheme.hpp"

namespace ui { namespace widgets {

    /** Transparent window.
        This is a panel that sits on a background image.
        It is <em>not</em> a transparent, see-through window that can be placed over other live widgets. */
    class TransparentWindow : public LayoutableGroup {
     public:
        /** Constructor.
            \param parentColors ColorScheme that provides widget colors in getColor(), and the background image in drawBackground()
            \param manager Layout manager */
        TransparentWindow(gfx::ColorScheme<util::SkinColor::Color>& parentColors, ui::layout::Manager& manager);

        virtual void draw(gfx::Canvas& can);
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        gfx::DimBackgroundColorScheme m_colorScheme;
    };

} }

#endif
