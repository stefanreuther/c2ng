/**
  *  \file game/actions/preconditions.hpp
  */
#ifndef C2NG_GAME_ACTIONS_PRECONDITIONS_HPP
#define C2NG_GAME_ACTIONS_PRECONDITIONS_HPP

#include "game/map/ship.hpp"
#include "game/map/planet.hpp"
#include "game/session.hpp"

namespace game { namespace actions {

    void mustBePlayed(const game::map::Ship& ship);
    void mustBePlayed(const game::map::Planet& planet);
    void mustHavePlayedBase(const game::map::Planet& planet);

    game::map::Planet& mustExist(game::map::Planet* planet);
    game::map::Ship& mustExist(game::map::Ship* ship);

    game::spec::ShipList& mustHaveShipList(game::Session& session);
    Root& mustHaveRoot(game::Session& session);
    Game& mustHaveGame(game::Session& session);

} }

#endif
