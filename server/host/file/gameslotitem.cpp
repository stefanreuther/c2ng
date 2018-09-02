/**
  *  \file server/host/file/gameslotitem.cpp
  *  \brief Class server::host::file::GameSlotItem
  */

#include "server/host/file/gameslotitem.hpp"
#include "afl/string/format.hpp"
#include "server/interface/filebase.hpp"
#include "server/host/file/fileitem.hpp"


server::host::file::GameSlotItem::GameSlotItem(Session& session, Root& root, int32_t gameId, int slotId, afl::base::Optional<String_t> slotName)
    : m_session(session), m_root(root), m_gameId(gameId), m_slotId(slotId),
      m_slotName(slotName)
{ }

String_t
server::host::file::GameSlotItem::getName()
{
    return afl::string::Format("%d", m_slotId);
}

server::host::file::Item::Info_t
server::host::file::GameSlotItem::getInfo()
{
    // Access game. Permission/existance checks have been done by creator of the GameSlotItem.
    GameArbiter::Guard guard(m_root.arbiter(), m_gameId, GameArbiter::Simple);
    Game game(m_root, m_gameId, Game::NoExistanceCheck);

    Info_t i;
    i.name     = getName();
    i.type     = server::interface::FileBase::IsDirectory;
    i.slotId   = m_gameId;
    i.slotName = m_slotName;
    i.label    = server::interface::HostFile::SlotLabel;

    return i;
}

server::host::file::Item*
server::host::file::GameSlotItem::find(const String_t& name)
{
    return defaultFind(name);
}

void
server::host::file::GameSlotItem::listContent(ItemVector_t& out)
{
    // Content is: current output files, turn file
    afl::net::CommandHandler& filer = m_root.hostFile();
    GameArbiter::Guard guard(m_root.arbiter(), m_gameId, GameArbiter::Simple);
    Game game(m_root, m_gameId, Game::NoExistanceCheck);
    String_t dirName = game.getDirectory();

    // List output files
    FileItem::listFileServerContent(filer, afl::string::Format("%s/out/%d", dirName, m_slotId), m_session.getUser(), out);

    // Add turn file, if any. The turn file folder is only accessible to admins.
    // (Players have write access to the 'new' folder, but no read access.)
    int turnStatus = game.getSlot(m_slotId).turnStatus().get();
    if (turnStatus != Game::TurnMissing && turnStatus != Game::TurnRed) {
        afl::data::StringList_t filter;
        filter.push_back(afl::string::Format("player%d.trn", m_slotId));
        FileItem::listFileServerContent(filer, afl::string::Format("%s/in", dirName), String_t(), filter, out);
    }
}

String_t
server::host::file::GameSlotItem::getContent()
{
    return defaultGetContent();
}
