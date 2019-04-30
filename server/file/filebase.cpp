/**
  *  \file server/file/filebase.cpp
  *  \brief Class server::file::FileBase
  */

#include <stdexcept>
#include "server/file/filebase.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/textfile.hpp"
#include "server/errors.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/fileitem.hpp"
#include "server/file/pathresolver.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "server/types.hpp"
#include "util/configurationfile.hpp"

namespace {
    void snoopFileContent(server::file::DirectoryItem& dir,
                          const String_t& fileName,
                          const String_t& content)
    {
        if (fileName == "pconfig.src") {
            // Load
            using util::ConfigurationFile;
            afl::io::ConstMemoryStream ms(afl::string::toBytes(content));
            afl::io::TextFile rdr(ms);
            ConfigurationFile file;
            file.load(rdr);

            // Check
            const ConfigurationFile::Element* ele = file.findElement(ConfigurationFile::Assignment, "phost.gamename");
            if (!ele) {
                ele = file.findElement(ConfigurationFile::Assignment, "gamename");
            }
            if (ele != 0) {
                String_t value = afl::string::strTrim(ele->value);
                if (!value.empty()) {
                    dir.setProperty("prop:name", value);
                }
            }
        }
    }
}

server::file::FileBase::FileBase(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::file::FileBase::copyFile(String_t sourceFile, String_t destFile)
{
    // ex Connection::doCopy
    // Resolve source [see GET]
    PathResolver sourceRes(m_root, m_root.rootDirectory(), m_session.getUser());
    FileItem& sourceItem = sourceRes.resolveToFile(sourceFile, DirectoryItem::AllowRead);

    // Resolve destFile [see PUT]
    PathResolver destRes(m_root, m_root.rootDirectory(), m_session.getUser());
    destRes.resolvePath(destFile);
    destRes.checkPermission(DirectoryItem::AllowWrite);
    destRes.getDirectory().readContent(m_root);

    // Try underlay copy
    if (destRes.getDirectory().copyFile(sourceRes.getDirectory(), sourceItem, destFile)) {
        // ok
    } else {
        // Local copy
        afl::base::Ref<afl::io::FileMapping> map(sourceRes.getDirectory().getFileContent(sourceItem));
        afl::base::ConstBytes_t bytes(map->get());

        if (bytes.size() > m_root.getMaxFileSize()) {
            throw std::runtime_error(FILE_TOO_LARGE);
        }

        // Write the file
        destRes.getDirectory().createFile(destFile, bytes);
    }
}

void
server::file::FileBase::forgetDirectory(String_t dirName)
{
    // ex Connection::doForget
    // We want to avoid pulling in directories that do not exist or are not loaded,
    // and we do not need permission or syntax checks because we're not giving out information to the user.
    // So we implement our own, simple path parser here.
    DirectoryItem* p = &m_root.rootDirectory();
    String_t::size_type n;
    while (p != 0 && (n = dirName.find('/')) != String_t::npos) {
        p = p->wasRead() ? p->findDirectory(String_t(dirName, 0, n)) : 0;
        dirName.erase(0, n+1);
    }
    if (p != 0) {
        p = p->wasRead() ? p->findDirectory(dirName) : 0;
    }
    if (p != 0) {
        p->forgetContent(m_root);
    }
}

void
server::file::FileBase::testFiles(afl::base::Memory<const String_t> fileNames, afl::data::IntegerList_t& resultFlags)
{
    // ex Connection::doFileTest
    while (const String_t* p = fileNames.eat()) {
        int ok;
        try {
            PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
            res.resolveToFile(*p, DirectoryItem::AllowRead);
            ok = 1;
        }
        catch (...) {
            ok = 0;
        }
        resultFlags.push_back(ok);
    }
}

String_t
server::file::FileBase::getFile(String_t fileName)
{
    // ex Connection::doGet
    // @change c2file-classic has these checks in a different order
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    FileItem& file = res.resolveToFile(fileName, DirectoryItem::AllowRead);

    // Load
    afl::base::Ref<afl::io::FileMapping> map(res.getDirectory().getFileContent(file));
    afl::base::ConstBytes_t bytes(map->get());

    if (bytes.size() > m_root.getMaxFileSize()) {
        throw std::runtime_error(FILE_TOO_LARGE);
    }

    return afl::string::fromBytes(bytes);
}

void
server::file::FileBase::getDirectoryContent(String_t dirName, ContentInfoMap_t& result)
{
    // ex Connection::doList
    // @change Permission checks totally reworked
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& d = res.resolveToDirectory(dirName, DirectoryItem::AllowList);
    d.readContent(m_root);
    for (size_t i = 0, n = d.getNumDirectories(); i < n; ++i) {
        if (DirectoryItem* p = d.getDirectoryByIndex(i)) {
            result.insertNew(p->getName(), new Info(describeItem(*p)));
        }
    }
    for (size_t i = 0, n = d.getNumFiles(); i < n; ++i) {
        if (FileItem* p = d.getFileByIndex(i)) {
            result.insertNew(p->getName(), new Info(describeItem(*p)));
        }
    }
}

void
server::file::FileBase::getDirectoryPermission(String_t dirName, String_t& ownerUserId, std::vector<Permission>& result)
{
    // ex Connection::doListPerms
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& dir = res.resolveToDirectory(dirName, DirectoryItem::AllowAccess);
    dir.readContent(m_root);

    // Produce result
    ownerUserId = dir.getOwner();
    dir.listPermissions(result);
}

void
server::file::FileBase::createDirectory(String_t dirName)
{
    createDirectoryCommon(dirName, String_t());
}

void
server::file::FileBase::createDirectoryTree(String_t dirName)
{
    // ex Connection::doMkdirHier
    /* Simple dumb-ass version: try to create path */
    for (size_t i = 0; i <= dirName.size(); ++i) {
        if (i == dirName.size() || dirName[i] == '/') {
            /* Got a partial path name */
            String_t part(dirName, 0, i);
            PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
            res.resolvePath(part);
            Item* it = res.resolveLeaf(part);
            if (it != 0) {
                if (dynamic_cast<DirectoryItem*>(it) == 0) {
                    /* Item exists but is not a directory */
                    throw std::runtime_error(ALREADY_EXISTS);
                }
            } else {
                // We'll be creating something, so we need write permission
                res.checkPermission(DirectoryItem::AllowWrite);
                res.getDirectory().createDirectory(part);
            }
        }
    }
}

void
server::file::FileBase::createDirectoryAsUser(String_t dirName, String_t userId)
{
    if (!m_session.isAdmin()) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
    if (userId.empty()) {
        throw std::runtime_error(BAD_REQUEST);
    }
    createDirectoryCommon(dirName, userId);
}

afl::data::Value*
server::file::FileBase::getDirectoryProperty(String_t dirName, String_t propName)
{
    // FIXME: change signature to return String?
    // ex Connection::doPropGet
    // @change Returns 405 when applied to directories
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& dir = res.resolveToDirectory(dirName, DirectoryItem::AllowRead);
    dir.readContent(m_root);

    return makeStringValue(dir.getProperty("prop:" + propName));
}

void
server::file::FileBase::setDirectoryProperty(String_t dirName, String_t propName, String_t propValue)
{
    // ex Connection::doPropSet
    // @change Returns 405 when applied to directories
    if (propValue.find_first_of("\r\n") != String_t::npos || propName.find_first_of("\r\n=") != String_t::npos) {
        throw std::runtime_error(BAD_REQUEST);
    }

    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& dir = res.resolveToDirectory(dirName, DirectoryItem::AllowWrite);

    dir.readContent(m_root);
    dir.setProperty("prop:" + propName, propValue);
}

void
server::file::FileBase::putFile(String_t fileName, String_t content)
{
    // ex Connection::doPut
    // @change c2file-classic has no size check and relies on the input parser
    if (content.size() > m_root.getMaxFileSize()) {
        throw std::runtime_error(FILE_TOO_LARGE);
    }
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    res.resolvePath(fileName);
    res.checkPermission(DirectoryItem::AllowWrite);

    DirectoryItem& dir = res.getDirectory();
    dir.readContent(m_root);
    dir.createFile(fileName, afl::string::toBytes(content));
    snoopFileContent(dir, fileName, content);
}

void
server::file::FileBase::removeFile(String_t fileName)
{
    // ex Connection::doRemove
    // @change c2file-ng has different access checking
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    Item& it = res.resolveToItem(fileName, DirectoryItem::AllowWrite);
    res.getDirectory().removeItem(m_root, &it);
}

void
server::file::FileBase::removeDirectory(String_t dirName)
{
    // ex Connection::doRmdir
    // @change c2file-ng has different access checking
    // Resolve path.
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    res.resolvePath(dirName);

    // Resolve final component
    Item* it = res.resolveLeaf(dirName);
    DirectoryItem* dir = dynamic_cast<DirectoryItem*>(it);
    if (!dir) {
        if (res.hasPermission(DirectoryItem::AllowList)) {
            if (it != 0) {
                throw std::runtime_error(NOT_A_DIRECTORY);
            } else {
                throw std::runtime_error(FILE_NOT_FOUND);
            }
        } else {
            throw std::runtime_error(PERMISSION_DENIED);
        }
    }

    // Must have write access to parent.
    res.checkPermission(DirectoryItem::AllowWrite);

    // Build list of directories.
    // While doing so, check permissions.
    // Must have write permission for everyone.
    std::vector<DirectoryItem*> dirs;
    dirs.push_back(dir);
    size_t index = 0;
    while (index < dirs.size()) {
        // Fetch directory
        DirectoryItem* it = dirs[index++];
        it->readContent(m_root);
        if (!it->hasPermission(m_session.getUser(), DirectoryItem::AllowWrite)) {
            throw std::runtime_error(PERMISSION_DENIED);
        }

        // Enumerate subdirectories
        for (size_t i = 0, e = it->getNumDirectories(); i < e; ++i) {
            dirs.push_back(it->getDirectoryByIndex(i));
        }
    }

    // We now have all directories in /dirs/, parents preceding their children.
    // We can therefore delete them recursively by just going backwards.
    for (size_t i = dirs.size(); i > 0; --i) {
        dirs[i-1]->removeUserContent(m_root);
    }

    // Finally, delete the directory itself
    res.getDirectory().removeItem(m_root, dir);
}

void
server::file::FileBase::setDirectoryPermissions(String_t dirName, String_t userId, String_t permission)
{
    // ex Connection::doSetPerms
    // @change This check is new in c2file-ng.
    if (userId.empty()) {
        throw std::runtime_error(BAD_REQUEST);
    }

    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& dir = res.resolveToDirectory(dirName, DirectoryItem::AllowAccess);
    dir.readContent(m_root);
    dir.setPermission(userId, permission);
}

server::file::FileBase::Info
server::file::FileBase::getFileInformation(String_t fileName)
{
    // ex Connection::doStat
    // @change c2file-classic does not have this permission check
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    Item& it = res.resolveToItem(fileName, DirectoryItem::AllowList);
    return describeItem(it);
}

server::file::FileBase::Usage
server::file::FileBase::getDiskUsage(String_t dirName)
{
    // ex Connection::doUsage
    // @change This has pretty much no permission checking in c2file-classic.
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    DirectoryItem& dir = res.resolveToDirectory(dirName, DirectoryItem::AllowList);
    dir.readContent(m_root);

    // Compute statistics
    Usage result;
    dir.computeTotals(m_root, result.numItems, result.totalKBytes);
    return result;
}

void
server::file::FileBase::createDirectoryCommon(String_t dirName, String_t userId)
{
    // ex Connection::doMkdir
    /* Resolve path */
    PathResolver res(m_root, m_root.rootDirectory(), m_session.getUser());
    res.resolvePath(dirName);
    res.checkPermission(DirectoryItem::AllowWrite);
    if (res.resolveLeaf(dirName)) {
        throw std::runtime_error(ALREADY_EXISTS);
    }

    DirectoryItem* item = res.getDirectory().createDirectory(dirName);
    if (item != 0 && !userId.empty()) {
        item->readContent(m_root);
        item->setProperty("owner", userId);
    }
}

server::file::FileBase::Info
server::file::FileBase::describeItem(Item& it)
{
    // ex Connection::doStat (sort-of)
    Info result;
    if (DirectoryItem* d = dynamic_cast<DirectoryItem*>(&it)) {
        d->readContent(m_root);
        result.type = IsDirectory;
        result.visibility = d->getVisibilityLevel();
    } else if (FileItem* f = dynamic_cast<FileItem*>(&it)) {
        result.type = IsFile;
        result.size = f->getInfo().size;
        result.contentId = f->getInfo().contentId;
    } else {
        result.type = IsUnknown;
    }
    return result;
}
