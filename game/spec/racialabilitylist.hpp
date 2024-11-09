/**
  *  \file game/spec/racialabilitylist.hpp
  *  \brief Class game::spec::RacialAbilityList
  */
#ifndef C2NG_GAME_SPEC_RACIALABILITYLIST_HPP
#define C2NG_GAME_SPEC_RACIALABILITYLIST_HPP

#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/playerset.hpp"
#include "game/spec/shiplist.hpp"
#include "util/numberformatter.hpp"
#include "game/spec/advantagelist.hpp"

namespace game { namespace spec {

    /** List of racial abilities.
        In addition to the racial abilities derived from hull functions (see ShipList::racialAbilities()),
        players have other racial abilities such as special missions ("Lay Web Mines") or
        special configuration values ("200% mining rate").
        This class provides a way of obtaining those abilities.

        This object is intended to be transient, and it can be passed between game and UI.
        It contains only derived information that can be reproduced at any time. */
    class RacialAbilityList {
     public:
        /** Origin of an ability. */
        enum Origin {
            FromHullFunction,              ///< From hull function (ShipList::racialAbilities())
            FromConfiguration,             ///< From configuration (array-ized option).
            FromAdvantages                 ///< From advantages.
        };

        /** Category of an ability. */
        enum Category {
            Unclassified,                  ///< Unclassified.
            Combat,                        ///< Combat.
            Economy,                       ///< Economy/planets.
            Minefield,                     ///< Minefield (laying, sweeping).
            Sensor,                        ///< Sensor visibility.
            Ship,                          ///< Ships (missions, abilities).
            ShipBuilding                   ///< Ship building (build queue).
        };

        /** Ability description. */
        struct Ability {
            /** Origin of this ability. */
            Origin origin;

            /** Category of this ability. */
            Category category;

            /** Unique identifier.
                This value can be used to find the same ability in another RacialAbilityList instance created from the same original data.
                It is not guaranteed to be stable across turns or different ship lists. */
            uint32_t uniqueId;

            /** Basic hull function Id for hull functions. */
            int basicFunctionId;

            /** Name to be shown in lists. */
            String_t name;

            /** Detailed description. */
            String_t explanation;

            /** Picture name. */
            String_t pictureName;

            /** Players who can use this ability. */
            PlayerSet_t players;

            Ability(Origin origin, Category category, uint32_t uniqueId, int basicFunctionId, const String_t& name, const String_t& explanation, const String_t& pictureName, PlayerSet_t players)
                : origin(origin), category(category), uniqueId(uniqueId), basicFunctionId(basicFunctionId), name(name), explanation(explanation), pictureName(pictureName), players(players)
                { }
        };

        /** Container of abilities. */
        typedef std::vector<Ability> Abilities_t;

        /** Iterator. */
        typedef Abilities_t::const_iterator Iterator_t;


        /** Constructor.
            Makes an empty list. */
        RacialAbilityList();

        /** Destructor. */
        ~RacialAbilityList();

        /** Add abilities derived from ship list.
            \param shipList Ship List */
        void addShipRacialAbilities(const ShipList& shipList);

        /** Add abilities derived from configuration.
            \param config Host configuration
            \param fmt Number formatter
            \param tx Translator */
        void addConfigRacialAbilities(const game::config::HostConfiguration& config, util::NumberFormatter fmt, afl::string::Translator& tx);

        /** Add abilities derived from advantages.
            \param advList Advantage list */
        void addAdvantages(const AdvantageList& advList);

        /** Filter players.
            Keeps only abilities that are available to a player in the given set.
            \param players Players to keep */
        void filterPlayers(PlayerSet_t players);

        /** Get number of abilities.
            \return number */
        size_t size() const;

        /** Get ability by index.
            \param index Index [0,size())
            \return ability; null if index is out-of-range */
        const Ability* get(size_t index) const;

        /** Get iterator to beginning.
            \return iterator */
        Iterator_t begin() const;

        /** Get iterator to one-past-end.
            \return iterator */
        Iterator_t end() const;

     private:
        class ConfigBuilder;

        Abilities_t m_data;
    };

    /** Format RacialAbilityList::Category to human-readable string.
        \param cat Value to format
        \param tx Translator
        \return Human-readable value */
    String_t toString(RacialAbilityList::Category cat, afl::string::Translator& tx);

    /** Format RacialAbilityList::Origin to human-readable string.
        \param origin Value to format
        \param tx Translator
        \return Human-readable value */
    String_t toString(RacialAbilityList::Origin origin, afl::string::Translator& tx);

} }

#endif
