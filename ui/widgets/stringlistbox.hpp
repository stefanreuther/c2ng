/**
  *  \file ui/widgets/stringlistbox.hpp
  *  \brief Class ui::widgets::StringListbox
  */
#ifndef C2NG_UI_WIDGETS_STRINGLISTBOX_HPP
#define C2NG_UI_WIDGETS_STRINGLISTBOX_HPP

#include "afl/functional/stringtable.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/colorscheme.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "util/stringlist.hpp"

namespace ui { namespace widgets {

    /** Standard String List Box Widget.

        This class provides the standard list box widget used for most cases.
        It displays a util::StringList, and provides handy functions to access
        the StringList's keys.

        In a StringListbox, users can type the first letter of an entry for quick search. */
    class StringListbox : public AbstractListbox {
     public:
        /** Constructor.
            @param provides  ResourceProvider instance
            @param scheme    ColorScheme instance */
        StringListbox(gfx::ResourceProvider& provider, ui::ColorScheme& scheme);

        /** Destructor. */
        ~StringListbox();

        // AbstractListbox:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget:
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

        // StringListbox:

        /** Add single item.
            @param key Key
            @param s   Value (text) */
        void addItem(int32_t key, const String_t& s);

        /** Add items from table.
            @param tab Table */
        void addItems(const afl::functional::StringTable_t& tab);

        /** Sort items.
            This will keep the cursor unchanged; it will not move with the current item. */
        void sortItemsAlphabetically();

        /** Exchange content.
            Exchanges this list with the other list's content (more efficient than setItems).
            This will keep the cursor unchanged; it will not try to find the current item in the new list.
            @param other List */
        void swapItems(util::StringList& other);

        /** Set content.
            Replaces this list with the other list's content.
            This will keep the cursor unchanged; it will not try to find the current item in the new list.
            @param other List */
        void setItems(const util::StringList& other);

        /** Get current content.
            @return content */
        const util::StringList& getStringList() const;

        /** Get current item's key.
            @return Current item's key, if any. */
        afl::base::Optional<int32_t> getCurrentKey() const;

        /** Set current key.
            If the given key corresponds to an existing list item, selects it as current.
            @param key Key */
        void setCurrentKey(int32_t key);

        /** Set preferred width of widget.
            Defines this widget's getLayoutInfo().
            @param n      Width
            @param pixels If true, n is number of pixels. If false, n, is em. */
        void setPreferredWidth(int n, bool pixels);

        /** Set preferred height of widget.
            Defines this widget's getLayoutInfo().
            @param n Number of lines */
        void setPreferredHeight(int n);

     private:
        util::StringList m_content;
        gfx::ResourceProvider& m_provider;
        ui::ColorScheme& m_colorScheme;

        int m_preferredWidth;   // in ems or pixels, 0 for automatic
        int m_preferredHeight;  // in lines, 0 for automatic
        bool m_preferredWidthInPixels;

        int m_tabWidth;
        int m_totalWidth;

        void updateMetrics(size_t from, size_t to);
        void clearMetrics();
    };

} }

#endif
