/**
  *  \file server/host/file/toolitem.cpp
  */

#include "server/host/file/toolitem.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/baseclient.hpp"
#include "server/host/file/fileitem.hpp"

using server::interface::BaseClient;
using server::interface::FileBaseClient;

server::host::file::ToolItem::ToolItem(Session& session, afl::net::CommandHandler& filer, String_t name, String_t pathName, String_t title, afl::base::Optional<String_t> restriction)
    : m_session(session),
      m_filer(filer),
      m_name(name),
      m_pathName(pathName),
      m_title(title),
      m_restriction(restriction)
{ }

String_t
server::host::file::ToolItem::getName()
{
    return m_name;
}

server::host::file::Item::Info_t
server::host::file::ToolItem::getInfo()
{
    Info_t i;
    i.name = m_name;
    i.type = server::interface::FileBase::IsDirectory;
    i.toolName = m_title.empty() ? m_name : m_title;
    i.label = server::interface::HostFile::ToolLabel;
    return i;
}

server::host::file::Item*
server::host::file::ToolItem::find(const String_t& name)
{
    // FIXME: use an optimistic "file" implementation that directly hits the filer without prior LS?
    return defaultFind(name);
}

void
server::host::file::ToolItem::listContent(ItemVector_t& out)
{
    // List content
    if (const String_t* p = m_restriction.get()) {
        afl::data::StringList_t filter;
        String_t acc = *p;
        String_t ele;
        while (afl::string::strSplit(acc, ele, acc, ",")) {
            filter.push_back(ele);
        }
        if (!acc.empty()) {
            filter.push_back(acc);
        }
        FileItem::listFileServerContent(m_filer, m_pathName, m_session.getUser(), filter, out);
    } else {
        FileItem::listFileServerContent(m_filer, m_pathName, m_session.getUser(), out);
    }
}

String_t
server::host::file::ToolItem::getContent()
{
    return defaultGetContent();
}
