/**
  *  \file ui/widgets/simpleiconbox.hpp
  */
#ifndef C2NG_UI_WIDGETS_SIMPLEICONBOX_HPP
#define C2NG_UI_WIDGETS_SIMPLEICONBOX_HPP

#include <vector>
#include "gfx/fontrequest.hpp"
#include "ui/root.hpp"
#include "ui/widgets/iconbox.hpp"

namespace ui { namespace widgets {

    class SimpleIconBox : public IconBox {
     public:
        static const int UsePlainKeys = 1;
        static const int UseAltKeys = 2;

        struct Item {
            String_t text;
            gfx::FontRequest font;
            Item(String_t text, gfx::FontRequest font = gfx::FontRequest())
                : text(text), font(font)
                { }
        };
        typedef std::vector<Item> Items_t;

        SimpleIconBox(gfx::Point size, ui::Root& root);
        ~SimpleIconBox();

        void setItemKeys(int itemKeys);

        // Widget:
        virtual ui::layout::Info getLayoutInfo() const;

        // IconBox:
        virtual int getItemWidth(size_t nr) const;
        virtual bool isItemKey(size_t nr, util::Key_t key) const;
        virtual size_t getNumItems() const;
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
        virtual void drawBlank(gfx::Canvas& can, gfx::Rectangle area);

        void swapContent(Items_t& items, size_t current);

     private:
        Items_t m_items;
        gfx::Point m_size;
        ui::Root& m_root;
        int m_itemKeys;
    };

} }

#endif
