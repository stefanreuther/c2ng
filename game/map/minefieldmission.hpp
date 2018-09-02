/**
  *  \file game/map/minefieldmission.hpp
  */
#ifndef C2NG_GAME_MAP_MINEFIELDMISSION_HPP
#define C2NG_GAME_MAP_MINEFIELDMISSION_HPP

#include "game/types.hpp"
#include "game/spec/shiplist.hpp"
#include "game/root.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {

    class Ship;
    class Universe;

    class MinefieldMission {
     public:
        MinefieldMission();

        bool checkLayMission(const Ship& ship, const Universe& univ,
                             const Root& root,
                             const UnitScoreDefinitionList& shipScores,
                             const game::spec::ShipList& shipList);
        bool checkScoopMission(const Ship& ship,
                               const Root& root,
                               const UnitScoreDefinitionList& shipScores,
                               const game::spec::ShipList& shipList);

        Id_t getRequiredMinefieldId() const;
        int getMinefieldOwner() const;
        bool isWeb() const;
        int getNumTorpedoes() const;
        int32_t getNumUnits() const;
        bool isMissionUsed() const;
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
