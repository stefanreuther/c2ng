/**
  *  \file game/actions/preconditions.hpp
  *  \brief Preconditions
  */
#ifndef C2NG_GAME_ACTIONS_PRECONDITIONS_HPP
#define C2NG_GAME_ACTIONS_PRECONDITIONS_HPP

#include "game/map/ship.hpp"
#include "game/map/planet.hpp"
#include "game/session.hpp"

namespace game {
    class Turn;
}

namespace game { namespace actions {

    /** Check for a ship that is played.
        Throws if it is not.
        If this returns normally, the ship is played and has full data.
        \param ship Ship
        \param tx Translator (for error message)
        \return ship
        \throw game::Exception on precondition violation */
    void mustBePlayed(const game::map::Ship& ship, afl::string::Translator& tx);

    /** Check for a planet that is played.
        Throws if it is not.
        If this returns normally, the planet is played and has full data.
        \param planet Planet
        \param tx Translator (for error message)
        \return planet
        \throw game::Exception on precondition violation */
    void mustBePlayed(const game::map::Planet& planet, afl::string::Translator& tx);

    /** Check for a planet that has a base and is played.
        Throws if it is not.
        If this returns normally, the planet has a base, is played and has full data.
        \param planet Planet
        \param tx Translator (for error message)
        \return planet
        \throw game::Exception on precondition violation */
    void mustHavePlayedBase(const game::map::Planet& planet, afl::string::Translator& tx);

    /** Check for existing planet.
        Throws if the given Planet pointer is null.
        \param planet Planet
        \param tx Translator (for error message)
        \return *planet
        \throw game::Exception on precondition violation */
    game::map::Planet& mustExist(game::map::Planet* planet, afl::string::Translator& tx);

    /** Check for existing ship.
        Throws if the given Ship pointer is null.
        \param ship Ship
        \param tx Translator (for error message)
        \return *ship
        \throw game::Exception on precondition violation */
    game::map::Ship& mustExist(game::map::Ship* ship, afl::string::Translator& tx);

    /** Check for existing turn.
        Throws if the given Turn pointer is null.
        \param turn Turn
        \param tx Translator (for error message)
        \return *turn
        \throw game::Exception on precondition violation */
    Turn& mustExist(Turn* turn, afl::string::Translator& tx);

    /** Check for ship list.
        Returns the session's ship list; throws if it has none.
        \param session Session
        \return ship list
        \throw game::Exception on precondition violation */
    game::spec::ShipList& mustHaveShipList(game::Session& session);

    /** Check for Root.
        Returns the session's Root; throws if it has none.
        \param session Session
        \return Root
        \throw game::Exception on precondition violation */
    Root& mustHaveRoot(game::Session& session);

    /** Check for Game.
        Returns the session's Game; throws if it has none.
        \param session Session
        \return Game
        \throw game::Exception on precondition violation */
    Game& mustHaveGame(game::Session& session);

} }

#endif
