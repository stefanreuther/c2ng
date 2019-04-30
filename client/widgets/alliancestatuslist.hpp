/**
  *  \file client/widgets/alliancestatuslist.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_ALLIANCESTATUSLIST_HPP
#define C2NG_CLIENT_WIDGETS_ALLIANCESTATUSLIST_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {


    // /** Alliance Status Widget.
    //     Displays a list of players, and the alliance status for each.
    //     - red if no offer
    //     - yellow of either side offers
    //     - green if alliance is established
    //     - bright red if an enemy order is given

    //     The player who owns the result file is displayed but cannot be selected.

    //     User must arrange for WAllyList::drawWidget() to be called if the GAlliances object changes. */

    class AllianceStatusList : public ui::widgets::AbstractListbox {
     public:
        enum ItemFlag {
            WeOffer,            // We offer an alliance
            TheyOffer,          // They offer an alliance
            Enemy,              // We declare them enemy
            Self                // It's ourselves (overrides all others)
        };
        typedef afl::bits::SmallSet<ItemFlag> ItemFlags_t;

        AllianceStatusList(ui::Root& root, afl::string::Translator& tx);

        void add(int id, const String_t& name, ItemFlags_t flags);
        void setFlags(int id, ItemFlags_t flags);
        int getCurrentPlayer() const;

        // AbstractListbox:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight();
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

        afl::base::Signal<void(int)> sig_selectPlayer;
        afl::base::Signal<void(int)> sig_toggleAlliance;

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;

        struct Item {
            int id;
            String_t name;
            ItemFlags_t flags;
            Item(int id, String_t name, ItemFlags_t flags)
                : id(id), name(name), flags(flags)
                { }
        };
        std::vector<Item> m_items;

        int getItemHeight() const;
        void computeWidth(int& leftWidth, int& rightWidth, int availableWidth) const;
        void onItemClickAt(size_t item, gfx::Point relativePosition);
        void onChange();
    };

} }

#endif
