/**
  *  \file game/map/minefieldformula.hpp
  */
#ifndef C2NG_GAME_MAP_MINEFIELDFORMULA_HPP
#define C2NG_GAME_MAP_MINEFIELDFORMULA_HPP

#include <vector>
#include "game/hostversion.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/point.hpp"
#include "game/types.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace map {

    class Minefield;
    class Universe;
    class MinefieldMission;
    class Ship;

    struct MinefieldEffect {
        Point center;
        Id_t id;
        int32_t radiusChange;   // relative radius change
        int32_t newUnits;       // new number of units (from which the radius can be computed)
        int owner;
        int numTorps;
        bool isWeb;
        bool isEndangered;

        MinefieldEffect(Point center, Id_t id, int32_t radiusChange, int32_t newUnits, int owner, int numTorps, bool isWeb, bool isEndangered)
            : center(center), id(id), radiusChange(radiusChange), newUnits(newUnits), owner(owner), numTorps(numTorps), isWeb(isWeb), isEndangered(isEndangered)
            { }
    };

    typedef std::vector<MinefieldEffect> MinefieldEffects_t;

    bool isMinefieldEndangered(const Minefield& field, const Universe& univ, const HostVersion& host, const game::config::HostConfiguration& config);

    void computeMineLayEffect(MinefieldEffects_t& result, const MinefieldMission& mission, const Ship& ship, const Universe& univ, const Root& root);

    void computeMineScoopEffect(MinefieldEffects_t& result, const MinefieldMission& mission, const Ship& ship, const Universe& univ, const Root& root,
                                const game::spec::ShipList& shipList);

} }

#endif
