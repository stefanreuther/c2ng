/**
  *  \file game/map/minefieldmission.hpp
  *  \brief Class game::map::MinefieldMission
  */
#ifndef C2NG_GAME_MAP_MINEFIELDMISSION_HPP
#define C2NG_GAME_MAP_MINEFIELDMISSION_HPP

#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/types.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {

    class Configuration;
    class Ship;
    class Universe;

    /** Minefield mission parameters.
        Stores parameters of a parsed "lay mines" or "scoop mines" mission. */
    class MinefieldMission {
     public:
        /** Default constructor.
            Makes an empty object. */
        MinefieldMission();

        /** Check for "lay mines" mission.
            \param ship        Ship to check
            \param univ        Universe (used for existing minefields)
            \param root        Root (used for host version, registration key, host configuration)
            \param mapConfig   Map configuration
            \param shipScores  Ship scores (used to determine availability of friendly codes)
            \param shipList    Ship list (used to determine availability of friendly codes
            \retval true Mission recognized; object has been updated
            \retval false No "lay mines" mission found; object unchanged */
        bool checkLayMission(const Ship& ship, const Universe& univ,
                             const Root& root,
                             const Configuration& mapConfig,
                             const UnitScoreDefinitionList& shipScores,
                             const game::spec::ShipList& shipList);

        /** Check for "lay mines" mission.
            \param ship        Ship to check
            \param univ        Universe (used for existing minefields)
            \param hostVersion Host version
            \param key         Registration key (osed to determine availability of friendly codes)
            \param mapConfig   Map configuration
            \param config      Host configuration
            \param shipScores  Ship scores (used to determine availability of friendly codes)
            \param shipList    Ship list (used to determine availability of friendly codes
            \retval true Mission recognized; object has been updated
            \retval false No "lay mines" mission found; object unchanged  */
        bool checkLayMission(const Ship& ship, const Universe& univ,
                             const HostVersion& hostVersion,
                             const RegistrationKey& key,
                             const Configuration& mapConfig,
                             const game::config::HostConfiguration& config,
                             const UnitScoreDefinitionList& shipScores,
                             const game::spec::ShipList& shipList);

        /** Check for "scoop mines" mission.
            \param ship        Ship to check
            \param root        Root (used for host version, registration key, host configuration)
            \param shipScores  Ship scores (used to determine availability of friendly codes)
            \param shipList    Ship list (used to determine availability of friendly codes
            \retval true Mission recognized; object has been updated
            \retval false No "lay mines" mission found; object unchanged  */
        bool checkScoopMission(const Ship& ship,
                               const Root& root,
                               const UnitScoreDefinitionList& shipScores,
                               const game::spec::ShipList& shipList);

        /** Get required minefield Id.
            - lay: this field shall be extended, preconditions (existence, position) have been verified. 0 to lay a new one.
            - scoop: this field shall be scooped, preconditions not verified. 0 to scoop all fields in range.
            \return Id or zero */
        Id_t getRequiredMinefieldId() const;

        /** Get minefield owner.
            - lay: new/extended minefield's owner.
            - scoop: owner of fields to scoop
            \return owner */
        int getMinefieldOwner() const;

        /** Get minefield type.
            - lay: true to lay/extend a web field, false to lay/extend normal field.
            - scoop: not relevant
            \return type */
        bool isWeb() const;

        /** Get number of torpedoes to lay/scoop.
            - lay: number of torpedoes to lay
            - scoop: maximum number of torpedoes to make. 0 for no limit.
            \return number */
        int getNumTorpedoes() const;

        /** Get number of units to lay.
            - lay: number of units produced
            - scoop: not relevant
            \return number */
        int32_t getNumUnits() const;

        /** Check whether ship's mission was used.
            \return true if mission was used */
        bool isMissionUsed() const;

        /** Check whether ship's friendly code was used.
            \return true if friendly code was used */
        bool isFriendlyCodeUsed() const;

     private:
        Id_t    m_mineId;                /**< Id of mine field to manipulate. 0 to accept any. */
        int     m_owner;                 /**< Required owner. */
        bool    m_isWeb;                 /**< Type of minefield. */
        int     m_numTorpedoes;          /**< Number of torpedoes. */
        int32_t m_numUnits;              /**< Number of mine units. */
        bool    m_usedMission;
        bool    m_usedFriendlyCode;
    };

} }

inline game::Id_t
game::map::MinefieldMission::getRequiredMinefieldId() const
{
    return m_mineId;
}

inline int
game::map::MinefieldMission::getMinefieldOwner() const
{
    return m_owner;
}

inline bool
game::map::MinefieldMission::isWeb() const
{
    return m_isWeb;
}

inline int
game::map::MinefieldMission::getNumTorpedoes() const
{
    return m_numTorpedoes;
}

inline int32_t
game::map::MinefieldMission::getNumUnits() const
{
    return m_numUnits;
}

inline bool
game::map::MinefieldMission::isMissionUsed() const
{
    return m_usedMission;
}

inline bool
game::map::MinefieldMission::isFriendlyCodeUsed() const
{
    return m_usedFriendlyCode;
}

#endif
