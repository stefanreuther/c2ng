/**
  *  \file server/host/hostslot.cpp
  *  \brief Class server::host::HostSlot
  */

#include "server/host/hostslot.hpp"
#include "server/errors.hpp"
#include "server/host/game.hpp"
#include "server/host/gamearbiter.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"

server::host::HostSlot::HostSlot(const Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::host::HostSlot::add(int32_t gameId, afl::base::Memory<const int32_t> slotNrs)
{
    // Obtain critical access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::AdminPermission);

    // Master must not have run
    if (game.getConfigInt("masterHasRun") != 0) {
        throw std::runtime_error(WRONG_GAME_STATE);
    }

    // OK, do it
    while (const int32_t* p = slotNrs.eat()) {
        // Verify slot
        const int slotNr = *p;
        if (slotNr <= 0 || slotNr > Game::NUM_PLAYERS) {
            throw std::runtime_error(INVALID_VALUE);
        }

        // Create it
        Game::Slot slot(game.getSlot(slotNr));
        if (slot.slotStatus().get() == 0) {
            slot.slotStatus().set(1);
            slot.turnStatus().set(0);
        }
    }
}

void
server::host::HostSlot::remove(int32_t gameId, afl::base::Memory<const int32_t> slotNrs)
{
    // Obtain critical access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::AdminPermission);

    // Master must not have run
    if (game.getConfigInt("masterHasRun") != 0) {
        throw std::runtime_error(WRONG_GAME_STATE);
    }

    // OK, do it
    while (const int32_t* p = slotNrs.eat()) {
        // Verify slot. We accept out-of-range numbers because they trivially fulfill the post-condition.
        const int slotNr = *p;
        if (game.isSlotInGame(slotNr)) {
            // There must not be any player on this slot
            Game::Slot slot(game.getSlot(slotNr));
            if (!slot.players().empty()) {
                throw std::runtime_error(SLOT_NOT_AVAILABLE);
            }

            // Remove it
            slot.slotStatus().set(0);
            slot.turnStatus().set(0);
        }
    }
}

void
server::host::HostSlot::getAll(int32_t gameId, afl::data::IntegerList_t& result)
{
    // This function is similar in meaning to HostPlayer::list.
    // We list only the current slots, not slots that existed at the start of game but died now.
    // The HostSlot module is intended for pre-game slot manipulation where this does not make a difference.
    // HostPlayer::list would have an the 'all' flag.

    // Obtain simple access; read-only
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Generate result
    for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
        if (game.isSlotInGame(i)) {
            result.push_back(i);
        }
    }
}
