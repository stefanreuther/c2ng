/**
  *  \file server/host/file/gameitem.cpp
  *  \brief Class server::host::file::GameItem
  */

#include "server/host/file/gameitem.hpp"
#include "afl/string/format.hpp"
#include "server/errors.hpp"
#include "server/host/file/fileitem.hpp"
#include "server/host/file/gameslotitem.hpp"
#include "server/host/file/historyitem.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"

using server::interface::BaseClient;
using server::interface::FileBaseClient;

namespace {
    void checkPermissions(const server::host::Session& session, server::host::Game& game)
    {
        if (!session.isAdmin() && !game.isUserOnGame(session.getUser())) {
            throw std::runtime_error(server::PERMISSION_DENIED);
        }
    }
}


server::host::file::GameItem::GameItem(const Session& session, Root& root, int32_t gameId)
    : m_session(session),
      m_root(root),
      m_gameId(gameId)
{ }

String_t
server::host::file::GameItem::getName()
{
    return afl::string::Format("%s", m_gameId);
}

server::host::file::Item::Info_t
server::host::file::GameItem::getInfo()
{
    // Access game. Permission/existance checks have been done by creator of the GameItem.
    GameArbiter::Guard guard(m_root.arbiter(), m_gameId, GameArbiter::Simple);
    Game game(m_root, m_gameId, Game::NoExistanceCheck);

    Info_t i;
    i.name     = getName();
    i.type     = server::interface::FileBase::IsDirectory;
    i.gameId   = m_gameId;
    i.gameName = game.getName();
    i.label    = server::interface::HostFile::GameLabel;

    return i;
}

server::host::file::Item*
server::host::file::GameItem::find(const String_t& name)
{
    // FIXME: more optimisations? if the name directly indicates a slot or file, address it directly
    if (name == "history") {
        // Special-casing "history" speeds up host/50_gamerootitem from 7.5 -> 4.1 seconds (-45%).
        // Need to manually check permissions though.
        GameArbiter::Guard guard(m_root.arbiter(), m_gameId, GameArbiter::Simple);
        Game game(m_root, m_gameId, Game::NoExistanceCheck);
        checkPermissions(m_session, game);
        return new HistoryItem(m_session, m_root, m_gameId);
    } else {
        return defaultFind(name);
    }
}

void
server::host::file::GameItem::listContent(ItemVector_t& out)
{
    // Content is: current output files, folders for accessible slots, 'history'
    GameArbiter::Guard guard(m_root.arbiter(), m_gameId, GameArbiter::Simple);
    Game game(m_root, m_gameId, Game::NoExistanceCheck);
    checkPermissions(m_session, game);

    // List output files
    // FIXME: listFileServerContent will check the user against /out/all's permissions.
    //   This means that a user who has left the game (but still satisfies isUserOnGame()) will be refused listing this directory.
    //   As of 20180617, ex-players still cannot access games even if we disable here.
    FileItem::listFileServerContent(m_root.hostFile(), game.getDirectory() + "/out/all", m_session.getUser(), out);

    // List accessible slots
    server::common::RaceNames raceNames;
    game.loadRaceNames(raceNames, m_root);
    for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
        if (game.isSlotInGame(i)) {
            afl::data::StringList_t players;
            game.getSlot(i).players().getAll(players);
            if (m_session.isAdmin() || std::find(players.begin(), players.end(), m_session.getUser()) != players.end()) {
                afl::base::Optional<String_t> slotName;
                if (const String_t* pName = raceNames.longNames().at(i)) {
                    slotName = *pName;
                }
                out.pushBackNew(new GameSlotItem(m_session, m_root, m_gameId, i, slotName));
            }
        }
    }

    // List history
    out.pushBackNew(new HistoryItem(m_session, m_root, m_gameId));
}

String_t
server::host::file::GameItem::getContent()
{
    return defaultGetContent();
}
