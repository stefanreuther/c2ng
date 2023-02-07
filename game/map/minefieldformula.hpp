/**
  *  \file game/map/minefieldformula.hpp
  *  \brief Minefield Formulas
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

    class Configuration;
    class Minefield;
    class MinefieldMission;
    class Ship;
    class Universe;

    /** Representation of one minefield effect. */
    struct MinefieldEffect {
        Point center;           ///< Center of minefield.
        Id_t id;                ///< Minefield Id.
        int32_t radiusChange;   ///< Relative radius change (e.g. "+10").
        int32_t newUnits;       ///< New number of units. New radius can be computed from it.
        int32_t unitLimit;      ///< Maximum number of units.
        int owner;              ///< Minefield owner.
        int numTorps;           ///< Number of torpedoes laid/scooped.
        bool isWeb;             ///< true for Web Mines.
        bool isEndangered;      ///< true if minefield is in danger of being swept by enemy.

        MinefieldEffect(Point center, Id_t id, int32_t radiusChange, int32_t newUnits, int32_t unitLimit, int owner, int numTorps, bool isWeb, bool isEndangered)
            : center(center), id(id), radiusChange(radiusChange), newUnits(newUnits), unitLimit(unitLimit), owner(owner), numTorps(numTorps), isWeb(isWeb), isEndangered(isEndangered)
            { }
    };

    typedef std::vector<MinefieldEffect> MinefieldEffects_t;

    /** Check whether minefield is in danger of being swept by enemy.
        A minefield is in danger if
        (a) a foreign ship is close enough, or
        (b) a foreign, unowned or unknown planet is close enough, and we don't have a ship there.
        @param field      Minefield
        @param univ       Universe (for ships, planets)
        @param mapConfig  Map configuration (for distance computation)
        @param host       Host version (for HostVersion::isMineLayingAfterMineDecay())
        @param config     Host configuration (for mine sweep ranges)
        @return true if minefield is endangered */
    bool isMinefieldEndangered(const Minefield& field, const Universe& univ, const Configuration& mapConfig, const HostVersion& host, const game::config::HostConfiguration& config);

    /** Compute effect of mine laying mission.
        Adds a description of the effect to the given result.
        Mine laying produces one effect.

        If we are enlarging an existing minefield,
        this function assumes that MinefieldMission::checkLayMission() has already verified positions.

        @param [in,out] result     Effect added here
        @param [in]     mission    Parsed mission
        @param [in]     ship       Ship
        @param [in]     univ       Universe (for isMinefieldEndangered)
        @param [in]     mapConfig  Map configuration (for distance computation)
        @param [in]     root       Root (for host version, host configuration) */
    void computeMineLayEffect(MinefieldEffects_t& result, const MinefieldMission& mission, const Ship& ship, const Universe& univ, const Configuration& mapConfig, const Root& root);

    /** Compute effect of mine scoop mission.
        Adds a description of the effect to the given result.
        Mine scooping can produce multiple effects.

        @param [in,out] result     Effect added here
        @param [in]     mission    Parsed mission
        @param [in]     ship       Ship
        @param [in]     univ       Universe (for isMinefieldEndangered)
        @param [in]     mapConfig  Map configuration (for distance computation)
        @param [in]     root       Root (for host version, host configuration)
        @param [in]     shipList   Ship list (for ship cargo capacity) */
    void computeMineScoopEffect(MinefieldEffects_t& result, const MinefieldMission& mission, const Ship& ship, const Universe& univ, const Configuration& mapConfig, const Root& root,
                                const game::spec::ShipList& shipList);

} }

#endif
