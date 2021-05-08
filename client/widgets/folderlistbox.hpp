/**
  *  \file client/widgets/folderlistbox.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_FOLDERLISTBOX_HPP
#define C2NG_CLIENT_WIDGETS_FOLDERLISTBOX_HPP

#include <vector>
#include "ui/widgets/abstractlistbox.hpp"
#include "gfx/point.hpp"
#include "ui/root.hpp"
#include "afl/base/signalconnection.hpp"

namespace client { namespace widgets {

    class FolderListbox : public ui::widgets::AbstractListbox {
     public:
        enum Icon { iNone, iFile, iGame, iFolder, iAccount, iUp, iComputer, iLink, iFavoriteFolder, iRoot, iFavorite };
        struct Item {
            String_t name;
            int indent;
            bool canEnter;
            Icon icon;
            Item(String_t name, int indent, bool canEnter, Icon icon)
                : name(name), indent(indent), canEnter(canEnter), icon(icon)
                { }
        };
        typedef std::vector<Item> Items_t;

        FolderListbox(gfx::Point cells, ui::Root& root);

        // AbstractListbox virtuals:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget:
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);

        void swapItems(Items_t& items);
        const Item* getItem(size_t n);

     private:
        Items_t m_items;
        gfx::Point m_cells;
        ui::Root& m_root;
        afl::base::Ref<gfx::Font> m_font;
        afl::base::Ptr<gfx::Canvas> m_icons;
        afl::base::SignalConnection conn_imageChange;

        void onImageChange();
    };

} }

#endif
