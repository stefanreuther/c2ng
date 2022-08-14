/**
  *  \file server/host/file/toolrootitem.cpp
  *  \brief Class server::host::file::ToolRootItem
  */

#include "server/host/file/toolrootitem.hpp"
#include "server/host/file/toolitem.hpp"
#include "server/interface/filebase.hpp"

server::host::file::ToolRootItem::ToolRootItem(const Session& session, afl::net::CommandHandler& filer, String_t name, const Root::ToolTree& tree, bool restricted)
    : m_session(session),
      m_filer(filer),
      m_name(name),
      m_tree(tree),
      m_restricted(restricted)
{ }

String_t
server::host::file::ToolRootItem::getName()
{
    return m_name;
}

server::host::file::Item::Info_t
server::host::file::ToolRootItem::getInfo()
{
    Info_t i;
    i.name = m_name;
    i.type = server::interface::FileBase::IsDirectory;
    i.label = server::interface::HostFile::NoLabel;
    return i;
}

server::host::file::Item*
server::host::file::ToolRootItem::find(const String_t& name)
{
    String_t pathName = m_tree.byName(name).stringField("path").get();
    if (!pathName.empty()) {
        afl::base::Optional<String_t> restriction;
        if (m_restricted) {
            restriction = m_tree.byName(name).stringField("files").get();
        }
        return new ToolItem(m_session, m_filer, name, pathName, m_tree.byName(name).stringField("description").get(), restriction);
    } else {
        return 0;
    }
}

void
server::host::file::ToolRootItem::listContent(ItemVector_t& /*out*/)
{
    // Not listable
}

String_t
server::host::file::ToolRootItem::getContent()
{
    return defaultGetContent();
}
