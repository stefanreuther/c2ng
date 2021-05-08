/**
  *  \file ui/widgets/stringlistbox.hpp
  */
#ifndef C2NG_UI_WIDGETS_STRINGLISTBOX_HPP
#define C2NG_UI_WIDGETS_STRINGLISTBOX_HPP

#include "ui/widgets/abstractlistbox.hpp"
#include "util/stringlist.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/colorscheme.hpp"
#include "afl/functional/stringtable.hpp"

namespace ui { namespace widgets {

    /** Standard String List Box Widget.

        This class provides the standard list box widget used for most cases.
        It displays a util::StringList, and provides handy functions to access
        the StringList's keys.

        In a StringListbox, users can type the first letter of an entry for quick search. */
    class StringListbox : public AbstractListbox {
     public:
        StringListbox(gfx::ResourceProvider& provider, ui::ColorScheme& scheme);
        ~StringListbox();

        // AbstractListbox:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget:
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

        // StringListbox:
        void addItem(int32_t key, const String_t& s);
        void addItems(const afl::functional::StringTable_t& tab);
        void sortItemsAlphabetically();
        void swapItems(util::StringList& other);
        void setItems(const util::StringList& other);
        const util::StringList& getStringList() const;

        bool getCurrentKey(int32_t& key) const;
        void setCurrentKey(int32_t key);

        void setPreferredWidth(int n, bool pixels);
        void setPreferredHeight(int n);

     private:
        util::StringList m_content;
        gfx::ResourceProvider& m_provider;
        ui::ColorScheme& m_colorScheme;

        int m_preferredWidth;   // in ems, 0 for automatic
        int m_preferredHeight;  // in lines, 0 for automatic
        bool m_preferredWidthInPixels;
    };

} }

#endif
