/**
  *  \file game/map/movementpredictor.hpp
  *  \brief Class game::map::MovementPredictor
  */
#ifndef C2NG_GAME_MAP_MOVEMENTPREDICTOR_HPP
#define C2NG_GAME_MAP_MOVEMENTPREDICTOR_HPP

#include "afl/base/optional.hpp"
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

    /** Movement prediction for universe-at-once.
        Resolves intercept and tow missions and computes movement for all ships in the proper order.
        Internally, uses ShipPredictor to resolve the individual ships. */
    class MovementPredictor {
     public:
        /** Shortcut type name. */
        typedef game::spec::Cost Cargo_t;

        /** Default constructor.
            Makes blank object.
            Call computeMovement() to fill it in. */
        MovementPredictor();

        /** Destructor. */
        ~MovementPredictor();

        /** Compute one turn of movement.
            Populates all predicted position and cargo information.
            \param univ     Universe to start with
            \param game     Game (required for shipScores)
            \param shipList Ship list
            \param root     Root (required for hostConfiguration, hostVersion, registrationKey) */
        void computeMovement(const Universe& univ,
                             const Game& game,
                             const game::spec::ShipList& shipList,
                             const Root& root);

        /** Get ship position.
            Call after computeMovement().
            \param [in]  sid  Ship Id
            \return position if known */
        afl::base::Optional<Point> getShipPosition(Id_t sid) const;

        /** Get ship cargo.
            Call after computeMovement().
            \param [in]  sid  Ship Id
            \param [out] out  Cargo
            \retval true success
            \retval false ship position not known, \c out not set */
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
