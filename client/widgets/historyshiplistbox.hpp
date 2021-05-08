/**
  *  \file client/widgets/historyshiplistbox.hpp
  *  \brief Class client::widgets::HistoryShipListbox
  */
#ifndef C2NG_CLIENT_WIDGETS_HISTORYSHIPLISTBOX_HPP
#define C2NG_CLIENT_WIDGETS_HISTORYSHIPLISTBOX_HPP

#include "afl/string/translator.hpp"
#include "game/ref/historyshiplist.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    /** List box displaying a game::ref::HistoryShipList.
        This is a close relative to ReferenceListbox. */
    class HistoryShipListbox : public ui::widgets::AbstractListbox {
     public:
        typedef game::ref::HistoryShipList::Item Item_t;

        /** Constructor.
            \param root User-interface root
            \param tx Translator */
        HistoryShipListbox(ui::Root& root, afl::string::Translator& tx);

        /** Destructor. */
        ~HistoryShipListbox();

        /** Set number of lines.
            This is used to determine the preferred layout size and needs not correlate with the actual content.
            \param n Number of lines */
        void setNumLines(int n);

        /** Set width.
            This is used to determine the preferred layout size and needs not correlate with the actual content.
            \param width Width in pixels */
        void setWidth(int width);

        /** Set content.
            If the new list contains the same object that is currently selected, it will remain selected.
            \param list New content */
        void setContent(const game::ref::HistoryShipList& list);

        /** Set current position to an object by reference.
            If an object with the desired Reference is part of the current content, selects it.
            \param ref Reference */
        void setCurrentReference(game::Reference ref);

        /** Get reference of currently-selected item.
            \return reference (null reference if no object selected (because list has no selectable objects)). */
        game::Reference getCurrentReference() const;

        // AbstractListbox / Widget:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        game::ref::HistoryShipList m_content;

        int m_numLines;
        int m_width;

        const Item_t* getItem(size_t index) const;
    };


} }

#endif
