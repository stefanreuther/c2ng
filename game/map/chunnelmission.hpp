/**
  *  \file game/map/chunnelmission.hpp
  */
#ifndef C2NG_GAME_MAP_CHUNNELMISSION_HPP
#define C2NG_GAME_MAP_CHUNNELMISSION_HPP

#include "game/unitscoredefinitionlist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/root.hpp"

namespace game { namespace map {

    class Ship;
    class Universe;

    class ChunnelMission {
     public:
        /* Failure modes. Note that the sequence of these values is used in check(). */
        enum {
            chf_MateDamaged = 1,
            chf_MateMoving  = 2,
            chf_MateTowed   = 4,
            chf_MateFuel    = 8,
            chf_MateAny     = 15,

            chf_Damaged     = 16,
            chf_Moving      = 32,
            chf_Towed       = 64,
            chf_Fuel        = 128,
            chf_Training    = 256,
            chf_SelfAny     = 496,

            chf_Distance    = 512
        };

        /* Kinds */
        enum {
            chk_Self   = 1,
            chk_Others = 2
        };

        ChunnelMission();

        bool check(const Ship& sh, const Universe& univ,
                   const UnitScoreDefinitionList& scoreDefinitions,
                   const game::spec::ShipList& shipList,
                   const Root& root);

        bool isValid() const;
        int getTargetId() const;
        int getFailureReasons() const;
        int getChunnelType() const;

     private:
        int m_target;
        int m_failure;
        int m_kind;

        static int checkChunnelFailures(const Ship& sh, const Universe& univ, const int minFuel, const Root& root);
    };

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
