/**
  *  \file server/host/file/historyitem.cpp
  *  \brief Class server::host::file::HistoryItem
  */

#include "server/host/file/historyitem.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/pack.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "server/host/file/historyturnitem.hpp"

namespace {
    bool loadPrimaryPlayers(server::host::Game::Turn turn, game::PlayerArray<String_t>& out)
    {
        // Read on one database action
        afl::data::StringList_t tmp;
        turn.playerId().getAll(tmp);

        // Parse
        for (size_t i = 0, n = tmp.size(); i+1 < n; i += 2) {
            int32_t slot;
            if (afl::string::strToInteger(tmp[i], slot)) {
                out.set(slot, tmp[i+1]);
            }
        }

        // Result
        return !tmp.empty();
    }
}

class server::host::file::HistoryItem::Loader {
 public:
    Loader(Game& game, Root& root, const Session& session)
        : m_game(game), m_root(root), m_session(session),
          m_trustingPrimaries(), m_slotsAsPrimary()
        { init(); }

    HistoryTurnItem* create(int turnNr);

 private:
    void init();

    Game& m_game;
    Root& m_root;
    const Session& m_session;

    game::PlayerArray<String_t> m_trustingPrimaries;
    game::PlayerSet_t m_slotsAsPrimary;
};

server::host::file::HistoryTurnItem*
server::host::file::HistoryItem::Loader::create(int turnNr)
{
    Game::Turn t = m_game.turn(turnNr+1);

    game::PlayerSet_t turnAccess;
    if (!m_session.isAdmin()) {
        // User: offer matching slots
        game::PlayerArray<String_t> primaryPlayers;
        loadPrimaryPlayers(t, primaryPlayers);

        for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
            const String_t turnPrimary = primaryPlayers.get(slot);
            if (!turnPrimary.empty() && (turnPrimary == m_session.getUser() || turnPrimary == m_trustingPrimaries.get(slot))) {
                turnAccess += slot;
            }
        }
    } else {
        // Admin: offer all valid slots
        int16_t turnStatus[Game::NUM_PLAYERS];
        afl::base::Memory<int16_t>(turnStatus).fill(-1);
        afl::bits::unpackArray<afl::bits::Int16LE>(turnStatus, afl::string::toBytes(t.info().turnStatus().get()), -1);
        for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
            if (turnStatus[slot-1] != -1) {
                turnAccess += slot;
            }
        }
    }

    game::PlayerSet_t resultAccess = m_slotsAsPrimary + turnAccess;
    if (!resultAccess.empty()) {
        return new HistoryTurnItem(m_session, m_root, m_game.getId(), turnNr, resultAccess, turnAccess);
    } else {
        return 0;
    }
}

void
server::host::file::HistoryItem::Loader::init()
{
    /*
     *  A user is allowed to access a slot's result if
     *  ...they are primary player on that slot now
     *
     *     This means a player can access game history after taking over a slot.
     *
     *  A user is allowed to access a slot's turn and result if
     *  ...they are primary player on that slot then
     *  ...they are secondary player on that slot now, and primary player then is same as primary player now
     *
     *     This means a player can access their old turn files.
     *     They cannot access historic turn files after taking over a game.
     *     Historic turn files contain other player's registration keys,
     *     so we allow access to those only if we see a relation of trust being established
     *     between the two players, by means of the "replacement/secondary" role.
     */

    // Determine slots we're primary player on, and trusting primaries where we're secondary
    game::PlayerArray<String_t> trustingPrimaries;
    game::PlayerSet_t slotsAsPrimary;
    if (!m_session.isAdmin()) {
        for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
            afl::data::StringList_t players;
            m_game.getSlot(slot).players().getAll(players);

            size_t n = 0;
            while (n < players.size() && players[n] != m_session.getUser()) {
                ++n;
            }

            if (n < players.size()) {
                if (n == 0) {
                    m_slotsAsPrimary += slot;
                } else {
                    m_trustingPrimaries.set(slot, players[0]);
                }
            }
        }
    }
}


/*
 *  HistoryItem
 */

server::host::file::HistoryItem::HistoryItem(const Session& session, Root& root, int32_t gameId)
    : m_session(session), m_root(root), m_gameId(gameId)
{ }

String_t
server::host::file::HistoryItem::getName()
{
    return "history";
}

server::host::file::Item::Info_t
server::host::file::HistoryItem::getInfo()
{
    Info_t i;
    i.name = getName();
    i.type = server::interface::FileBase::IsDirectory;
    i.label = server::interface::HostFile::HistoryLabel;
    return i;
}

server::host::file::Item*
server::host::file::HistoryItem::find(const String_t& name)
{
    // Directly parse the turn number.
    // Using direct parsing instead of defaultFind() drastically improves performance:
    //    c2systest host/50_gamerootitem: 10.5s -> 7.5s real time (-30%)
    //    testsuite ServerHostFile: 0.45 -> 0.41s real time (-10%)
    GameArbiter::Guard guard(m_root.arbiter(), m_gameId, GameArbiter::Simple);
    Game game(m_root, m_gameId, Game::NoExistanceCheck);
    Loader ldr(game, m_root, m_session);

    int turnNr;
    if (afl::string::strToInteger(name, turnNr)
        && turnNr >= 1
        && turnNr < game.turnNumber().get()
        && String_t(afl::string::Format("%d", turnNr)) == name)
    {
        return ldr.create(turnNr);
    } else {
        return 0;
    }
}

void
server::host::file::HistoryItem::listContent(ItemVector_t& out)
{
    // Content is: one directory per turn
    GameArbiter::Guard guard(m_root.arbiter(), m_gameId, GameArbiter::Simple);
    Game game(m_root, m_gameId, Game::NoExistanceCheck);
    Loader ldr(game, m_root, m_session);

    // Build result
    int turnNumber = game.turnNumber().get();
    for (int i = 1; i < turnNumber; ++i) {
        if (Item* p = ldr.create(i)) {
            out.pushBackNew(p);
        }
    }
}

String_t
server::host::file::HistoryItem::getContent()
{
    return defaultGetContent();
}
