/**
  *  \file game/spec/info/browser.hpp
  *  \brief Class game::spec::info::Browser
  */
#ifndef C2NG_GAME_SPEC_INFO_BROWSER_HPP
#define C2NG_GAME_SPEC_INFO_BROWSER_HPP

#include "afl/string/translator.hpp"
#include "game/root.hpp"
#include "game/spec/fighter.hpp"
#include "game/spec/info/types.hpp"
#include "game/spec/racialabilitylist.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace spec { namespace info {

    class Filter;
    class PictureNamer;

    /** Specification browser (Universe Almanac).

        Provides access to filtered lists of items, and formatted information about those.
        An item is identified by a Page and an Id.
        Valid Ids are provided by the Page's object list.

        This class ties together all objects required to produce this information,
        it contains no user-perceived mutable state. */
    class Browser {
     public:
        /** Constructor.
            \param picNamer Picture Namer (required to produce e.g. PageContent::pictureName attribute; must live longer than Browser)
            \param root Root (required for host/user configuration, host version, player list; must live longer than Browser)
            \param list Ship list (provides information to display; must live longer than Browser)
            \param viewpointPlayer Viewpoint player (required for some information)
            \param tx Translator; must live longer than Browser */
        Browser(const PictureNamer& picNamer, const Root& root, const ShipList& list, int viewpointPlayer, afl::string::Translator& tx);

        /** Describe an item.
            \param p Page (object type)
            \param id Id (obtained from listItems())
            \param withCost true to include cost and tech level in textual output
            \return newly-allocated information */
        std::auto_ptr<PageContent> describeItem(Page p, Id_t id, bool withCost) const;

        /** List items.
            \param p Page (object type)
            \param f Filter
            \param sort Sort order (Range_Id for default sort)
            \return newly-allocated information */
        std::auto_ptr<ListContent> listItems(Page p, const Filter& f, FilterAttribute sort) const;

        /** Describe filters.
            Same as Filter::describe(), but also marks those that are inactive on the given page.
            \param p Page (object type)
            \param filter Filter
            \return newly-allocated information */
        std::auto_ptr<FilterInfos_t> describeFilters(Page p, const Filter& filter) const;

        /** Get list of available filters for a page.
            Given the list of existing filters, this produces the list of filters that can still be added.
            The existing filters will not be part of the list.

            Use Filter::describe() to pack the existing filter into an equivalent list.
            
            \param p Page (object type)
            \param existing Existing filters
            \return newly-allocated information */
        std::auto_ptr<FilterInfos_t> getAvailableFilters(Page p, const Filter& existing) const;

        /** Get set of available filter attributes.
            \param p Page */
        FilterAttributes_t getAvailableFilterAttributes(Page p) const;

        /** Get set of available sort attributes.
            \param p Page */
        FilterAttributes_t getAvailableSortAttributes(Page p) const;

        /** Use item as filter.
            For example, when looking at player 3 (p=PlayerPage, id=3), this will add a (Value_Player, 3) filter.
            \param [in/out] f Filter
            \param [in] p Page (object type)
            \param [in] id Id */
        void addItemFilter(Filter& f, Page p, Id_t id) const;

        /** Access root.
            \return root */
        const Root& root() const;

        /** Access ship list.
            \return ship list */
        const ShipList& shipList() const;

        /** Access translator.
            \return translator */
        afl::string::Translator& translator() const;

     private:
        class Matcher;
        class CompareName;
        class CompareKey;

        const PictureNamer& m_picNamer;
        const Root& m_root;
        const ShipList& m_shipList;
        afl::string::Translator& m_translator;
        int m_viewpointPlayer;
        RacialAbilityList m_racialAbilities;

        OptionalInt_t getAttribute(Page p, Id_t id, FilterAttribute att) const;

        void describePlayer(PageContent& content, Id_t id) const;
        void listPlayers(ListContent& content, const Filter& f) const;
        bool matchPlayer(const Player& pl, const Filter& f) const;
        bool matchPlayer(const Player& pl, const FilterElement& e) const;
        bool matchPlayerName(const Player& pl, const String_t& f) const;

        void describeRacialAbility(PageContent& content, Id_t id) const;
        void listRacialAbilities(ListContent& content, const Filter& f) const;
        bool matchRacialAbility(const RacialAbilityList::Ability& a, const Filter& f) const;
        bool matchRacialAbility(const RacialAbilityList::Ability& a, const FilterElement& e) const;
        bool matchRacialAbilityName(const RacialAbilityList::Ability& a, const String_t& f) const;
        OptionalInt_t getRacialAbilityAttribute(const RacialAbilityList::Ability& a, FilterAttribute att) const;

        void describeShipAbility(PageContent& content, Id_t id) const;
        void listShipAbilities(ListContent& content, const Filter& f) const;
        bool matchShipAbility(const BasicHullFunction& hf, const Filter& f) const;
        bool matchShipAbility(const BasicHullFunction& hf, const FilterElement& e) const;
        bool matchShipAbilityName(const BasicHullFunction& hf, const String_t& f) const;

        void listHulls(ListContent& content, const Filter& f) const;
        bool matchHull(const Hull& h, const Filter& f, PlayerSet_t playerSet) const;
        bool matchHull(const Hull& h, const FilterElement& e, PlayerSet_t playerSet) const;

        void listEngines(ListContent& content, const Filter& f) const;
        bool matchEngine(const Engine& engine, const Filter& f) const;
        bool matchEngine(const Engine& engine, const FilterElement& e) const;

        void listBeams(ListContent& content, const Filter& f) const;
        bool matchBeam(const Beam& beam, const Filter& f) const;
        bool matchBeam(const Beam& beam, const FilterElement& e) const;

        void listTorpedoes(ListContent& content, const Filter& f) const;
        bool matchTorpedo(const TorpedoLauncher& torp, const Filter& f) const;
        bool matchTorpedo(const TorpedoLauncher& torp, const FilterElement& e) const;

        bool matchComponentName(const Component& comp, const String_t& name) const;

        void listFighters(ListContent& content, const Filter& f) const;
        bool matchFighter(const Fighter& ftr, const Filter& f) const;
        bool matchFighter(const Fighter& ftr, const FilterElement& e) const;

        void addFilterInfo(FilterInfos_t& result, FilterAttributes_t set, FilterAttribute att) const;

        bool checkShipAbility(const Hull& h, int basicFunctionId, IntRange_t levelRange, PlayerSet_t playerSet) const;
        bool checkPlayerShipAbility(int player, int basicFunctionId, IntRange_t levelRange) const;
    };

} } }

inline const game::Root&
game::spec::info::Browser::root() const
{
    return m_root;
}

inline const game::spec::ShipList&
game::spec::info::Browser::shipList() const
{
    return m_shipList;
}

inline afl::string::Translator&
game::spec::info::Browser::translator() const
{
    return m_translator;
}

#endif
