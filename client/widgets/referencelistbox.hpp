/**
  *  \file client/widgets/referencelistbox.hpp
  *  \brief Class client::widgets::ReferenceListbox
  */
#ifndef C2NG_CLIENT_WIDGETS_REFERENCELISTBOX_HPP
#define C2NG_CLIENT_WIDGETS_REFERENCELISTBOX_HPP

#include "game/ref/userlist.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    /** List box displaying a game::ref::UserList.
        This is used for all sorts of "list of objects" dialogs (e.g. mission targets).

        A Reference is a symbolic name for an object,
        a UserList contains a list of references pre-parsed to not require
        access to game data for rendering. */
    class ReferenceListbox : public ui::widgets::AbstractListbox {
     public:
        typedef game::ref::UserList::Item Item_t;

        /** Constructor.
            \param root User-interface root */
        explicit ReferenceListbox(ui::Root& root);

        /** Destructor. */
        ~ReferenceListbox();

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
        void setContent(const game::ref::UserList& list);

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

        static void drawItem(gfx::Context<util::SkinColor::Color>& ctx,
                             gfx::Rectangle area,
                             const Item_t& item,
                             gfx::ResourceProvider& provider);

     private:
        ui::Root& m_root;
        game::ref::UserList m_content;

        int m_numLines;
        int m_width;

        const Item_t* getItem(size_t index) const;
    };

} }

#endif
