/**
  *  \file server/host/file/rootitem.cpp
  *  \brief Class server::host::file::RootItem
  */

#include "server/host/file/rootitem.hpp"
#include "server/host/file/gamerootitem.hpp"
#include "server/host/file/toolrootitem.hpp"

server::host::file::RootItem::RootItem(const Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

String_t
server::host::file::RootItem::getName()
{
    // This function's result is never passed to the user.
    return String_t();
}

server::host::file::Item::Info_t
server::host::file::RootItem::getInfo()
{
    // This function's result is never passed to the user, but serves as initialisation.
    return Info_t();
}

server::host::file::Item*
server::host::file::RootItem::find(const String_t& name)
{
    if (name == "shiplist") {
        return new ToolRootItem(m_session, m_root.hostFile(), name, m_root.shipListRoot(), false);
    } else if (name == "tool") {
        return new ToolRootItem(m_session, m_root.hostFile(), name, m_root.toolRoot(), true);
    } else if (name == "game") {
        return new GameRootItem(m_session, m_root);
    } else {
        return 0;
    }
}

void
server::host::file::RootItem::listContent(ItemVector_t& /*out*/)
{ }

String_t
server::host::file::RootItem::getContent()
{
    return defaultGetContent();
}
