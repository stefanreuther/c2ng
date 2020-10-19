/**
  *  \file client/widgets/friendlycodelist.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_FRIENDLYCODELIST_HPP
#define C2NG_CLIENT_WIDGETS_FRIENDLYCODELIST_HPP

#include "game/spec/friendlycodelist.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/root.hpp"

namespace client { namespace widgets {

    class FriendlyCodeList : public ui::widgets::AbstractListbox {
     public:
        FriendlyCodeList(ui::Root& root, const game::spec::FriendlyCodeList::Infos_t& list);
        ~FriendlyCodeList();

        void setFriendlyCode(const String_t& code);
        String_t getFriendlyCode() const;

        // AbstractListbox virtuals:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight();
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget virtuals:
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        const game::spec::FriendlyCodeList::Infos_t& m_list;
    };

} }

#endif
