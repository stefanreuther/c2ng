/**
  *  \file game/map/ufotype.cpp
  */

#include "game/map/ufotype.hpp"

game::map::UfoType::UfoType(Universe& univ)
    : m_universe(univ),
      m_ufos()
{
    // ex GUfoType::GUfoType
}

game::map::UfoType::~UfoType()
{
    // ex GUfoType::~GUfoType
}

game::map::Ufo*
game::map::UfoType::addUfo(int id, int type, int color)
{
    // ex GUfoType::getUfoById (sort-of)

    // Ufo color cannot be 0
    if (color == 0) {
        return 0;
    }

    // FIXME: binary search?
    UfoList_t::iterator i = m_ufos.begin();
    while (i != m_ufos.end() && (*i)->getId() < id) {
        ++i;
    }

    // Create the Ufo
    // (This function's interface allows to reject a creation request.)
    Ufo* result = 0;
    if (i == m_ufos.end()) {
        // does not exist, and has higher Id than we know so far
        result = m_ufos.pushBackNew(new Ufo(id));
    } else if ((*i)->getId() == id) {
        // does already exist
        result = *i;
    } else {
        // does not exist, have to insert in the middle
        result = m_ufos.insertNew(i, new Ufo(id));
    }

    if (result != 0) {
        result->setTypeCode(type);
        result->setColorCode(color);
    }
    return result;
}

void
game::map::UfoType::addMessageInformation(const game::parser::MessageInformation& info)
{
    namespace gp = game::parser;

    // Try to obtain Ufo object
    Ufo* existing = getUfoById(info.getObjectId());
    if (existing == 0) {
        // Does not exist. Do we have the essential information to create it?
        // type/color are essential for addUfo().
        int32_t type, color, x, y;
        if (info.getValue(gp::mi_Type, type) && info.getValue(gp::mi_UfoColor, color) && info.getValue(gp::mi_X, x) && info.getValue(gp::mi_Y, y)) {
            existing = addUfo(info.getObjectId(), type, color);
        }
    }

    // Assimilate data
    if (existing != 0) {
        existing->addMessageInformation(info);
    }
}

// /** Postprocess Ufos after loading. */
void
game::map::UfoType::postprocess(int turn)
{
    // ex GUfoType::postprocess
    // /* Convert util.dat wormholes into Ufos */
    // if (!wormholes.empty()) {
    //     /* There is no reproducible 1:1 correspondence between wormhole Ids
    //        and Ufo Ids. PHost allocates two wormhole slots for each type of
    //        wormhole, but unidirectional wormholes get just one Ufo slot. Hence,
    //        we assume that the sequence of Ufos matches the sequence of wormholes
    //        if there are some Ufos inside the reserved range. If there are no
    //        Ufos (because the player is using Dosplan), we generate the sequence
    //        internally. */
    //     // FIXME: this does not deal with multiple RSTs. They may even have
    //     // different registration status! Things to do to support multiple RSTs:
    //     // - sort/uniq wormholes
    //     // - track sources for Ufos/wormholes

    //     /* First, find first wormhole Ufo. */
    //     iterator i = getFirstAt(config.WormholeUFOsStartAt());
    //     while (i != end() && !i->isSeenThisTurn())
    //         ++i;

    //     /* Regular merging */
    //     while (!wormholes.empty() && i != end() && i->getId() < config.WormholeUFOsStartAt() + 200) {
    //         /* Merge */
    //         i->addWormholeData(wormholes.front(), false, univ.getTurnNumber());

    //         /* Advance to next Ufo; skip those that we didn't see this turn */
    //         do
    //             ++i;
    //         while (i != end() && !i->isSeenThisTurn());

    //         /* Drop wormhole */
    //         wormholes.pop_front();
    //     }

    //     /* Generating new Ufos */
    //     while (!wormholes.empty()) {
    //         getUfoById(wormholes.front().worm_id + config.WormholeUFOsStartAt()).addWormholeData(wormholes.front(), true, univ.getTurnNumber());
    //         wormholes.pop_front();
    //     }
    // }

    // Postprocessing. This updates guessed positions.
    for (UfoList_t::iterator i = m_ufos.begin(); i != m_ufos.end(); ++i) {
        (*i)->postprocess(turn);
    }

    // /* Connect wormhole Ufos */
    // for (iterator i = getFirstAt(config.WormholeUFOsStartAt());
    //      i != end() && i->getId() < config.WormholeUFOsStartAt() + 200;
    //      ++i)
    // {
    //     /* We can connect this Ufo with the next one if it has an even
    //        real_id, and the next one exists and has a one-higher Id */
    //     iterator n = i;
    //     if ((i->getRealId() & 1) == 0 && ++n != end() && n->getRealId() == i->getRealId()+1) {
    //         i->connectWith(*n);
    //     }
    // }
}


game::map::Ufo*
game::map::UfoType::getObjectByIndex(Id_t index)
{
    // ex GUfoType::getObjectByIndex, GUfoType::getUfoByIndex
    if (index > 0 && index <= Id_t(m_ufos.size()) && m_ufos[index-1]->isValid()) {
        return m_ufos[index-1];
    } else {
        return 0;
    }
}

game::map::Universe*
game::map::UfoType::getUniverseByIndex(Id_t /*index*/)
{
    // ex GUfoType::getUniverseByIndex
    return &m_universe;
}

game::Id_t
game::map::UfoType::getNextIndex(Id_t index) const
{
    // ex GUfoType::getNextIndex
    if (index < Id_t(m_ufos.size())) {
        return index+1;
    } else {
        return 0;
    }
}

game::Id_t
game::map::UfoType::getPreviousIndex(Id_t index) const
{
    // ex GUfoType::getPreviousIndex
    if (index == 0) {
        return static_cast<Id_t>(m_ufos.size());
    } else {
        return index-1;
    }
}

game::map::Ufo*
game::map::UfoType::getUfoById(int id)
{
    // FIXME: binary search?
    UfoList_t::iterator i = m_ufos.begin();
    while (i != m_ufos.end() && (*i)->getId() < id) {
        ++i;
    }

    if (i != m_ufos.end() && (*i)->getId() == id) {
        return *i;
    } else {
        return 0;
    }
}
