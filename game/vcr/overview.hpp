/**
  *  \file game/vcr/overview.hpp
  *  \brief Class game::vcr::Overview
  */
#ifndef C2NG_GAME_VCR_OVERVIEW_HPP
#define C2NG_GAME_VCR_OVERVIEW_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/vcr/battle.hpp"
#include "game/vcr/database.hpp"

namespace game { namespace vcr {

    /** Combat Overview.
        Generate summaries over a set of combat recordings.

        Usage:
        - construct object, passing desired Database as parameter
        - retrieve desired summary

        Implemented summaries:
        - combat diagram

        TODO:
        - per-unit summary (public interface)
        - scores */
    class Overview {
     public:
        /** A unit's appearance. */
        struct Appearance {
            size_t firstIn;               ///< Battle this unit appears first in.
            size_t firstAs;               ///< Position (side) this unit appears first as.

            size_t lastIn;                ///< Battle this unit appears last in.
            size_t lastAs;                ///< Position (side) this unit appears last as.

            size_t num;                   ///< Number of appearances.

            Appearance(size_t in, size_t as)
                : firstIn(in), firstAs(as), lastIn(in), lastAs(as), num(1)
                { }
        };

        /** Information for a battle overview diagram.
            The battle overview diagram has
            - an axis with all units (struct Unit)
            - an axis with all battles (struct Battle)
            - markers at each place a unit fights in a battle (struct Participant) */
        struct Diagram {
            /** Unit axis definition. */
            struct Unit {
                int initialOwner;         ///< Initial owner of the unit.
                String_t name;            ///< Pre-formatted name of the unit.

                Unit()
                    : initialOwner(), name()
                    { }
                Unit(int initialOwner, String_t name)
                    : initialOwner(initialOwner), name(name)
                    { }
            };

            /** A participant of a battle. */
            struct Participant {
                size_t slot;              ///< Slot (=index into Unit list).
                int status;               ///< Status. 0 = unit survived/won, -1 = unit died, >0 = captured by...

                Participant()
                    : slot(), status()
                    { }
                Participant(size_t slot, int status)
                    : slot(slot), status(status)
                    { }
            };

            /** Battle axis definition. */
            struct Battle {
                String_t name;            ///< Name of battle.
                int status;               ///< Status. 0 = statemate, -1 = kill or non-unique captor, >0 = captured by...
                std::vector<Participant> participants; ///< Participants.

                Battle()
                    : name(), status(), participants()
                    { }
            };

            std::vector<Unit> units;      ///< Initial owners for all units.
            std::vector<Battle> battles;  ///< Result for all battles.
        };

        /** Internal representation. */
        struct Item {
            bool planet;             ///< true iff this item describes a planet.
            Id_t id;                 ///< Id of this object.
            Id_t groupId;            ///< Group Id.
            size_t sequence;         ///< Uniquifier to make sort stable.
            Appearance appears;

            Item(bool planet, Id_t id, Id_t groupId, size_t sequence, const Appearance& appears)
                : planet(planet), id(id), groupId(groupId), sequence(sequence), appears(appears)
                { }
        };


        /** Constructor.
            \param battles       Battles (non-const because this will compute battle results)
            \param config        Host configuration
            \param shipList      Ship list */
        Overview(Database& battles, const game::config::HostConfiguration& config, const game::spec::ShipList& shipList);
        ~Overview();

        /** Build diagram.
            \param [out] out      Diagram
            \param [in]  players  Player list (used for labeling battles)
            \param [in]  tx       Translator (used for labeling units, battles) */
        void buildDiagram(Diagram& out, const PlayerList& players, afl::string::Translator& tx) const;

     private:
        // Environment:
        Database& m_battles;
        const game::config::HostConfiguration& m_config;
        const game::spec::ShipList& m_shipList;

        // Status:
        std::vector<Item> m_units;
        Id_t m_groupCounter;

        // Helpers for producing output:
        void packUnits(std::vector<Diagram::Unit>& units, afl::string::Translator& tx) const;
        void packBattles(std::vector<Diagram::Battle>& out, const PlayerList& players, afl::string::Translator& tx) const;

        // Helpers for producing state:
        void addBattle(Battle& b, size_t index);
        void finish();
        std::vector<Item>::iterator findObject(const Object& obj);
        void renameGroup(int from, int to);
    };

} }

#endif
