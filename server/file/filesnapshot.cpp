/**
  *  \file server/file/filesnapshot.cpp
  *  \brief Class server::file::FileSnapshot
  */

#include <stdexcept>
#include "server/file/filesnapshot.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/directoryitem.hpp"
#include "afl/string/char.hpp"
#include "server/errors.hpp"

namespace {
    bool hasInvalidCharacter(const String_t& name)
    {
        for (size_t i = 0; i < name.size(); ++i) {
            char ch = name[i];
            if (afl::string::charIsAlphanumeric(ch) || ch == '-' || ch == '.' || ch == '_' || ch == '+') {
                // ok
            } else {
                return true;
            }
        }
        return false;
    }
}

server::file::FileSnapshot::FileSnapshot(Session& session, Root& root)
    : m_session(session), m_root(root)
{ }

void
server::file::FileSnapshot::createSnapshot(String_t name)
{
    m_session.checkAdmin();
    verifyName(name);
    handler().createSnapshot(name);
}

void
server::file::FileSnapshot::copySnapshot(String_t oldName, String_t newName)
{
    m_session.checkAdmin();
    verifyName(oldName);
    verifyName(newName);

    DirectoryHandler::SnapshotHandler& hdl = handler();
    try {
        // The message of the exception thrown by copySnapshot is unspecified; we want to guarantee a 404 response.
        hdl.copySnapshot(oldName, newName);
    }
    catch (std::exception&) {
        throw std::runtime_error(SNAPSHOT_NOT_FOUND);
    }
}

void
server::file::FileSnapshot::removeSnapshot(String_t name)
{
    m_session.checkAdmin();
    verifyName(name);
    handler().removeSnapshot(name);
}

void
server::file::FileSnapshot::listSnapshots(afl::data::StringList_t& out)
{
    m_session.checkAdmin();
    handler().listSnapshots(out);
}

server::file::DirectoryHandler::SnapshotHandler&
server::file::FileSnapshot::handler()
{
    DirectoryHandler::SnapshotHandler* hdl = m_root.rootDirectory().getSnapshotHandler();
    if (hdl == 0) {
        throw std::runtime_error(SNAPSHOTTING_NOT_AVAILABLE);
    }
    return *hdl;
}

void
server::file::FileSnapshot::verifyName(const String_t& name)
{
    if (name.empty()
        || name[0] == '.'
        || name[name.size()-1] == '.'
        || name.find("..") != String_t::npos
        || hasInvalidCharacter(name))
    {
        throw std::runtime_error(INVALID_SNAPSHOT);
    }
}
