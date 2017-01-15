/**
  *  \file ui/widgets/stringlistbox.hpp
  */
#ifndef C2NG_UI_WIDGETS_STRINGLISTBOX_HPP
#define C2NG_UI_WIDGETS_STRINGLISTBOX_HPP

#include "ui/widgets/abstractlistbox.hpp"
#include "util/stringlist.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/colorscheme.hpp"

namespace ui { namespace widgets {

    // /** \class UIStandardListbox
    //     \brief Standard List Box Widget

    //     This class provides the standard list box widget used for most cases.
    //     It displays a UIStringList, and provides handy functions to access
    //     the UIStringList's keys.

    //     In a UIStandardListbox, users can type the first letter of an entry
    //     for "quick search". */
    class StringListbox : public AbstractListbox {
     public:
        StringListbox(gfx::ResourceProvider& provider, ui::ColorScheme& scheme);
        ~StringListbox();

        // AbstractListbox:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight();
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget:
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

        // StringListbox:
        void addItem(int32_t key, const String_t& s);
        void sortItemsAlphabetically();
        void swapItems(util::StringList& other);

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
