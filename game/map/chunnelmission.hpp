/**
  *  \file game/map/chunnelmission.hpp
  *  \brief Class game::map::ChunnelMission and related functions
  */
#ifndef C2NG_GAME_MAP_CHUNNELMISSION_HPP
#define C2NG_GAME_MAP_CHUNNELMISSION_HPP

#include "afl/data/stringlist.hpp"
#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {

    class Ship;
    class Universe;
    class Configuration;

    /** Chunnel mission parser.
        Contains information about a chunnel mission.
        Use check() to populate it. */
    class ChunnelMission {
     public:
        /** Failure modes. Note that the sequence of these values is used in check(). */
        enum {
            chf_MateDamaged = 1,       ///< Mate is damaged.
            chf_MateMoving  = 2,       ///< Mate is moving.
            chf_MateTowed   = 4,       ///< Mate is being towed.
            chf_MateFuel    = 8,       ///< Mate is lacking fuel.
            chf_MateAny     = 15,      ///< Shortcut for any Mate problem.

            chf_Damaged     = 16,      ///< Initiator is damaged.
            chf_Moving      = 32,      ///< Initiator is moving.
            chf_Towed       = 64,      ///< Initiator is being towed.
            chf_Fuel        = 128,     ///< Initiator is lacking fuel.
            chf_Training    = 256,     ///< Initiator is training.
            chf_SelfAny     = 496,     ///< Shortcut for any Initiator problem.

            chf_Distance    = 512      ///< Distance is too small.
        };

        /** Chunnel kinds. */
        enum {
            chk_Self   = 1,            ///< Initiator will chunnel itself.
            chk_Others = 2             ///< Initiator will chunnel other ships.
        };

        /** Constructor.
            Makes a blank object. */
        ChunnelMission();

        /** Parse a ship's chunnel mission.
            Checks for possible chunnel attempts, even if that fails.
            A possible chunnel attempt is defined as a ship that can initiate a chunnel
            having its friendly code set to the Id of a ship that can receive a chunnel.

            If a chunnel attempt is detected, all attributes will be set.

            \param sh                Ship to check
            \param univ              Containing universe
            \param scoreDefinitions  Ship score definitions (required for experience/hull functions)
            \param shipList          Ship list (required for hull functions)
            \param root              Root (required for host config/version)

            \return true if possible chunnel attempt detected */
        bool check(const Ship& sh, const Universe& univ,
                   const UnitScoreDefinitionList& scoreDefinitions,
                   const game::spec::ShipList& shipList,
                   const Root& root);

        /** Check validity.
            \return result of last check() */
        bool isValid() const;

        /** Get target (mate) Id.
            \return Id if last check() succeeded; otherwise, 0. */
        int getTargetId() const;

        /** Get failure reasons.
            \return Combination of failure reasons (chf_XXX) if last check() succeeded. Zero means we expect success.
            If check() failed, the return value is meaningless (0). */
        int getFailureReasons() const;

        /** Get chunnel type.
            \return Combination of chunnel kinds (chk_XXX) if last check() succeeded; otherwise, 0. */
        int getChunnelType() const;

     private:
        int m_target;
        int m_failure;
        int m_kind;

        static int checkChunnelFailures(const Ship& sh, const Universe& univ, const int minFuel, const Root& root);
    };

    /** Format failure reasons into list of strings.
        \param failure Return value of ChunnelMission::getFailureReasons().
        \param tx Translator
        \return list of failures as strings. Empty if no failures. */
    afl::data::StringList_t formatChunnelFailureReasons(int failures, afl::string::Translator& tx);

    /** Check validity of a chunnel mate.

        \param initiator         Initiator
        \param mate              Potential mate
        \param mapConfig         Starchart geometry configuration (used for distance computations)
        \param root              Root (used for host version, configuration)
        \param shipScores        Ship score definitions (required for experience/hull functions)
        \param shipList          Ship list (required for hull functions)

        \retval true Should offer to set up a channel from Initiator to Potential mate
        \retval false Chunnel is impossible */
    bool isValidChunnelMate(const Ship& initiator,
                            const Ship& mate,
                            const Configuration& mapConfig,
                            const Root& root,
                            const UnitScoreDefinitionList& shipScores,
                            const game::spec::ShipList& shipList);

    /** Set up a chunnel.
        This sets up the chunnel, but does not verify it preconditions.

        \param initiator         Initiator
        \param mate              Mate
        \param univ              Universe
        \param config            Host configuration (required for fleet operations)
        \param shipList          Ship list (required for fleet operations) */
    void setupChunnel(Ship& initiator, Ship& mate, Universe& univ,
                      const game::config::HostConfiguration& config,
                      const game::spec::ShipList& shipList);

} }

inline bool
game::map::ChunnelMission::isValid() const
{
    return m_kind != 0;
}
inline int
game::map::ChunnelMission::getTargetId() const
{
    return m_target;
}

inline int
game::map::ChunnelMission::getFailureReasons() const
{
    return m_failure;
}

inline int
game::map::ChunnelMission::getChunnelType() const
{
    return m_kind;
}

#endif
