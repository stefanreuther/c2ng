/**
  *  \file game/vcr/classic/battle.hpp
  *  \brief Class game::vcr::classic::Battle
  */
#ifndef C2NG_GAME_VCR_CLASSIC_BATTLE_HPP
#define C2NG_GAME_VCR_CLASSIC_BATTLE_HPP

#include "game/vcr/battle.hpp"
#include "game/vcr/classic/types.hpp"
#include "game/vcr/object.hpp"
#include "game/vcr/score.hpp"
#include "afl/base/optional.hpp"

namespace game { namespace vcr { namespace classic {

    class Algorithm;
    class Visualizer;

    /** Classic battle.
        Represents a classic 1:1 fight.

        This is the union of PCC2's GClassicVcrEntry and GClassicVcr.

        Methods that compute battle results need ship list and configuration.
        These are passed in as parameters.
        This allows accessing battles with a different instance of ship list and configuration than it was created with
        (it must still be identical of course!).
        This allows rather painless multithreading for simulators,
        letting background simulators work on a background copy of the ship list and configuration
        without having to worry about mutual exclusion with the foreground thread. */
    class Battle : public game::vcr::Battle {
     public:
        /** Constructor.
            Use setType() to set the type of the fight.
            \param left Left unit (will be copied)
            \param right Right unit (will be copied)
            \param seed Random number seed
            \param signature Signature
            \param planetTemperatureCode Planet temperature code (FIXME: probably obsolete) */
        Battle(const Object& left,
               const Object& right,
               uint16_t seed,
               uint16_t signature,
               uint16_t planetTemperatureCode);

        /** Destructor. */
        ~Battle();

        /*
         *  game::vcr::Battle virtual methods
         */

        virtual size_t getNumObjects() const;
        virtual const Object* getObject(size_t slot, bool after) const;
        virtual int getOutcome(const game::config::HostConfiguration& config,
                               const game::spec::ShipList& shipList,
                               size_t slot);
        virtual Playability getPlayability(const game::config::HostConfiguration& config,
                                           const game::spec::ShipList& shipList);
        virtual void prepareResult(const game::config::HostConfiguration& config,
                                   const game::spec::ShipList& shipList,
                                   int resultLevel);
        virtual String_t getAlgorithmName(afl::string::Translator& tx) const;
        virtual bool isESBActive(const game::config::HostConfiguration& config) const;
        virtual bool getPosition(game::map::Point& result) const;

        /*
         *  Additional methods
         */

        /** Set battle type.
            \param type Type of battle (algorithm name)
            \param capabilities Capability flags received from creator of this fight */
        void setType(Type type, uint16_t capabilities);

        /** Get battle type. */
        Type getType() const;

        /** Set position.
            \param pos Position */
        void setPosition(game::map::Point pos);

        /** Get capabilities. */
        uint16_t getCapabilities() const;

        /** Get battle signature.
            This is exported through the script interface. */
        uint16_t getSignature() const;

        /** Get random number seed.
            This is exported through the script interface and of general interest for propellerheads. */
        uint16_t getSeed() const;

        /** Format current status as string.
            \param player Assume this player's point of view (0=neutral)
            \param annotation Optional annotation (points)
            \param tx Translator */
        String_t formatResult(int player, const String_t& annotation, afl::string::Translator& tx) const;

        /** Get result.
            FIXME: do we need this? Right now, it simplifies some things,
            but it violates our abstractions. */
        BattleResult_t getResult() const;

        // FIXME: need this method?
        // void setResultFromPlayer(VcrPlayer& player);

        /** Create a player algorithm that can play this battle.
            \param vis Visualizer to use
            \param config Configuration to use
            \param shipList Ship list to use
            \return newly-allocated Algorithm. Null if it cannot be created.
            Caller must call
            - setCapabilities()
            - checkBattle(), initBattle()
            - playCycle()
            - doneBattle()
            on it. */
        Algorithm* createAlgorithm(Visualizer& vis,
                                   const game::config::HostConfiguration& config,
                                   const game::spec::ShipList& shipList) const;

        /** Create a player algorithm for a given algorithm name
            \param type Algorithm name
            \param vis Visualizer to use
            \param config Configuration to use
            \param shipList Ship list to use
            \return newly-allocated Algorithm. Null if it cannot be created. */
        static Algorithm* createAlgorithmForType(Type type,
                                                 Visualizer& vis,
                                                 const game::config::HostConfiguration& config,
                                                 const game::spec::ShipList& shipList);

        /** Compute scores.
            \param score [in/out] Scores are added here
            \param side [in] Compute scores for this side
            \param config Configuration to use
            \param shipList Ship list to use */
        void computeScores(Score& score, Side side,
                           const game::config::HostConfiguration& config,
                           const game::spec::ShipList& shipList) const;

        /** Access left object.
            \return object */
        const Object& left() const
            { return m_before[0]; }

        /** Access right object.
            \return object */
        const Object& right() const
            { return m_before[1]; }

        /** Apply classic shield limits.
            Freighters do not have shields. */
        void applyClassicLimits();

     private:
        // Attributes from GClassicVcr:
        uint16_t m_seed;
        uint16_t m_signature;              // strictly speaking, redundant, but exported by script interface.
        uint16_t m_planetTemperatureCode;  // obsolete?
        Object m_before[2];
        Object m_after[2];

        // Attributes from GClassicVcrEntry:
        BattleResult_t m_result;
        Type m_type;
        uint16_t m_capabilities;

        afl::base::Optional<game::map::Point> m_position;
    };

} } }

#endif
