/**
  *  \file client/widgets/fleetmemberlistbox.hpp
  *  \brief Class client::widgets::FleetMemberListbox
  */
#ifndef C2NG_CLIENT_WIDGETS_FLEETMEMBERLISTBOX_HPP
#define C2NG_CLIENT_WIDGETS_FLEETMEMBERLISTBOX_HPP

#include "game/ref/fleetmemberlist.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    /** List box displaying a game::ref::FleetMemberList. */
    class FleetMemberListbox : public ui::widgets::AbstractListbox {
     public:
        /** Constructor.
            @param root        UI root (for fonts, colors)
            @param prefLines   Preferred number of lines
            @param prefWidth   Preferred width in pixels */
        FleetMemberListbox(ui::Root& root, int prefLines, int prefWidth);

        /** Destructor. */
        ~FleetMemberListbox();

        /** Set content.
            @param content FleetMemberList */
        void setContent(const game::ref::FleetMemberList& content);

        /** Set current fleet member.
            Tries to place the cursor to point at the given ship.
            @param shipId  Fleet member Id */
        void setCurrentFleetMember(game::Id_t shipId);

        /** Get current fleet member.
            If the cursor is pointing at a ship, returns its Id.
            @return ship id or 0 */
        game::Id_t getCurrentFleetMember() const;

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
        game::ref::FleetMemberList m_content;
        int m_preferredNumLines;
        int m_preferredWidth;

        afl::base::Ref<gfx::Font> getFont() const;
    };

} }

#endif
