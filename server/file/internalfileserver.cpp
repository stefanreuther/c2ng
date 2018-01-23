/**
  *  \file server/file/internalfileserver.cpp
  *  \brief Class server::file::InternalFileServer
  */

#include "server/file/internalfileserver.hpp"
#include "afl/io/internaldirectory.hpp"
#include "server/file/commandhandler.hpp"
#include "server/file/directoryhandler.hpp"

server::file::InternalFileServer::InternalFileServer()
    : m_rootDir("(root)"),
      m_rootDirItem("(root)", 0, std::auto_ptr<DirectoryHandler>(new InternalDirectoryHandler("(root)", m_rootDir))),
      m_root(m_rootDirItem, afl::io::InternalDirectory::create("(spec)")),
      m_session()
{ }

server::file::InternalFileServer::~InternalFileServer()
{ }

bool
server::file::InternalFileServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    return server::file::CommandHandler(m_root, m_session).handleCommand(upcasedCommand, args, result);
}
