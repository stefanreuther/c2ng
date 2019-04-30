/**
  *  \file ui/widgets/focusablegroup.hpp
  *  \brief Class ui::widgets::FocusableGroup
  */
#ifndef C2NG_UI_WIDGETS_FOCUSABLEGROUP_HPP
#define C2NG_UI_WIDGETS_FOCUSABLEGROUP_HPP

#include "ui/layoutablegroup.hpp"
#include "afl/base/deleter.hpp"

namespace ui { namespace widgets {

    /** Group that can have focus, visibly.

        This is a group which can indicate visibly that it has focus.
        It can be used to contain widgets that have no focus handling of their own,
        and ensures that keyboard focus is handled correctly.

        Otherwise it is very similar to ui::Group. */
    class FocusableGroup : public LayoutableGroup {
     public:
        static const int DEFAULT_PAD = 2;

        FocusableGroup(ui::layout::Manager& mgr, int pad = DEFAULT_PAD);
        ~FocusableGroup();

        // LayoutableGroup:
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const;

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);

        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        static FocusableGroup& wrapWidget(afl::base::Deleter& del, int pad, Widget& widget);
        static FocusableGroup& wrapWidget(afl::base::Deleter& del, Widget& widget);

     private:
        int m_pad;
    };

} }

#endif
