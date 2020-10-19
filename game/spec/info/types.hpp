/**
  *  \file game/spec/info/types.hpp
  *  \brief Common types for code in game::spec::info
  */
#ifndef C2NG_GAME_SPEC_INFO_TYPES_HPP
#define C2NG_GAME_SPEC_INFO_TYPES_HPP

#include <vector>
#include "afl/base/optional.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/experiencelevelset.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/types.hpp"
#include "util/range.hpp"

namespace game { namespace spec { namespace info {

    /*
     *  Basic Types
     */

    /** Shortcut: value range for game::spec::info namespace. */
    typedef util::Range<int32_t> IntRange_t;

    /** Shortcut: optional value for game::spec::info namespace */
    typedef afl::base::Optional<int> OptionalInt_t;


    /*
     *  Pages
     */

    /** Page identifier.
        Identifies a page in game::spec::info::Browser, and also serves as an object type identifier. */
    enum Page {
        PlayerPage,                ///< Player. Id is player Id (Player::getId()).
        HullPage,                  ///< Hull. Id is hull Id (Hull::getId()).
        RacialAbilitiesPage,       ///< Racial ability. Id is index into RacialAbilityList (RacialAbilityList::get()).
        ShipAbilitiesPage,         ///< Ship ability. Id is basic function Id (BasicHullFunction::getId()).
        EnginePage,                ///< Engine. Id is engine Id (Engine::getId()).
        BeamPage,                  ///< Beam. Id is beam Id (Beam::getId()).
        TorpedoPage                ///< Torpedo launcher. Id is torpedo Id (TorpedoLauncher::getId()).
    };

    /** Set of pages. */
    typedef afl::bits::SmallSet<Page> Pages_t;

    /** Object attribute. */
    struct Attribute {
        String_t name;             ///< Name of object.
        String_t value;            ///< Associated value. Empty to just show the name as a single line.
        Attribute(const String_t& name, const String_t& value)
            : name(name), value(value)
            { }
    };

    /** List of attributes. */
    typedef std::vector<Attribute> Attributes_t;

    /** Description of a ship/racial ability. */
    struct Ability {
        String_t info;             ///< Textual description.
        String_t pictureName;      ///< Picture name. \see PictureNamer::getAbilityPicture.
        Ability(const String_t& info, const String_t& pictureName)
            : info(info), pictureName(pictureName)
            { }
    };

    /** List of abilities. */
    typedef std::vector<Ability> Abilities_t;

    /** Page content.
        Contains the human-readable information for an object. */
    struct PageContent {
        String_t     title;        ///< Page title (object name).
        String_t     pictureName;  ///< Picture name. \see PictureNamer::getAbilityPicture.
        Attributes_t attributes;   ///< List of attributes (detail information).
        Pages_t      pageLinks;    ///< Related pages. \see Browser::addItemFilter.
        Abilities_t  abilities;    ///< Ship/racial abilities.
        PlayerSet_t  players;      ///< Players that can use this.
    };

    /** Object list.
        As of 20200510, this is NOT a util::StringList to allow addition of possible
        future attributes (icon/markers, colors). */
    struct ListEntry {
        String_t name;             ///< Name (list entry).
        Id_t id;                   ///< Associated Id.
        ListEntry(const String_t& name, Id_t id)
            : name(name), id(id)
            { }
    };

    /** Page object list. */
    struct ListContent {
        std::vector<ListEntry> content;
    };


    /*
     *  Filtering
     */

    /** Attribute to filter on. */
    enum FilterAttribute {
        Range_CostD,
        Range_CostM,
        Range_CostMC,
        Range_CostT,
        Range_DamagePower,
        Range_HitOdds,
        Range_IsArmed,
        Range_IsDeathRay,
        Range_KillPower,
        Range_Mass,
        Range_MaxBeams,
        Range_MaxCargo,
        Range_MaxCrew,
        Range_MaxEfficientWarp,
        Range_MaxFuel,
        Range_MaxLaunchers,
        Range_NumBays,
        Range_NumEngines,
        Range_NumMinesSwept,
        Range_RechargeTime,
        Range_Tech,
        Range_TorpCost,
        Range_Id,
        Value_Hull,
        Value_Player,
        Value_Category,
        Value_Origin,
        ValueRange_ShipAbility,
        String_Name
    };

    /** Set of filter attributes. */
    typedef afl::bits::SmallSet<FilterAttribute> FilterAttributes_t;

    /** Raw filter element.
        A Range_XXX attribute is filtered for an attribute being in the given range.
        A Value_XXX attribute is filtered for the attribute being exactly the given value.
        A ValueRange_XXX attribute needs to match both. */
    struct FilterElement {
        FilterAttribute att;       ///< Attribute to filter on.
        int32_t value;             ///< Value filter (for Value_XXX or ValueRange_XXX filter).
        IntRange_t range;          ///< Range filter (for Range_XXX or ValueRange_XXX filter).
        FilterElement(FilterAttribute att, int32_t value, IntRange_t range)
            : att(att), value(value), range(range)
            { }
    };

    /** Editing mode.

        Editable filters are presented with a FilterElement.
        - for existing filters, the existing FilterElement
        - for filters to add (Browser::getAvailableFilters), a default template
        The FilterEditMode specifies whether the range or value can be edited,
        and gives hints about the possible UI.
        After editing, call setRange()/setValue() (for existing filter) or add() (for new filter).

        As a special case, NotEditable marks filters that cannot sensibly be edited.

        As another special case, SetValueRange marks filters that have fixed parameter combinations.
        In this case, the FilterElement of an existing filter contains the NEW values to toggle that filter. */
    enum FilterEditMode {
        /** Filter is not editable. UI should not offer any edit. */
        NotEditable,
        /** Edit range.
            elem.range is current/default range; edit to be subrange of maxRange.
            elem.value is fixed. */
        EditRange,
        /** Edit range. Like EditRange, but offer special UI for level range. */
        EditRangeLevel,
        /** Edit value. Offer special UI for choosing a player.
            elem.value is current/default value; edit to be element of maxRange.
            elem.range is fixed. */
        EditValuePlayer,
        /** Edit value. Like EditValuePlayer, but offer special UI for choosing a hull. */
        EditValueHull,
        /** Edit string. The string is not in the filter element, but in FilterInfo::value. */
        EditString,
        /** Set fixed values. Should be represented in UI as a toggle. */
        SetValueRange
    };

    /** Cooked filter element. */
    struct FilterInfo {
        String_t name;             ///< Name of filter.
        String_t value;            ///< Current value of filter.
        FilterEditMode mode;       ///< Possible edit mode.
        IntRange_t maxRange;       ///< Maximum range (depends on mode).
        FilterElement elem;        ///< Current/default filter (depends on mode).
        bool active;               ///< true if filter is active.

        FilterInfo(const String_t& name, const String_t& value, FilterEditMode mode, IntRange_t maxRange, const FilterElement& elem)
            : name(name), value(value), mode(mode), maxRange(maxRange), elem(elem), active(true)
            { }
    };

    /** List of cooked filter elements. */
    typedef std::vector<FilterInfo> FilterInfos_t;

    /** Get name of FilterAttribute.
        \param att Attribute
        \param tx Translator
        \return name */
    String_t toString(FilterAttribute att, afl::string::Translator& tx);

    /** Convert integer range to ExperienceLevelSet_t.
        \param r Range
        \return set */
    ExperienceLevelSet_t convertRangeToSet(IntRange_t r);

    /** Get available experience level range.
        Used as range for EditRangeLevel.
        \param root Root
        \return Level range */
    IntRange_t getLevelRange(const Root& root);

    /** Get available hull Id range.
        Used as range for EditValueHull.
        \param shipList Ship list
        \return Level range */
    IntRange_t getHullRange(const ShipList& shipList);

    /** Get available player Id range.
        Used as range for EditValuePlayer.
        \param root Root
        \return Level range */
    IntRange_t getPlayerRange(const Root& root);

    /** Get default range for a filter attribute.
        This is for use in attribute queries (EditRange) and does NOT consider configuration.
        \param att Attribute
        \return Range */
    IntRange_t getAttributeRange(FilterAttribute att);

} } }

#endif
