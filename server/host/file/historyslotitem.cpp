/**
  *  \file server/host/file/historyslotitem.cpp
  *  \brief Class server::host::file::HistorySlotItem
  */

#include "server/host/file/historyslotitem.hpp"
#include "afl/string/format.hpp"
#include "server/host/file/fileitem.hpp"

server::host::file::HistorySlotItem::HistorySlotItem(Session& session, Root& root, int32_t gameId, int turnNumber, int slotNumber, String_t slotName,
                                                     bool resultAccess, bool turnAccess)
    : m_session(session), m_root(root), m_gameId(gameId), m_turnNumber(turnNumber), m_slotNumber(slotNumber), m_slotName(slotName),
      m_resultAccess(resultAccess), m_turnAccess(turnAccess)
{ }

String_t
server::host::file::HistorySlotItem::getName()
{
    return afl::string::Format("%d", m_slotNumber);
}

server::host::file::Item::Info_t
server::host::file::HistorySlotItem::getInfo()
{
    Info_t i;
    i.name = getName();
    i.type = server::interface::FileBase::IsDirectory;
    i.slotId = m_slotNumber;
    i.slotName = m_slotName;
    i.label = server::interface::HostFile::SlotLabel;
    return i;
}

server::host::file::Item*
server::host::file::HistorySlotItem::find(const String_t& name)
{
    return defaultFind(name);
}

void
server::host::file::HistorySlotItem::listContent(ItemVector_t& out)
{
    // Content is: player files (if enabled), turn file (if enabled)
    GameArbiter::Guard guard(m_root.arbiter(), m_gameId, GameArbiter::Simple);
    Game game(m_root, m_gameId, Game::NoExistanceCheck);

    // Player files
    if (m_resultAccess) {
        afl::data::StringList_t fileNames;
        game.turn(m_turnNumber+1).files().playerFiles(m_slotNumber).getAll(fileNames);
        String_t pathName = afl::string::Format("%s/backup/pre-%03d", game.getDirectory(), m_turnNumber+1);
        FileItem::listFileServerContent(m_root.hostFile(), pathName, String_t(), fileNames, out);
    }

    // Turn file
    if (m_turnAccess) {
        afl::data::StringList_t fileNames;
        fileNames.push_back(afl::string::Format("player%d.trn", m_slotNumber));
        String_t pathName = afl::string::Format("%s/backup/trn-%03d", game.getDirectory(), m_turnNumber+1);
        FileItem::listFileServerContent(m_root.hostFile(), pathName, String_t(), fileNames, out);
    }
}

String_t
server::host::file::HistorySlotItem::getContent()
{
    return defaultGetContent();
}
