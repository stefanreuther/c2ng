/**
  *  \file ui/widgets/framegroup.hpp
  *  \brief Class ui::widgets::FrameGroup
  */
#ifndef C2NG_UI_WIDGETS_FRAMEGROUP_HPP
#define C2NG_UI_WIDGETS_FRAMEGROUP_HPP

#include "ui/layoutablegroup.hpp"
#include "ui/colorscheme.hpp"
#include "afl/base/deleter.hpp"
#include "ui/draw.hpp"

namespace ui { namespace widgets {

    /** Frame around a group of widgets.
        The frame has a configurable thickness (setFrameWidth) and color (setType).
        In addition, the group leaves a configurable padding (setPadding) to the content. */
    class FrameGroup : public LayoutableGroup {
     public:
        /** Constructor.
            \param mgr Layout manager for content
            \param colors UI color scheme to allow palette access
            \param type Frame type (color) */
        FrameGroup(ui::layout::Manager& mgr, ColorScheme& colors, FrameType type);

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
        void setType(FrameType type);

        /** Get type (color).
            \return Type */
        FrameType getType() const;

        /** Wrap a single widget within a FrameGroup.
            \param del Deleter. The FrameGroup instance will be added here.
            \param colors UI color scheme to allow palette access
            \param type Frame type (color)
            \param widget Widget to add */
        static FrameGroup& wrapWidget(afl::base::Deleter& del, ColorScheme& colors, FrameType type, Widget& widget);

        // LayoutableGroup:
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const;

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);

        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ColorScheme& m_colors;
        FrameType m_frameType;
        int m_frameWidth;
        int m_padding;
    };

} }

#endif
