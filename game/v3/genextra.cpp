/**
  *  \file game/v3/genextra.cpp
  *  \brief Class game::v3::GenExtra
  */

#include "game/v3/genextra.hpp"

namespace {
    const game::ExtraIdentifier<game::Turn, game::v3::GenExtra> ID = {{}};
}

game::v3::GenExtra::GenExtra(Turn& parent)
    : m_parent(parent),
      m_genFiles()
{ }

game::v3::GenExtra::~GenExtra()
{ }

game::v3::GenFile&
game::v3::GenExtra::create(int player)
{
    GenFile* c = m_genFiles[player];
    if (c == 0) {
        c = m_genFiles.insertNew(player, new GenFile());
    }
    return *c;
}

game::v3::GenFile*
game::v3::GenExtra::get(int player) const
{
    return m_genFiles[player];
}

game::v3::GenExtra&
game::v3::GenExtra::create(Turn& parent)
{
    GenExtra* ex = parent.extras().get(ID);
    if (ex == 0) {
        ex = parent.extras().setNew(ID, new GenExtra(parent));
    }
    return *ex;
}

game::v3::GenExtra*
game::v3::GenExtra::get(Turn& parent)
{
    return parent.extras().get(ID);
}

const game::v3::GenExtra*
game::v3::GenExtra::get(const Turn& parent)
{
    return parent.extras().get(ID);
}

game::v3::GenFile*
game::v3::GenExtra::get(Turn& parent, int player)
{
    GenExtra* p = get(parent);
    if (p) {
        return p->get(player);
    } else {
        return 0;
    }
}

const game::v3::GenFile*
game::v3::GenExtra::get(const Turn& parent, int player)
{
    const GenExtra* p = get(parent);
    if (p) {
        return p->get(player);
    } else {
        return 0;
    }
}
