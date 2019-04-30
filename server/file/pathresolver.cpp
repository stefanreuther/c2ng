/**
  *  \file server/file/pathresolver.cpp
  *  \brief Class server::file::PathResolver
  */

#include <stdexcept>
#include "server/file/pathresolver.hpp"
#include "server/errors.hpp"

namespace {
    bool isValidFileName(const String_t& n)
    {
        // @change Refuse '\0', ':', '/', '\', which make trouble in the OS interface later.
        // The front-end servers will also verify.
        return !n.empty() && n[0] != '.' && n.find_first_of(String_t("\0:/\\", 4)) == String_t::npos;
    }
}

// Constructor.
server::file::PathResolver::PathResolver(Root& root, DirectoryItem& item, String_t user)
    : m_root(root),
      m_base(&item),
      m_user(user)
{
    // ex PathResolver::PathResolver
}

// Resolve a directory path.
void
server::file::PathResolver::resolvePath(String_t& path)
{
    // ex PathResolver::resolvePath
    String_t::size_type i;
    while ((i = path.find('/')) != String_t::npos) {
        // Verify path component.
        // If path starts with a slash or contains a double-slash, this will see an empty component
        // which isValidFileName rejects.
        if (!isValidFileName(String_t(path, 0, i))) {
            throw std::runtime_error(BAD_REQUEST);
        }

        // Check item
        m_base->readContent(m_root);
        DirectoryItem* dir = m_base->findDirectory(String_t(path, 0, i));
        if (!dir) {
            // It does not exist.
            // If the user is allowed to list this directory, say him;
            // otherwise, just say permission denied,
            // to avoid them guessing directory names
            if (m_base->hasPermission(m_user, DirectoryItem::AllowList)) {
                throw std::runtime_error(FILE_NOT_FOUND);
            } else {
                throw std::runtime_error(PERMISSION_DENIED);
            }
        }

        // Sucess: continue in found directory
        m_base = dir;
        path.erase(0, i+1);
    }

    // Check final component for syntactic validity
    if (!isValidFileName(path)) {
        throw std::runtime_error(BAD_REQUEST);
    }
}

// Resolve the final component.
server::file::Item*
server::file::PathResolver::resolveLeaf(String_t path) const
{
    // ex PathResolver::resolveLeaf
    m_base->readContent(m_root);
    Item* it = m_base->findFile(path);
    if (!it) {
        it = m_base->findDirectory(path);
    }
    return it;
}

// Check for permission.
bool
server::file::PathResolver::hasPermission(DirectoryItem::Permission priv) const
{
    // ex PathResolver::checkPrivilege
    m_base->readContent(m_root);
    return m_base->hasPermission(m_user, priv);
}

// Check for permission, throw.
void
server::file::PathResolver::checkPermission(DirectoryItem::Permission priv) const
{
    if (!hasPermission(priv)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
}

// Get current directory.
server::file::DirectoryItem&
server::file::PathResolver::getDirectory() const
{
    // ex PathResolver::getDirectory
    return *m_base;
}

// Resolve path to a directory and check permissions.
server::file::DirectoryItem&
server::file::PathResolver::resolveToDirectory(String_t path, DirectoryItem::Permission priv)
{
    // ex PathResolver::resolveToDirectory
    resolvePath(path);
    Item* it = resolveLeaf(path);
    DirectoryItem* dir = dynamic_cast<DirectoryItem*>(it);
    if (dir == 0) {
        if (hasPermission(DirectoryItem::AllowList)) {
            if (it != 0) {
                throw std::runtime_error(NOT_A_DIRECTORY);
            } else {
                throw std::runtime_error(FILE_NOT_FOUND);
            }
        } else {
            throw std::runtime_error(PERMISSION_DENIED);
        }
    }
    dir->readContent(m_root);
    if (!dir->hasPermission(m_user, priv)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
    return *dir;
}

// Resolve path to a file.
server::file::FileItem&
server::file::PathResolver::resolveToFile(String_t fileName, DirectoryItem::Permission priv)
{
    resolvePath(fileName);
    FileItem* file = dynamic_cast<FileItem*>(resolveLeaf(fileName));
    if (file == 0) {
        if (hasPermission(DirectoryItem::AllowList)) {
            throw std::runtime_error(FILE_NOT_FOUND);
        } else {
            throw std::runtime_error(PERMISSION_DENIED);
        }
    }
    if (!hasPermission(priv)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
    return *file;
}

// Resolve path to an item.
server::file::Item&
server::file::PathResolver::resolveToItem(String_t itemName, DirectoryItem::Permission priv)
{
    resolvePath(itemName);
    Item* it = resolveLeaf(itemName);
    if (it == 0) {
        if (hasPermission(DirectoryItem::AllowList)) {
            throw std::runtime_error(FILE_NOT_FOUND);
        } else {
            throw std::runtime_error(PERMISSION_DENIED);
        }
    }
    if (priv == DirectoryItem::AllowList) {
        if (DirectoryItem* dir = dynamic_cast<DirectoryItem*>(it)) {
            // STAT on a directory: target directory (not container!) must be listable
            dir->readContent(m_root);
            if (!dir->hasPermission(m_user, priv)) {
                throw std::runtime_error(PERMISSION_DENIED);
            }
        } else {
            // STAT on a file: directory must be listable
            checkPermission(priv);
        }
    } else {
        checkPermission(priv);
    }
    return *it;
}
