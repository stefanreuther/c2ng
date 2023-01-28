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


    /** Alliance Status Widget.
        Displays a list of players, and the alliance status for each.
        - red if no offer
        - yellow of either side offers
        - green if alliance is established
        - bright red if an enemy order is given
        - unselectable if that's the field representing us */
    class AllianceStatusList : public ui::widgets::AbstractListbox {
     public:
        /** Flags for an item. */
        enum ItemFlag {
            WeOffer,            ///< We offer an alliance.
            TheyOffer,          ///< They offer an alliance.
            Enemy,              ///< We declare them enemy.
            Self                ///< It's ourselves (overrides all others).
        };
        typedef afl::bits::SmallSet<ItemFlag> ItemFlags_t;

        /** Constructor.
            \param root UI root
            \param tx   Translator */
        AllianceStatusList(ui::Root& root, afl::string::Translator& tx);

        /** Add an element.
            \param id     Id (player number)
            \param name   Name
            \param flags  Flags */
        void add(int id, const String_t& name, ItemFlags_t flags);

        /** Set element flags.
            \param id     Id (player number)
            \param flags  Flags */
        void setFlags(int id, ItemFlags_t flags);

        /** Get currently-selected player.
            \return player number */
        int getCurrentPlayer() const;

        // AbstractListbox:
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
