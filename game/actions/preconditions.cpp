/**
  *  \file game/actions/preconditions.cpp
  *  \brief Preconditions
  */

#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "util/translation.hpp"
#include "game/root.hpp"
#include "game/game.hpp"

void
game::actions::mustBePlayed(const game::map::Ship& ship)
{
    // ex game/actions/preconditions.h:mustBePlayed
    if (!ship.isPlayable(game::map::Object::Playable)) {
        throw Exception(Exception::eNotPlaying, _("You do not play this ship"));
    }
}

void
game::actions::mustBePlayed(const game::map::Planet& planet)
{
    // ex game/actions/preconditions.h:mustBePlayed
    if (!planet.isPlayable(game::map::Object::Playable)) {
        throw Exception(Exception::eNotPlaying, _("You do not play this planet"));
    }
}

void
game::actions::mustHavePlayedBase(const game::map::Planet& planet)
{
    // ex game/actions/preconditions.h:mustHavePlayedBase
    mustBePlayed(planet);
    if (!planet.hasBase()) {
        throw Exception(Exception::eNoBase, _("The planet does not have a starbase"));
    }
}

game::map::Planet&
game::actions::mustExist(game::map::Planet* planet)
{
    if (planet == 0) {
        throw Exception(Exception::eRange, _("The planet does not exist"));
    }
    return *planet;
}

game::map::Ship&
game::actions::mustExist(game::map::Ship* ship)
{
    if (ship == 0) {
        throw Exception(Exception::eRange, _("The ship does not exist"));
    }
    return *ship;
}

game::spec::ShipList&
game::actions::mustHaveShipList(game::Session& session)
{
    game::spec::ShipList* sl = session.getShipList().get();
    if (sl == 0) {
        throw Exception(Exception::eUser, _("No ship list loaded"));
    }
    return *sl;
}

game::Root&
game::actions::mustHaveRoot(game::Session& session)
{
    Root* r = session.getRoot().get();
    if (r == 0) {
        throw Exception(Exception::eUser, _("No game configuration loaded"));
    }
    return *r;
}

game::Game&
game::actions::mustHaveGame(game::Session& session)
{
    Game* g = session.getGame().get();
    if (g == 0) {
        throw Exception(Exception::eUser, _("No game loaded"));
    }
    return *g;
}
