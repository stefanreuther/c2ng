/**
  *  \file game/v3/commandextra.cpp
  */

#include "game/v3/commandextra.hpp"
#include "game/extraidentifier.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/map/ship.hpp"
#include "game/map/planet.hpp"
#include "game/map/minefield.hpp"

namespace {
    const game::ExtraIdentifier<game::Turn, game::v3::CommandExtra> ID = {{}};
}

game::v3::CommandExtra::CommandExtra(Turn& parent)
    : m_parent(parent),
      m_commandContainers()
{ }

game::v3::CommandExtra::~CommandExtra()
{ }

game::v3::CommandContainer&
game::v3::CommandExtra::create(int player)
{
    CommandContainer* c = m_commandContainers[player];
    if (c == 0) {
        // Create new command container
        c = m_commandContainers.insertNew(player, new CommandContainer());

        // Hook events
        m_signalConnections.pushBackNew(new afl::base::SignalConnection(c->sig_commandChange.add(this, &CommandExtra::onCommandChange)));
    }
    return *c;
}

game::v3::CommandContainer*
game::v3::CommandExtra::get(int player) const
{
    return m_commandContainers[player];
}

game::v3::CommandExtra&
game::v3::CommandExtra::create(Turn& parent)
{
    CommandExtra* ex = parent.extras().get(ID);
    if (ex == 0) {
        ex = parent.extras().setNew(ID, new CommandExtra(parent));
    }
    return *ex;
}

game::v3::CommandExtra*
game::v3::CommandExtra::get(Turn& parent)
{
    return parent.extras().get(ID);
}

game::v3::CommandContainer*
game::v3::CommandExtra::get(Turn& parent, int player)
{
    CommandExtra* p = get(parent);
    if (p) {
        return p->get(player);
    } else {
        return 0;
    }
}

void
game::v3::CommandExtra::onCommandChange(Command& cmd, bool /*added*/)
{
    if (Id_t id = cmd.getAffectedShip()) {
        if (game::map::Ship* sh = m_parent.universe().ships().get(id)) {
            sh->markDirty();
        }
    }
    if (Id_t id = cmd.getAffectedPlanet()) {
        if (game::map::Planet* pl = m_parent.universe().planets().get(id)) {
            pl->markDirty();
        }
    }
    if (Id_t id = cmd.getAffectedMinefield()) {
        if (game::map::Minefield* mf = m_parent.universe().minefields().get(id)) {
            mf->markDirty();
        }
    }
}
