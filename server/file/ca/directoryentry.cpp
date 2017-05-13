/**
  *  \file server/file/ca/directoryentry.cpp
  *  \brief Class server::file::ca::DirectoryEntry
  */

#include "server/file/ca/directoryentry.hpp"
#include "afl/string/format.hpp"

namespace {
    const uint32_t MASK_FORMAT = 0170000;
    const uint32_t TYPE_FILE   = 0100000;
    const uint32_t TYPE_DIR    = 0040000;

    const uint32_t MODE_FILE   = 0644;
    const uint32_t MODE_DIR    = 0000;
}

// Default constructor.
server::file::ca::DirectoryEntry::DirectoryEntry()
    : m_mode(0),
      m_name(),
      m_id()
{ }

// Construct from data.
server::file::ca::DirectoryEntry::DirectoryEntry(const String_t& name, const ObjectId& id, DirectoryHandler::Type type)
    : m_mode(type == DirectoryHandler::IsDirectory ? MODE_DIR+TYPE_DIR : MODE_FILE+TYPE_FILE),
      m_name(name),
      m_id(id)
{ }

// Parse a tree object.
bool
server::file::ca::DirectoryEntry::parse(afl::base::ConstBytes_t& in)
{
    // Parse mode; octal
    m_mode = 0;
    while (1) {
        const uint8_t* p = in.eat();
        if (p == 0) {
            return false;
        }
        if (*p == ' ') {
            break;
        }
        if (*p < '0' || *p >= '8') {
            return false;
        }
        m_mode = (m_mode << 3) | (*p - '0');
    }

    // Parse name
    m_name.clear();
    while (1) {
        const uint8_t* p = in.eat();
        if (p == 0) {
            return false;
        }
        if (*p == 0) {
            break;
        }
        m_name.append(1, char(*p));
    }

    // Parse Id
    if (in.size() < sizeof(m_id.m_bytes)) {
        return false;
    }
    afl::base::Bytes_t(m_id.m_bytes).copyFrom(in.split(sizeof m_id.m_bytes));
    return true;
}

// Store into tree object.
void
server::file::ca::DirectoryEntry::store(afl::base::GrowableMemory<uint8_t>& out) const
{
    out.append(afl::string::toBytes(afl::string::Format("%o %s", m_mode, m_name)));
    out.append(0);
    out.append(m_id.m_bytes);
}

// Get type of pointed-to object.
server::file::DirectoryHandler::Type
server::file::ca::DirectoryEntry::getType() const
{
    switch (m_mode & MASK_FORMAT) {
     case TYPE_FILE: return DirectoryHandler::IsFile;
     case TYPE_DIR:  return DirectoryHandler::IsDirectory;
     default:        return DirectoryHandler::IsUnknown;
    }
}

// Get name.
const String_t&
server::file::ca::DirectoryEntry::getName() const
{
    return m_name;
}

// Get object Id.
const server::file::ca::ObjectId&
server::file::ca::DirectoryEntry::getId() const
{
    return m_id;
}

// Check whether this entry is sorted before another.
bool
server::file::ca::DirectoryEntry::isBefore(const DirectoryEntry& other) const
{
    String_t myName = getName();
    String_t theirName = other.getName();
    if (getType() == DirectoryHandler::IsDirectory) {
        myName += '/';
    }
    if (other.getType() == DirectoryHandler::IsDirectory) {
        theirName += '/';
    }
    return myName < theirName;
}
