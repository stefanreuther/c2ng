/**
  *  \file ui/widgets/iconbox.hpp
  */
#ifndef C2NG_UI_WIDGETS_ICONBOX_HPP
#define C2NG_UI_WIDGETS_ICONBOX_HPP

#include "ui/simplewidget.hpp"
#include "ui/root.hpp"

namespace ui { namespace widgets {

// /** Icon box. Provides a horizontally-scrolling box of variable-width, same-height
//     icons. Users must derive from this class and implement the methods that provide
//     icon metrics and appearance. This class manages draw arbitration and mouse event
//     handling: one of the icons will be selected, and one may be hovered by the mouse. */
    class IconBox : public SimpleWidget {
     public:
        /** State of an item. */
        enum ItemState {
            Normal,             /**< Normal (idle). */
            Hover,              /**< Mouse is on item. */
            Selected            /**< Item is selected. */
        };

        explicit IconBox(ui::Root& root);

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        // virtual ui::layout::Info getLayoutInfo() const; --> child
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);
        
        // IconBox:
        void setCurrentItem(size_t nr);
        size_t getCurrentItem() const;

        void setChangeOnClick(bool enable);

        /** Signal: selection changed.
            Invoked when the current item changes, either by a setCurrentItem() call or by a mouse event. */
        afl::base::Signal<void(size_t)> sig_change;

        /** Return width of an item.
            \param number Item, [0,getNumberOfItems()).
            \return width in pixels */
        virtual int getItemWidth(size_t nr) = 0;

        /** Return number of items.
            \return number of items, >= 0 */
        virtual size_t getNumItems() = 0;

        /** Draw an item.
            \param number Item, [0,getNumberOfItems()).
            \param can Canvas to draw on
            \param r Position of item (width will be same as getItemWidth(number))
            \param state Item state */
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state) = 0;

        void handleStructureChange(size_t n);

     private:
        size_t m_currentItem;  /**< Currently-selected item. */
        size_t m_hoverItem;    /**< Currently-hovered item. */
        int m_leftX;           /**< X coordinate of leftmost item, as displayed. */
        int m_targetLeftX;     /**< X coordinate of leftmost item, target. */
        int m_scrollSpeed;     /**< Scroll speed used to make left_x == m_targetLeftX. */
        bool m_pendingScroll;  /**< True if we need to recenter when user releases the mouse. */
        bool m_mousePressed;   /**< True if the mouse is currently pressed. */
        bool m_mouseBlocked;   /**< True if the mouse is currently blocked. */
        bool m_changeOnClick;  /**< True if we want to change current upon a mouse click-release, not a mouse click-hold. */

        bool adjustPosition();

        ui::Root& m_root;

        afl::base::Ref<gfx::Timer> m_timer;
        void handleTimer();
    };

} }

#endif
