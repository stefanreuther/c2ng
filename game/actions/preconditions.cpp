/**
  *  \file game/actions/preconditions.cpp
  *  \brief Preconditions
  */

#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

void
game::actions::mustBePlayed(const game::map::Object& obj)
{
    // ex game/actions/preconditions.h:mustBePlayed
    if (!obj.isPlayable(game::map::Object::Playable)) {
        throw Exception(Exception::eNotPlaying);
    }
}

void
game::actions::mustHavePlayedBase(const game::map::Planet& planet)
{
    // ex game/actions/preconditions.h:mustHavePlayedBase
    mustBePlayed(planet);
    if (!planet.hasBase()) {
        throw Exception(Exception::eNoBase);
    }
}

game::map::Planet&
game::actions::mustExist(game::map::Planet* planet)
{
    if (planet == 0) {
        throw Exception(Exception::eRange);
    }
    return *planet;
}

game::map::Ship&
game::actions::mustExist(game::map::Ship* ship)
{
    if (ship == 0) {
        throw Exception(Exception::eRange);
    }
    return *ship;
}

game::spec::ShipList&
game::actions::mustHaveShipList(game::Session& session)
{
    game::spec::ShipList* sl = session.getShipList().get();
    if (sl == 0) {
        throw Exception(Exception::eUser);
    }
    return *sl;
}

game::Root&
game::actions::mustHaveRoot(game::Session& session)
{
    Root* r = session.getRoot().get();
    if (r == 0) {
        throw Exception(Exception::eUser);
    }
    return *r;
}

game::Game&
game::actions::mustHaveGame(game::Session& session)
{
    Game* g = session.getGame().get();
    if (g == 0) {
        throw Exception(Exception::eUser);
    }
    return *g;
}

game::Turn&
game::actions::mustBeLocallyEditable(Turn& t)
{
    if (t.getLocalDataPlayers().empty()) {
        throw Exception("Read-only");
    }
    return t;
}

game::Turn&
game::actions::mustAllowCommands(Turn& t, int forPlayer)
{
    if (!t.getCommandPlayers().contains(forPlayer)) {
        throw Exception("Read-only");
    }
    return t;
}
