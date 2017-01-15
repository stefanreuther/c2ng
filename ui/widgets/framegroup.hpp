/**
  *  \file ui/widgets/framegroup.hpp
  *  \brief Class ui::widgets::FrameGroup
  */
#ifndef C2NG_UI_WIDGETS_FRAMEGROUP_HPP
#define C2NG_UI_WIDGETS_FRAMEGROUP_HPP

#include "ui/layoutablegroup.hpp"
#include "ui/colorscheme.hpp"

namespace ui { namespace widgets {

    /** Frame around a group of widgets.
        The frame has a configurable thickness (setFrameWidth) and color (setType).
        In addition, the group leaves a configurable padding (setPadding) to the content. */
    class FrameGroup : public LayoutableGroup {
     public:
        /// Frame type (color).
        enum Type {
            NoFrame,            ///< Don't draw a frame. ex WColorFrame cf_None, using fw=2
            RedFrame,           ///< Red frame.          ex WColorFrame cf_Red, using fw=2
            YellowFrame,        ///< Yellow frame.       ex WColorFrame cf_Yellow, using fw=2
            GreenFrame,         ///< Green frame.        ex WColorFrame cf_Green, using fw=2
            RaisedFrame,        ///< Raised 3D frame.
            LoweredFrame        ///< Lowered 3D frame.   ex UIFrameGroup, using fw=1, pad=frame-1
        };

        /** Constructor.
            \param mgr Layout manager for content
            \param colors UI color scheme to allow palette access
            \param type Frame type (color) */
        FrameGroup(ui::layout::Manager& mgr, ColorScheme& colors, Type type);

        /** Set frame width.
            This function should only be called during dialog setup (before layout).
            \param size Frame thickness in pixels. */
        void setFrameWidth(int size);

        /** Set padding.
            This function should only be called during dialog setup (before layout).
            \param size Padding in pixels. */
        void setPadding(int size);

        /** Set type (color).
            This function can be called at any time during the dialog.
            \param type New type */
        void setType(Type type);

        /** Get type (color).
            \return Type */
        Type getType() const;


        // LayoutableGroup:
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const;

        // Widget:
        virtual void draw(gfx::Canvas& can);

        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ColorScheme& m_colors;
        Type m_frameType;
        int m_frameWidth;
        int m_padding;
    };

} }

#endif
