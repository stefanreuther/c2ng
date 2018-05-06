/**
  *  \file ui/widgets/menuframe.hpp
  *  \brief Class ui::widgets::MenuFrame
  */
#ifndef C2NG_UI_WIDGETS_MENUFRAME_HPP
#define C2NG_UI_WIDGETS_MENUFRAME_HPP

#include "ui/eventloop.hpp"
#include "ui/layout/manager.hpp"
#include "ui/layoutablegroup.hpp"
#include "ui/root.hpp"

namespace ui { namespace widgets {

    class AbstractListbox;

    /** Popup menu frame.
        This is a top-level widget that contains other widgets (typically, a single listbox)
        and provides a frame for a popup menu, including appropriate behaviour.

        It is intended as a modal popup.
        It operates an EventLoop that returns nonzero when a menu item was selected (Key_Return),
        zero if the menu was cancelled (Key_Escape, click outside).

        MenuFrame can provide a pop-up animation. */
    class MenuFrame : public ui::LayoutableGroup {
     public:
        /** Constructor.
            \param mgr Layout manager (pass ui::layout::HBox::instance0 if you don't care)
            \param root Root (provides gfx::Engine/gfx::Timer, ColorScheme)
            \param loop EventLoop */
        MenuFrame(ui::layout::Manager& mgr, Root& root, EventLoop& loop);

        // LayoutableGroup/Widget:
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const;
        virtual void draw(gfx::Canvas& can);
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Start animation.
            Sets the frame size to \c startingSize, and starts the pop-up animation.
            The MenuFrame will expand until it has reached its preferred size.
            If will try to not exceed screen borders.
            \param startingSize Starting size of frame. Can be size 0. */
        void animate(gfx::Rectangle startingSize);

        /** Standard entry point for a normal pop-up menu.
            When calling this, both the MenuFrame and the given list box must be standalone widgets
            (not contained in any widget tree).
            \param list List box
            \param anchor Anchor point
            \return true if menu item was selected, false if menu was cancelled */
        bool doMenu(AbstractListbox& list, gfx::Point anchor);

     private:
        Root& m_root;
        EventLoop& m_loop;
        afl::base::Ref<gfx::Timer> m_timer;

        void onTick();
        void onMenuItemClick();
    };

} }

#endif
