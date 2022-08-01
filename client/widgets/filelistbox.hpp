/**
  *  \file client/widgets/filelistbox.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_FILELISTBOX_HPP
#define C2NG_CLIENT_WIDGETS_FILELISTBOX_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/string/string.hpp"
#include "ui/root.hpp"
#include "ui/scrollablewidget.hpp"

namespace client { namespace widgets {

    class FileListbox : public ui::ScrollableWidget {
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

        FileListbox(int columns, int lines, ui::Root& root);
        ~FileListbox();

        // FileListbox:
        void swapItems(Items_t& items);
        const Item* getItem(size_t n);
        void setCurrentIndex(size_t n);

        // ScrollableWidget:
        virtual int getPageTop() const;
        virtual int getPageSize() const;
        virtual int getCursorTop() const;
        virtual int getCursorSize() const;
        virtual int getTotalSize() const;
        virtual void setPageTop(int top);
        virtual void scroll(Operation op);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        afl::base::Signal<void(size_t)> sig_itemDoubleClick;

     private:
        ui::Root& m_root;

        int m_columns;          // FIXME m_numColumns
        int m_lines;            // FIXME m_numLines

        int m_currentColumns;   // FIXME m_numCurrentColumns
        int m_currentColumnWidth;
        int m_currentLines;

        size_t m_firstItem;
        size_t m_currentItem;

        Items_t m_items;

        afl::base::Ptr<gfx::Canvas> m_icons;
        afl::base::SignalConnection conn_imageChange;

        gfx::Point getPreferredCellSize() const;

        void onImageChange();
        void updateSize();
        void updatePageTop();
        void updateCurrentItem();
        void scrollUp(size_t amount);
        void scrollDown(size_t amount);
    };

} }

#endif
