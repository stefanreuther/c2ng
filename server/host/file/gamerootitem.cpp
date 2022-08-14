/**
  *  \file server/host/file/gamerootitem.cpp
  *  \brief Class server::host::file::GameRootItem
  */

#include "server/host/file/gamerootitem.hpp"
#include "afl/string/parse.hpp"
#include "server/host/gamearbiter.hpp"
#include "server/host/file/gameitem.hpp"

server::host::file::GameRootItem::GameRootItem(const Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

String_t
server::host::file::GameRootItem::getName()
{
    return "game";
}

server::host::file::Item::Info_t
server::host::file::GameRootItem::getInfo()
{
    Info_t i;
    i.name = "game";
    i.type = server::interface::FileBase::IsDirectory;
    i.label = server::interface::HostFile::NoLabel;
    return i;
}

server::host::file::Item*
server::host::file::GameRootItem::find(const String_t& name)
{
    // Determine game Id
    int32_t gameId;
    if (!afl::string::strToInteger(name, gameId)) {
        return 0;
    }

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    return new GameItem(m_session, m_root, gameId);
}

void
server::host::file::GameRootItem::listContent(ItemVector_t& /*out*/)
{
    // Not listable
}

String_t
server::host::file::GameRootItem::getContent()
{
    return defaultGetContent();
}
