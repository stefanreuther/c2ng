/**
  *  \file ui/widgets/optiongrid.hpp
  *  \brief Class ui::widgets::OptionGrid
  */
#ifndef C2NG_UI_WIDGETS_OPTIONGRID_HPP
#define C2NG_UI_WIDGETS_OPTIONGRID_HPP

#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"
#include "util/key.hpp"
#include "afl/functional/stringtable.hpp"

namespace ui { namespace widgets {

    /** Option grid.
        Implements a list of button/label/values used in option dialogs.
        Users can click the buttons to cause a sig_click callback. */
    class OptionGrid : public Widget {
     public:
        class Item;

        /** Reference to an item.
            Wraps a pointer to an item of the list, and offers operations on it.
            The pointer can be null.
            The idea is to use code like "findItem(id).setValue(...)" to modify an item,
            without having to deal with the option grid not containing a value today. */
        class Ref {
         public:
            /** Constructor. */
            Ref(Item* pItem = 0)
                : m_pItem(pItem)
                { }

            /** Change item value's font.
                \param font font request
                \return *this */
            Ref& setFont(gfx::FontRequest font);

            /** Change item's value.
                \param str New value
                \return *this */
            Ref& setValue(String_t str);

            /** Change item's value.
                \param str New value
                \return *this */
            Ref& setValue(const char* str);

            /** Change item's label.
                \param str New label
                \return *this */
            Ref& setLabel(String_t str);

            /** Change item's enabled status.
                \param flag New status
                \return *this */
            Ref& setEnabled(bool flag);

            /** Add possible value for this item.
                This will use the font configured for this item and make sure that the value column is large enough.
                Note that the font must have been set before calling this function.
                \param str potential value
                \return *this */
            Ref& addPossibleValue(String_t str);

            /** Add possible values for this item.
                This will use the font configured for this item and make sure that the value column is large enough.
                Note that the font must have been set before calling this function.
                \param values table of potential values
                \return *this */
            Ref& addPossibleValues(const afl::functional::StringTable_t& values);

         private:
            friend class OptionGrid;
            Item* m_pItem;
        };

        /** Constructor.
            \param leftWidth Minimum width of left (label) column, in pixels
            \param rightWidth Minimum width of right (value) column, in pixels
            \param root Root */
        OptionGrid(int leftWidth, int rightWidth, Root& root);
        ~OptionGrid();

        /** Add an item.
            \param id User-defined Id to locate the item later, should be unique in this OptionGrid
            \param key Invoking key (should be a single (unshifted) character)
            \param label Label string. Should not include a trailing ":".
            \return reference to newly-added item */
        Ref addItem(int id, util::Key_t key, String_t label);

        /** Find an item, given an Id.
            The reference allows modifying the item.
            If the item was not found, a null reference will be returned that just "swallows" calls.
            \return reference */
        Ref findItem(int id);

        /** Get anchor point for drop-down menu for an item.
            If the item is not visible, anchors the drop-down on the OptionGrid.
            \return anchor point */
        gfx::Point getAnchorPointForItem(int id);

        // Widget methods:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Signal: option selected.
            Called when the user selects an item.
            This should toggle the item.
            \param int Id of selected item */
        afl::base::Signal<void(int)> sig_click;

     private:
        int m_leftWidth;
        int m_rightWidth;
        afl::container::PtrVector<Item> m_items;
        Root& m_root;

        void doLayout(size_t from, size_t to);
    };

} }

#endif
