/**
  *  \file server/host/file/historyturnitem.cpp
  *  \brief Class server::host::file::HistoryTurnItem
  */

#include "server/host/file/historyturnitem.hpp"
#include "afl/string/format.hpp"
#include "server/host/file/fileitem.hpp"
#include "server/host/file/historyslotitem.hpp"

server::host::file::HistoryTurnItem::HistoryTurnItem(const Session& session, Root& root, int32_t gameId, int turnNumber, game::PlayerSet_t resultAccess, game::PlayerSet_t turnAccess)
    : m_session(session), m_root(root), m_gameId(gameId), m_turnNumber(turnNumber),
      m_resultAccess(resultAccess), m_turnAccess(turnAccess)
{ }

String_t
server::host::file::HistoryTurnItem::getName()
{
    return afl::string::Format("%d", m_turnNumber);
}

server::host::file::Item::Info_t
server::host::file::HistoryTurnItem::getInfo()
{
    Info_t i;
    i.name = getName();
    i.type = server::interface::FileBase::IsDirectory;
    i.turnNumber = m_turnNumber;
    i.label = server::interface::HostFile::TurnLabel;
    return i;
}

server::host::file::Item*
server::host::file::HistoryTurnItem::find(const String_t& name)
{
    return defaultFind(name);
}

void
server::host::file::HistoryTurnItem::listContent(ItemVector_t& out)
{
    // Content is: one directory per slot, global files
    GameArbiter::Guard guard(m_root.arbiter(), m_gameId, GameArbiter::Simple);
    Game game(m_root, m_gameId, Game::NoExistanceCheck);

    // Race names
    server::common::RaceNames raceNames;
    game.loadRaceNames(raceNames, m_root);

    // Slots
    for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
        bool result = m_resultAccess.contains(i);
        bool turn = m_turnAccess.contains(i);
        if (result || turn) {
            out.pushBackNew(new HistorySlotItem(m_session, m_root, m_gameId, m_turnNumber, i, raceNames.longNames().get(i), result, turn));
        }
    }

    // Files
    afl::data::StringList_t fileNames;
    game.turn(m_turnNumber+1).files().globalFiles().getAll(fileNames);
    String_t pathName = afl::string::Format("%s/backup/pre-%03d", game.getDirectory(), m_turnNumber+1);
    FileItem::listFileServerContent(m_root.hostFile(), pathName, String_t(), fileNames, out);
}

String_t
server::host::file::HistoryTurnItem::getContent()
{
    return defaultGetContent();
}
