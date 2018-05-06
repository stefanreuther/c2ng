/**
  *  \file ui/widgets/abstractlistbox.hpp
  */
#ifndef C2NG_UI_WIDGETS_ABSTRACTLISTBOX_HPP
#define C2NG_UI_WIDGETS_ABSTRACTLISTBOX_HPP

#include "ui/scrollablewidget.hpp"
#include "afl/base/signal.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/base/deleter.hpp"

namespace ui { namespace widgets {

    class AbstractListbox : public ScrollableWidget {
     public:
        /// List-box widget flags.
        enum Flag {
            MenuBehaviour,      ///< Behave as a menu (ex lb_Menu). Cursor follows mouse, single-click fires sig_itemDoubleClick.
            NoPageKeys,         ///< Do not handle Page keys (ex lb_NoPageKeys).
            KeyboardMenu,       ///< Allow keyboard activation of context menu (ex lb_KeyboardMenu).
            EqualSizes,         ///< Optimisation hint: all list items are the same size.
            Blocked             ///< Block input but don't optically disable the widget. For temporary delays.
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        /// List-box item states.
        enum ItemState {
            PassiveItem,        ///< Item is not selected.
            DisabledItem,       ///< Item is disabled.
            ActiveItem,         ///< Item is selected, but we are not focused.
            FocusedItem         ///< Item is selected and we are focused.
        };

        /// List-box selection direction.
        enum Direction {
            GoUp,
            GoDown
        };

        AbstractListbox();

        // AbstractListbox virtuals:
        virtual size_t getNumItems() = 0;
        virtual bool isItemAccessible(size_t n) = 0;
        virtual int getItemHeight(size_t n) = 0;
        virtual int getHeaderHeight() = 0;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area) = 0;
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state) = 0;

        // ScrollableWidget virtuals:
        virtual int getPageTop();
        virtual int getPageSize();
        virtual int getCursorTop();
        virtual int getCursorSize();
        virtual int getTotalSize();

        // Widget virtuals:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        // virtual void handlePositionChange(gfx::Rectangle& oldPosition); --> child
        // virtual ui::layout::Info getLayoutInfo() const; --> child
        // virtual bool handleKey(util::Key_t key, int prefix);
        bool defaultHandleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        void setFlag(Flag flag, bool enable);
        bool hasFlag(Flag flag) const;
        void updateItem(size_t item);
        void updateCurrentItem();
        gfx::Rectangle getRelativeItemPosition(size_t item);
        gfx::Rectangle getAbsoluteItemPosition(size_t item);

        bool getItemFromRelativePosition(gfx::Point pt, size_t& item, gfx::Rectangle& area);
        ItemState getItemState(size_t nr);

        size_t getCurrentItem() const;
        void setCurrentItem(size_t nr, Direction dir = GoDown);
        void handleModelChange();

        afl::base::Signal<void(size_t)> sig_itemDoubleClick;
        afl::base::Signal<void(size_t)> sig_itemClick;
        afl::base::Signal<void(gfx::Point)> sig_menuRequest;

     private:
        Flags_t m_flags;

        size_t m_currentItem;
        int m_topY;

        bool m_mouseDown;

        void makeVisible(const gfx::Rectangle& relativeArea);
        gfx::Point getRelativeToAbsoluteOffset();
    };

} }

#endif
