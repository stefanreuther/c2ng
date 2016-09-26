/**
  *  \file game/map/minefieldtype.cpp
  */

#include "game/map/minefieldtype.hpp"

game::map::MinefieldType::MinefieldType(Universe& univ)
    : ObjectVector<Minefield>(),
      ObjectVectorType<Minefield>(univ, *this),
      m_allMinefieldsKnown()
{ }

game::map::MinefieldType::~MinefieldType()
{ }

bool
game::map::MinefieldType::isValid(const Minefield& obj) const
{
    return obj.isValid();
}

void
game::map::MinefieldType::erase(Id_t id)
{
    // ex GMinefieldType::erase, sort-of
    if (Minefield* p = get(id)) {
        if (p->isValid()) {
            p->erase();
            sig_setChange.raise(0);
        }
    }
}

// /** Note that all minefields of a player are known with current data.
//     This means alternatively that if we have a minefield of this player in the history,
//     and did not scan it this turn, it is gone. This happens for Winplan result files
//     (KORE minefields). */
void
game::map::MinefieldType::setAllMinefieldsKnown(int player)
{
    // ex GMinefieldType::markAllMinefieldsKnown
    m_allMinefieldsKnown += player;
}

// /** Internal check/postprocess.
//     Postprocess all minefields and delete those that are gone.
//     \param turnNr current turn */
void
game::map::MinefieldType::internalCheck(int currentTurn, const game::HostVersion& host, const game::config::HostConfiguration& config)
{
    // ex GMinefieldType::internalCheck
    for (Id_t id = 1, n = size(); id <= n; ++id) {
        if (Minefield* mf = get(id)) {
            // If it's valid, update it
            if (mf->isValid()) {
                mf->internalCheck(currentTurn, host, config);
            }

            // Erase it if
            // - it reports gone anyway (clean up if it has an inconsistent state)
            // - it has no units remaining
            // - it was not seen this turn although we think it should
            int owner;
            if (!mf->getOwner(owner)
                || !mf->isValid()
                || mf->getUnits() == 0
                || (mf->getTurnLastSeen() < currentTurn && m_allMinefieldsKnown.contains(owner)))
            {
                mf->erase();
            }
        }
    }
}
