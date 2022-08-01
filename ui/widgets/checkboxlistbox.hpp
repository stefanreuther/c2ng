/**
  *  \file ui/widgets/checkboxlistbox.hpp
  *  \brief Class ui::widgets::CheckboxListbox
  */
#ifndef C2NG_UI_WIDGETS_CHECKBOXLISTBOX_HPP
#define C2NG_UI_WIDGETS_CHECKBOXLISTBOX_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "gfx/font.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "util/key.hpp"

namespace ui { namespace widgets {

    /** List of checkboxes.
        Displays a list of checkboxes.

        Each checkbox has:
        - an Id;
        - a label;
        - an info text;
        - an image name (usually, a checkbox);
        - an enabled/disabled state;
        - an optional hot-key.

        The checkbox image is displayed to the left.
        The label and info text can be displayed next to each other in a tabular layout (SingleLine),
        or above each other (MultiLine).

        Items can be reconfigured at any time.
        To address an item, this widget uses Item pointer.
        Setters will gracefully deal with null pointers.
        You can therefore use code such as:
        <pre>
            w.setItemImageName(w.findItem(MY_ID), "ui.cb1");
        </pre>

        This widget emits sig_checkboxClick whenever the user
        - presses an item's hot-key;
        - presses space on an item;
        - clicks an item's icon (the checkbox);
        - double-clicks an item.
        Caller needs to update the status and therefore the icon in this case. */
    class CheckboxListbox : public AbstractListbox {
     public:
        /** Layout. */
        enum Layout {
            SingleLine,         ///< Single line (checkbox, label, info).
            MultiLine           ///< Multiple lines (checkbox, label-atop-info).
        };
        struct Item;

        /** Constructor.
            \param root    UI root
            \param layout  Layout */
        CheckboxListbox(Root& root, Layout layout);
        ~CheckboxListbox();

        /** Set label width.
            By default, the width is determined automatically from the items' labels.
            If you use dynamic labels, it makes sense to fix it before.
            \param width Width. Negative to invoke default. */
        void setLabelWidth(int width);

        /** Set info width.
            By default, the width is determined automatically from the items' labels.
            If you use dynamic info, it makes sense to fix it before.
            \param width Width. Negative to invoke default. */
        void setInfoWidth(int width);

        /** Set preferred height.
            By default, the preferred height is determined from the number of items.
            If you have many items, it makes sense to set a fixed limit.
            \param height Height in number of items */
        void setPreferredHeight(int height);


        /*
         *  Item Management
         */

        /** Add item.
            \param id     Item Id
            \param label  Label
            \return Item handle */
        Item* addItem(int id, String_t label);

        /** Find item, given an Id.
            \param id Item Id
            \return Item handle; null if not found */
        Item* findItem(int id) const;

        /** Get item, given an index.
            \param index Item index [0, getNumItems())
            \return Item handle; null if index out of range */
        Item* getItemByIndex(size_t index) const;

        /** Set item accessibility state.
            \param p          Item (call is ignored if this is null)
            \param accessible New state
            \return p */
        Item* setItemAccessible(Item* p, bool accessible);

        /** Set item info.
            \param p     Item (call is ignored if this is null)
            \param info  New info
            \return p */
        Item* setItemInfo(Item* p, String_t info);

        /** Set item label.
            \param p      Item (call is ignored if this is null)
            \param label  New label
            \return p */
        Item* setItemLabel(Item* p, String_t label);

        /** Set item image name.
            \param p          Item (call is ignored if this is null)
            \param imageName  New image name
            \return p */
        Item* setItemImageName(Item* p, String_t imageName);

        /** Set item key.
            \param p    Item (call is ignored if this is null)
            \param key  New key
            \return p */
        Item* setItemKey(Item* p, util::Key_t key);

        /** Get item Id.
            \param p Item
            \return Id (0 if p is null) */
        int getItemId(Item* p) const;

        // AbstractListbox virtuals:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget virtuals:
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

        afl::base::Signal<void(int)> sig_checkboxClick;

     private:
        afl::base::Ref<gfx::Font> getLabelFont() const;
        afl::base::Ref<gfx::Font> getInfoFont() const;
        int getItemHeight() const;

        enum WidthMode {
            Unknown,            ///< Data has changed, value must be recomputed.
            Known,              ///< Data is unchanged, value is known.
            Fixed               ///< Value has been explicitly set, no need to compute.
        };

        Root& m_root;
        afl::container::PtrVector<Item> m_items;
        Layout m_layout;
        int m_labelWidth;
        int m_infoWidth;
        int m_preferredHeight;
        WidthMode m_labelMode;
        WidthMode m_infoMode;

        afl::base::SignalConnection conn_imageChange;

        static void invalidate(WidthMode& m);
        void updateWidths();
        void onItemDoubleClick(size_t index);
        void onItemClickAt(size_t index, gfx::Point pos);
    };

} }

#endif
