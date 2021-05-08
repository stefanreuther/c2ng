/**
  *  \file game/map/minefieldtype.cpp
  *  \brief Class game::map::MinefieldType
  */

#include "game/map/minefieldtype.hpp"

namespace {
    /* Maximum minefield Id.
       This code has no intrinsic limit.
       However, this limit means a rogue message cannot cause us to allocate unbounded memory.

       Host will use up to 500, PHost (optionally) up to 10000.
       Allowing 20000 means the ObjectVector will be at most 160k (on a 64-bit system). */
    const int MAX_MINEFIELD_ID = 20000;
}

game::map::MinefieldType::MinefieldType()
    : ObjectVector<Minefield>(),
      ObjectVectorType<Minefield>(static_cast<ObjectVector<Minefield>&>(*this)),
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
    // ex accessor.pas:DeleteMinefield, sort-of
    if (Minefield* p = get(id)) {
        if (p->isValid()) {
            p->erase(&sig_setChange);
        }
    }
}

void
game::map::MinefieldType::setAllMinefieldsKnown(int player)
{
    // ex GMinefieldType::markAllMinefieldsKnown
    m_allMinefieldsKnown += player;
}

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
            // - it was not scanned this turn although we think it should
            //   (a minefield may be laid but immediately be swept; in this case, it's gone.)
            int owner;
            if (!mf->getOwner(owner)
                || !mf->isValid()
                || mf->getUnits() == 0
                || (m_allMinefieldsKnown.contains(owner)
                    && (mf->getTurnLastSeen() < currentTurn
                        || mf->getReason() < Minefield::MinefieldScanned)))
            {
                mf->erase(0);
            }
        }
    }
}

void
game::map::MinefieldType::addMessageInformation(const game::parser::MessageInformation& info)
{
    // ex GMinefieldType::addMinefieldReport
    // ex accessor.pas:LookupMine (part)
    namespace gp = game::parser;

    // Range check
    if (info.getObjectId() <= 0 || info.getObjectId() > MAX_MINEFIELD_ID) {
        return;
    }
    Minefield* existing = get(info.getObjectId());

    // Find position
    Point pt;
    int32_t x, y;
    if (info.getValue(gp::mi_X, x) && info.getValue(gp::mi_Y, y)) {
        // New position
        pt = Point(x, y);
    } else if (existing != 0 && existing->getPosition(pt)) {
        // Keep old position
    } else {
        // No position known, cannot use this report
        return;
    }

    // Find owner
    int32_t owner;
    if (info.getValue(gp::mi_Owner, owner)) {
        // New owner
    } else if (existing != 0 && existing->getOwner(owner)) {
        // Keep old owner
    } else {
        // No owner known, cannot use this report
        return;
    }

    // Find size. A report without a size is useless and therefore ignored.
    int32_t size;
    Minefield::SizeReport sizeType;
    if (info.getValue(gp::mi_MineUnits, size)) {
        sizeType = Minefield::UnitsKnown;
    } else if (info.getValue(gp::mi_Radius, size)) {
        sizeType = Minefield::RadiusKnown;
    } else {
        // FIXME: can we deal with mi_MineUnitsRemoved?
        return;
    }

    // Find type
    int32_t rawType;
    Minefield::TypeReport type;
    if (info.getValue(gp::mi_Type, rawType)) {
        type = (rawType != 0 ? Minefield::IsWeb : Minefield::IsMine);
    } else {
        type = Minefield::UnknownType;
    }

    // Find reason
    int32_t rawReason;
    Minefield::ReasonReport reason;
    if (info.getValue(gp::mi_MineScanReason, rawReason)) {
        if (rawReason <= 0) {
            reason = Minefield::NoReason;
        } else if (rawReason == 1) {
            reason = Minefield::MinefieldLaid;
        } else if (rawReason == 2) {
            reason = Minefield::MinefieldSwept;
        } else {
            reason = Minefield::MinefieldScanned;
        }
    } else {
        reason = Minefield::MinefieldScanned;
    }

    // Process it:
    // If it's a report about an existing, still-unknown field, create it.
    // If it's a report about a now-nonexistant, known field, just add it; internalCheck() will clean up.
    if (existing == 0 && size > 0) {
        existing = create(info.getObjectId());
    }
    if (existing != 0) {
        existing->addReport(pt, owner, type, sizeType, size, info.getTurnNumber(), reason);
    }
}
