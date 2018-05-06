/**
  *  \file game/map/movementpredictor.hpp
  */
#ifndef C2NG_GAME_MAP_MOVEMENTPREDICTOR_HPP
#define C2NG_GAME_MAP_MOVEMENTPREDICTOR_HPP

#include "game/element.hpp"
#include "game/game.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/point.hpp"
#include "game/root.hpp"
#include "game/spec/cost.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace map {

    class Universe;
    class Ship;
    class ShipPredictor;

    class MovementPredictor {
     public:
        typedef game::spec::Cost Cargo_t;

        MovementPredictor();
        ~MovementPredictor();

        void computeMovement(const Universe& univ,
                             const Game& game,
                             const game::spec::ShipList& shipList,
                             const Root& root);

        bool getShipPosition(Id_t sid, Point& out) const;
        bool getShipCargo(Id_t sid, Cargo_t& out) const;

     private:
        enum Status {
            NonExisting,            // ship does not exist
            Normal,                 // ship moves normally
            Towing,                 // ship is towing another one
            Towed,                  // ship is being towed; do not move it
            ResolvingLoop,          // temporary state for resolving intercept loops
            Moved                   // ship has moved
        };
        struct Info {
            Status status : 8;
            Point pos;              // if Moved, current position. Otherwise: waypoint.
            Cargo_t cargo;

            Info(int)
                : status(NonExisting),
                  pos(0, 0),
                  cargo()
                { }
        };

        ObjectVector<Info> m_info;

        void init(const Universe& univ);
        void resolveTows(const Universe& univ);
        bool moveShips(const Universe& univ,
                       const Game& game,
                       const game::spec::ShipList& shipList,
                       const Root& root);

        Info* getInterceptTarget(const Ship& sh) const;
        static void copyCargo(Info& info, const Ship& sh, Cargo_t::Type infoElement, Element::Type shipElement);
        static void copyCargo(Info& info, const ShipPredictor& pred);
    };

} }

#endif
