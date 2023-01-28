/**
  *  \file client/widgets/combatunitlist.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_COMBATUNITLIST_HPP
#define C2NG_CLIENT_WIDGETS_COMBATUNITLIST_HPP

#include <vector>
#include "ui/widgets/abstractlistbox.hpp"
#include "afl/bits/smallset.hpp"
#include "ui/root.hpp"

namespace client { namespace widgets {

    class CombatUnitList : public ui::widgets::AbstractListbox {
     public:
        enum Kind {
            Fleet,              ///< Fleet.
            Unit                ///< Unit (ship, planet, participant).
        };
        enum Flag {
            Tagged,
            Dead,
            Inaccessible
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        CombatUnitList(ui::Root& root);
        ~CombatUnitList();

        void clear();
        void addItem(Kind k, size_t slot, String_t label, Flags_t flags, util::SkinColor::Color color);
        bool findItem(Kind k, size_t slot, size_t& index) const;
        bool getItem(size_t index, Kind& k, size_t& slot) const;

        void setFlagBySlot(Kind k, size_t slot, Flag flag, bool set);
        void setFlagByIndex(size_t index, Flag flag, bool set);

        bool getCurrentFleet(size_t& slot) const;
        bool getCurrentShip(size_t& slot) const;

        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        struct Item {
            Kind kind;
            size_t slot;
            Flags_t flags;
            String_t label;
            util::SkinColor::Color color;

            Item(Kind kind, size_t slot, Flags_t flags, String_t label, util::SkinColor::Color color)
                : kind(kind), slot(slot), flags(flags), label(label), color(color)
                { }
        };
        ui::Root& m_root;
        std::vector<Item> m_items;

        afl::base::Ref<gfx::Font> getFont() const;
    };

} }

#endif
