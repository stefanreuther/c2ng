/**
  *  \file ui/widgets/simpleiconbox.hpp
  */
#ifndef C2NG_UI_WIDGETS_SIMPLEICONBOX_HPP
#define C2NG_UI_WIDGETS_SIMPLEICONBOX_HPP

#include <vector>
#include "ui/widgets/iconbox.hpp"
#include "ui/root.hpp"
#include "gfx/fontrequest.hpp"

namespace ui { namespace widgets {

    class SimpleIconBox : public IconBox {
     public:
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

        // Widget:
        virtual ui::layout::Info getLayoutInfo() const;

        // IconBox:
        virtual int getItemWidth(size_t nr) const;
        virtual size_t getNumItems() const;
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        void swapContent(Items_t& items, size_t current);

     private:
        Items_t m_items;
        gfx::Point m_size;
        ui::Root& m_root;
    };

} }

#endif
