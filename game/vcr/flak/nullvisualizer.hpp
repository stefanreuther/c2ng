/**
  *  \file game/vcr/flak/nullvisualizer.hpp
  *  \brief Class game::vcr::flak::NullVisualizer
  */
#ifndef C2NG_GAME_VCR_FLAK_NULLVISUALIZER_HPP
#define C2NG_GAME_VCR_FLAK_NULLVISUALIZER_HPP

#include "game/vcr/flak/visualizer.hpp"
#include "game/vcr/flak/battle.hpp"

namespace game { namespace vcr { namespace flak {

    /** Null visualizer.
        Ignores all calls. */
    class NullVisualizer : public Visualizer {
     public:
        virtual void updateTime(int32_t time);

        virtual void fireBeamFighterFighter(Object_t from, Object_t to, bool hits);
        virtual void fireBeamFighterShip(Object_t from, Ship_t to, bool hits);
        virtual void fireBeamShipFighter(Ship_t from, int beamNr, Object_t to, bool hits);
        virtual void fireBeamShipShip(Ship_t from, int beamNr, Ship_t to, bool hits);

        virtual void createFighter(Object_t id, const Position& pos, int player, Ship_t enemy);
        virtual void killFighter(Object_t id);
        virtual void landFighter(Object_t id);
        virtual void moveFighter(Object_t id, const Position& pos, Ship_t to);

        virtual void createFleet(Fleet_t fleetNr, int32_t x, int32_t y, int player, Ship_t firstShip, size_t numShips);
        virtual void setEnemy(Fleet_t fleetNr, Ship_t enemy);
        virtual void killFleet(Fleet_t fleetNr);
        virtual void moveFleet(Fleet_t fleetNr, int32_t x, int32_t y);

        virtual void createShip(Ship_t shipNr, const Position& pos, const ShipInfo& info);
        virtual void killShip(Ship_t shipNr);
        virtual void moveShip(Ship_t shipNr, const Position& pos);

        virtual void createTorpedo(Object_t id, const Position& pos, int player, Ship_t enemy);
        virtual void hitTorpedo(Object_t id, Ship_t shipNr);
        virtual void missTorpedo(Object_t id);
        virtual void moveTorpedo(Object_t id, const Position& pos);
    };

} } }

#endif
