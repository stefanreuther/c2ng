/**
  *  \file game/ref/sortby.hpp
  *  \brief Class game::ref::SortBy - Sort Predicates
  */
#ifndef C2NG_GAME_REF_SORTBY_HPP
#define C2NG_GAME_REF_SORTBY_HPP

#include "afl/string/translator.hpp"
#include "game/battleorderrule.hpp"
#include "game/game.hpp"
#include "game/map/movementpredictor.hpp"
#include "game/map/universe.hpp"
#include "game/playerlist.hpp"
#include "game/ref/sortpredicate.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace ref {

    /** Sort predicates.
        This class serves as a container for all common SortPredicate implementations. */
    struct SortBy {
        /** Sort by Id.
            Unlike NullPredicate (which sorts by type first, i.e. first all ships, then all planets, etc.),
            this sorts by Id, i.e. first all objects with Id #1, then #2, etc.
            Does not support positions. */
        class Id : public SortPredicate {
         public:
            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;
        };

        /** Sort by name. */
        class Name : public SortPredicate {
         public:
            /** Constructor.
                @param Session  Session (resolve reference names) */
            explicit Name(Session& session);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            Session& m_session;
        };

        /** Sort by owner.
            Sort by owner numerically, provide owner names as class names. */
        class Owner : public SortPredicate {
         public:
            /** Constructor.
                @param univ     Universe (access objects)
                @param players  Player list (access player names)
                @param tx       Translator */
            Owner(const game::map::Universe& univ, const PlayerList& players, afl::string::Translator& tx);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            const PlayerList& m_players;
            afl::string::Translator& m_translator;
        };

        /** Sort by current position. */
        class Position : public SortPredicate {
         public:
            /** Constructor.
                @param univ     Universe (access objects)
                @param tx       Translator (for not-on-map objects) */
            Position(const game::map::Universe& univ, afl::string::Translator& tx);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            afl::string::Translator& m_translator;

            afl::base::Optional<game::map::Point> getPosition(const Reference& a) const;
        };

        /** Sort by next-turn position.
            Computes one turn prediction using game::map::MovementPredictor. */
        class NextPosition : public SortPredicate {
         public:
            /** Constructor.
                @param univ     Universe (access objects)
                @param game     Game (ship score definitions)
                @param shipList Ship list (hull functions etc.)
                @param root     Root (configuration, host version, registration)
                @param tx       Translator (for not-on-map objects) */
            NextPosition(const game::map::Universe& univ,
                         const Game& game,
                         const game::spec::ShipList& shipList,
                         const Root& root,
                         afl::string::Translator& tx);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            afl::string::Translator& m_translator;
            game::map::MovementPredictor m_predictor;

            afl::base::Optional<game::map::Point> getPosition(const Reference& a) const;
        };

        /** Sort by damage level.
            Sorts ships by numeric damage level; no class names. */
        class Damage : public SortPredicate {
         public:
            /** Constructor.
                @param univ Universe (access objects) */
            explicit Damage(const game::map::Universe& univ);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;

            int getDamage(const Reference& a) const;
        };

        /** Sort by mass.
            Sort ships by current total mass; no class names. */
        class Mass : public SortPredicate {
         public:
            /** Constructor.
                @param univ     Universe (access objects)
                @param shipList Ship list (component masses) */
            Mass(const game::map::Universe& univ, const game::spec::ShipList& shipList);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            const game::spec::ShipList& m_shipList;

            int getMass(const Reference& a) const;
        };

        /** Sort by hull mass.
            Sort ships by plain hull mass; no class names. */
        class HullMass : public SortPredicate {
         public:
            /** Constructor.
                @param univ     Universe (access objects)
                @param shipList Ship list (hulls) */
            HullMass(const game::map::Universe& univ, const game::spec::ShipList& shipList);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            const game::spec::ShipList& m_shipList;

            int getHullMass(const Reference& a) const;
        };

        /** Sort by hull type.
            Sorts by hull Id; provides hull names as class names. */
        class HullType : public SortPredicate {
         public:
            /** Constructor.
                @param univ     Universe (access objects)
                @param shipList Ship list (hulls)
                @param tx       Translator ("unknown") */
            HullType(const game::map::Universe& univ, const game::spec::ShipList& shipList, afl::string::Translator& tx);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            const game::spec::ShipList& m_shipList;
            afl::string::Translator& m_translator;
        };

        /** Sort by battle order.
            Provides groups of 100 as dividers (corresponding to FLAK fleets). */
        class BattleOrder : public SortPredicate {
         public:
            /** Constructor.
                @param univ     Universe (access objects)
                @param rule     BattleOrderRule to use
                @param tx       Translator */
            BattleOrder(const game::map::Universe& univ, BattleOrderRule rule, afl::string::Translator& tx);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            BattleOrderRule m_rule;
            afl::string::Translator& m_translator;

            int getBattleOrderValue(const Reference& a) const;
        };

        /** Sort by fleet membership.
            Fleet leaders will appear before their members. */
        class Fleet : public SortPredicate {
         public:
            /** Constructor.
                @param univ     Universe (access objects)
                @param tx       Translator */
            Fleet(const game::map::Universe& univ, afl::string::Translator& tx);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            afl::string::Translator& m_translator;

            int getFleetNumberKey(const Reference& a) const;
        };

        /** Sort by tow group.
            Ship(s) towing another ship will appear together, with the towers first,
            and the towee name as divider. */
        class TowGroup : public SortPredicate {
         public:
            /** Constructor.
                @param univ     Universe (access objects)
                @param tx       Translator */
            TowGroup(const game::map::Universe& univ, afl::string::Translator& tx);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            afl::string::Translator& m_translator;

            int getTowGroupKey(const Reference& a) const;
        };

        /** Sort by transfer target.
            Ships transfering to the same target will appear next to each other. */
        class TransferTarget : public SortPredicate {
         public:
            /** Constructor.
                @param univ Universe
                @param transporterId Transporter to check
                @param checkOther true to check the other transporter, too.
                                  Pass !HostVersion::hasParallelShipTransfers() here.
                @param tx Translator */
            TransferTarget(const game::map::Universe& univ,
                           game::map::Ship::Transporter transporterId,
                           bool checkOther,
                           afl::string::Translator& tx);

            // SortPredicate:
            virtual int compare(const Reference& a, const Reference& b) const;
            virtual String_t getClass(const Reference& a) const;

         private:
            const game::map::Universe& m_universe;
            game::map::Ship::Transporter m_transporterId;
            const bool m_checkOther;
            afl::string::Translator& m_translator;

            Reference getTarget(const Reference a) const;
        };
    };

} }

#endif
