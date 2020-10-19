/**
  *  \file game/historyturnlist.cpp
  *  \brief Class game::HistoryTurnList
  */

#include "game/historyturnlist.hpp"
#include "game/historyturn.hpp"
#include "game/score/turnscore.hpp"
#include "game/turnloader.hpp"

game::HistoryTurnList::HistoryTurnList()
    : m_turns()
{ }

game::HistoryTurnList::~HistoryTurnList()
{ }

game::HistoryTurn*
game::HistoryTurnList::get(int nr) const
{
    return m_turns[nr];
}

game::HistoryTurn*
game::HistoryTurnList::create(int nr)
{
    HistoryTurn* result = m_turns[nr];
    if (result == 0 && nr > 0) {
        result = new HistoryTurn(nr);
        m_turns.insertNew(nr, result);
    }
    return result;
}

int
game::HistoryTurnList::findNewestUnknownTurnNumber(int currentTurn) const
{
    afl::container::PtrMap<int, HistoryTurn>::iterator i = m_turns.end(), beg = m_turns.begin();
    int lastKnown = currentTurn;
    while (i != beg) {
        --i;
        if (HistoryTurn* t = i->second) {
            if (t->getTurnNumber() < lastKnown - 1) {
                return lastKnown - 1;
            }
            if (t->getStatus() == HistoryTurn::Unknown) {
                return t->getTurnNumber();
            }
            lastKnown = t->getTurnNumber();
        }
    }
    return lastKnown - 1;
}

void
game::HistoryTurnList::initFromTurnScores(const game::score::TurnScoreList& scores, int turn, int count)
{
    for (int i = 0; i < count; ++i, ++turn) {
        if (const game::score::TurnScore* score = scores.getTurn(turn)) {
            if (HistoryTurn* p = create(turn)) {
                if (p->getStatus() != HistoryTurn::Loaded) {
                    p->setTimestamp(score->getTimestamp());
                }
            }
        }
    }
}

void
game::HistoryTurnList::initFromTurnLoader(TurnLoader& loader, Root& root, int player, int turn, int count)
{
    // FIXME: use the query-many-at-once capability?
    for (int i = 0; i < count; ++i, ++turn) {
        if (HistoryTurn* p = create(turn)) {
            if (p->getStatus() == HistoryTurn::Unknown) {
                TurnLoader::HistoryStatus st[1] = {TurnLoader::Negative};
                loader.getHistoryStatus(player, turn, st, root);
                switch (st[0]) {
                 case TurnLoader::Negative:
                    p->setStatus(HistoryTurn::Unavailable);
                    break;
                 case TurnLoader::WeaklyPositive:
                    p->setStatus(HistoryTurn::WeaklyAvailable);
                    break;
                 case TurnLoader::StronglyPositive:
                    p->setStatus(HistoryTurn::StronglyAvailable);
                    break;
                }
            }
        }
    }
}

game::HistoryTurn::Status
game::HistoryTurnList::getTurnStatus(int turn) const
{
    if (const HistoryTurn* p = get(turn)) {
        return p->getStatus();
    } else {
        return HistoryTurn::Unknown;
    }
}

game::Timestamp
game::HistoryTurnList::getTurnTimestamp(int turn) const
{
    if (const HistoryTurn* p = get(turn)) {
        return p->getTimestamp();
    } else {
        return Timestamp();
    }
}
