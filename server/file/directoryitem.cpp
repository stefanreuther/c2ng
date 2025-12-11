/**
  *  \file server/file/directoryitem.cpp
  *  \brief Class server::file::DirectoryItem
  */

#include "server/file/directoryitem.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "server/errors.hpp"
#include "server/file/gamestatus.hpp"
#include "server/file/root.hpp"

namespace {
    const char LOG_NAME[] = "file.dir";

    const char CONTROL_FILE[] = ".c2file";
}

// Constructor.
server::file::DirectoryItem::DirectoryItem(String_t name, DirectoryItem* parent, std::auto_ptr<DirectoryHandler> handler)
    : Item(name),
      m_parent(parent),
      m_handler(handler),
      m_controlFile(),
      m_subdirectories(),
      m_files(),
      m_hasUnknownContent(false),
      m_wasRead(false),
      m_owner(),
      m_controlInfo(),
      m_gameStatus()
{ }

// Destructor.
server::file::DirectoryItem::~DirectoryItem()
{ }

// Read content.
void
server::file::DirectoryItem::readContent(Root& root)
{
    // ex UserDirectory::read
    // Do we need to read?
    if (m_wasRead) {
        return;
    }
    m_wasRead = true;

    // Read directory
    root.log().write(afl::sys::LogListener::Debug, LOG_NAME, afl::string::Format("reading %s", m_handler->getName()));
    class Callback : public DirectoryHandler::Callback {
     public:
        Callback(DirectoryItem& parent)
            : m_parent(parent)
            { }
        virtual void addItem(const DirectoryHandler::Info& info)
            {
                if (info.name.empty() || info.name[0] == '.') {
                    // Special
                    if (info.type == DirectoryHandler::IsFile && info.name == CONTROL_FILE) {
                        m_parent.m_controlFile.reset(new FileItem(info));
                    } else {
                        m_parent.m_hasUnknownContent = true;
                    }
                } else {
                    // Regular
                    switch (info.type) {
                     case DirectoryHandler::IsDirectory:
                        m_parent.m_subdirectories.pushBackNew(new DirectoryItem(info.name, &m_parent, std::auto_ptr<DirectoryHandler>(m_parent.m_handler->getDirectory(info))));
                        break;

                     case DirectoryHandler::IsFile:
                        m_parent.m_files.pushBackNew(new FileItem(info));
                        break;

                     case DirectoryHandler::IsUnknown:
                        m_parent.m_hasUnknownContent = true;
                        break;
                    }
                }
            }
     private:
        DirectoryItem& m_parent;
    };

    try {
        // Read content
        Callback c(*this);
        m_handler->readContent(c);

        // Read metainfo
        loadControlFile();
        updateOwner();
    }
    catch (std::exception& e) {
        root.log().write(afl::sys::LogListener::Error, LOG_NAME, m_handler->getName(), e);
    }
}

// Check whether this directory was read.
bool
server::file::DirectoryItem::wasRead() const
{
    // ex UserDirectory::wasRead
    return m_wasRead;
}

// Forget this directory and all its subdirectories.
void
server::file::DirectoryItem::forgetContent(Root& root)
{
    if (m_wasRead) {
        root.log().write(afl::sys::LogListener::Debug, LOG_NAME, afl::string::Format("forgetting %s", m_handler->getName()));

        m_controlFile.reset();
        m_subdirectories.clear();
        m_files.clear();
        m_hasUnknownContent = false;
        m_wasRead = false;
        m_owner.clear();
        m_controlInfo.clear();
        m_gameStatus.reset();
    }
}

// Read game status.
server::file::GameStatus&
server::file::DirectoryItem::readGameStatus(Root& root)
{
    // ex UserDirectory::getDirInfo
    if (m_gameStatus.get() == 0) {
        root.log().write(afl::sys::LogListener::Debug, LOG_NAME, afl::string::Format("checking %s", m_handler->getName()));
        m_gameStatus.reset(new GameStatus());
        m_gameStatus->load(root, *this);
    }
    return *m_gameStatus;
}

// Find directory by name.
server::file::DirectoryItem*
server::file::DirectoryItem::findDirectory(String_t name) const
{
    // ex UserDirectory::find (sort-of)
    for (size_t i = 0, n = m_subdirectories.size(); i < n; ++i) {
        if (DirectoryItem* p = m_subdirectories[i]) {
            if (p->getName() == name) {
                return p;
            }
        }
    }
    return 0;
}

// Find file by name.
server::file::FileItem*
server::file::DirectoryItem::findFile(String_t name) const
{
    // ex UserDirectory::find (sort-of)
    for (size_t i = 0, n = m_files.size(); i < n; ++i) {
        if (FileItem* p = m_files[i]) {
            if (p->getName() == name) {
                return p;
            }
        }
    }
    return 0;
}

// Get number of subdirectories.
size_t
server::file::DirectoryItem::getNumDirectories() const
{
    // ex UserDirectory::getNumDirs
    return m_subdirectories.size();
}

// Get subdirectory by index.
server::file::DirectoryItem*
server::file::DirectoryItem::getDirectoryByIndex(size_t n) const
{
    // ex UserDirectory::getDir
    return (n < m_subdirectories.size()
            ? m_subdirectories[n]
            : 0);
}

// Get number of files.
size_t
server::file::DirectoryItem::getNumFiles() const
{
    // ex UserDirectory::getNumFiles
    return m_files.size();
}

// Get file by index.
server::file::FileItem*
server::file::DirectoryItem::getFileByIndex(size_t n) const
{
    // ex UserDirectory::getFile
    return (n < m_files.size()
            ? m_files[n]
            : 0);
}

// Get content of a file.
afl::base::Ref<afl::io::FileMapping>
server::file::DirectoryItem::getFileContent(FileItem& fileItem)
{
    return m_handler->getFile(fileItem.getInfo());
}

// Create or overwrite a file.
void
server::file::DirectoryItem::createFile(const String_t& fileName, afl::base::ConstBytes_t content)
{
    // ex UserDirectory::createFile (sort-of)
    // @change: ASSERT(wasRead())

    // Check for existing folder of the same name
    if (findDirectory(fileName) != 0) {
        throw std::runtime_error(ALREADY_EXISTS);
    }

    // Writing a file invalidates game info
    m_gameStatus.reset();

    // Create it
    DirectoryHandler::Info info = m_handler->createFile(fileName, content);

    // Update content
    if (FileItem* it = findFile(fileName)) {
        it->setInfo(info);
    } else {
        m_files.pushBackNew(new FileItem(info));
    }
}

// Copy file.
bool
server::file::DirectoryItem::copyFile(DirectoryItem& sourceDirectory, FileItem& sourceFile, const String_t& fileName)
{
    // Check for existing folder of the same name
    if (findDirectory(fileName) != 0) {
        throw std::runtime_error(ALREADY_EXISTS);
    }

    afl::base::Optional<DirectoryHandler::Info> copyStatus = m_handler->copyFile(*sourceDirectory.m_handler, sourceFile.getInfo(), fileName);
    if (const DirectoryHandler::Info* p = copyStatus.get()) {
        // Copy succeeded; update status
        m_gameStatus.reset();
        if (FileItem* it = findFile(fileName)) {
            it->setInfo(*p);
        } else {
            m_files.pushBackNew(new FileItem(*p));
        }
        return true;
    } else {
        // Underlay copy did not work; tell caller to fall back to local copy
        return false;
    }
}

// Create subdirectory.
server::file::DirectoryItem*
server::file::DirectoryItem::createDirectory(const String_t& dirName)
{
    // ex UserDirectory::createDirectory (responsibilities changed a lot)
    // Check for existing item
    if (findDirectory(dirName) != 0 || findFile(dirName) != 0) {
        throw std::runtime_error(ALREADY_EXISTS);
    }

    // Create
    DirectoryHandler::Info info = m_handler->createDirectory(dirName);
    return m_subdirectories.pushBackNew(new DirectoryItem(dirName, this, std::auto_ptr<DirectoryHandler>(m_handler->getDirectory(info))));
}

// Remove an item.
void
server::file::DirectoryItem::removeItem(Root& root, Item* it)
{
    // ex UserDirectory::removeEntry
    // FIXME: This is a very simple implementation.
    // A more clever implementation would remove \c it from \c m_subdirectories or \c m_files
    // instead of forgetting the whole directory.

    // This readContent() should not be necessary because /it/ can only be valid if readContent() has already been called, but it does not hurt.
    readContent(root);

    if (dynamic_cast<FileItem*>(it) != 0) {
        // It's a file.
        m_handler->removeFile(it->getName());
    } else if (DirectoryItem* ud = dynamic_cast<DirectoryItem*>(it)) {
        // It's a directory.
        // Check whether it contains anything.
        ud->readContent(root);
        if (ud->getNumDirectories() != 0 || ud->getNumFiles() != 0) {
            // FIXME: Use a different error message?
            root.log().write(afl::sys::LogListener::Trace, LOG_NAME, afl::string::Format("rejecting removal of %s/%s because it contains user items", m_handler->getName(), it->getName()));
            throw std::runtime_error(PERMISSION_DENIED);
        }

        // Remove content (metadata file). Throws if this is not possible.
        ud->removeSystemContent(root);

        // Remove the subdirectory itself.
        m_handler->removeDirectory(it->getName());
    } else {
        // What is it?
        throw std::runtime_error(PERMISSION_DENIED);
    }

    // When we're here, the item has been successfully removed.
    // For simplicity, forget cached information.
    // This will invalidate /it/.
    forgetContent(root);
}

void
server::file::DirectoryItem::removeUserContent(Root& root)
{
    // ex UserDirectory::removeAllEntries
    readContent(root);
    try {
        for (size_t i = 0, n = m_files.size(); i < n; ++i) {
            m_handler->removeFile(m_files[i]->getName());
        }
        for (size_t i = 0, n = m_subdirectories.size(); i < n; ++i) {
            DirectoryItem* p = m_subdirectories[i];
            p->readContent(root);
            if (p->getNumDirectories() != 0 || p->getNumFiles() != 0) {
                root.log().write(afl::sys::LogListener::Trace, LOG_NAME, afl::string::Format("rejecting removal of %s/%s because it contains user items", m_handler->getName(), p->getName()));
                throw std::runtime_error(PERMISSION_DENIED);
            }
            p->removeSystemContent(root);
            m_handler->removeDirectory(p->getName());
        }
    }
    catch (...) {
        forgetContent(root);
        throw;
    }
    forgetContent(root);
}

// Get directory property.
String_t
server::file::DirectoryItem::getProperty(String_t p) const
{
    // ex UserDirectory::getProperty
    // ASSERT(wasRead());
    ControlInfo_t::const_iterator i = m_controlInfo.find(p);
    if (i != m_controlInfo.end()) {
        return i->second;
    } else {
        return String_t();
    }
}

// Modify/set directory property.
void
server::file::DirectoryItem::setProperty(String_t p, String_t v)
{
    // ex UserDirectory::setProperty
    // ASSERT(wasRead());
    // FIXME: set-to-empty should remove it
    // FIXME: move name/value validation in here
    m_controlInfo[p] = v;
    saveControlFile();
    if (p == "owner") {
        updateOwner();
    }
}

// Get owner of this directory.
String_t
server::file::DirectoryItem::getOwner() const
{
    // ex UserDirectory::getOwner
    // ASSERT(wasRead());
    return m_owner;
}

// Check for user permission.
bool
server::file::DirectoryItem::hasPermission(String_t user, Permission p) const
{
    // ex UserDirectory::getPermissions
    // ASSERT(wasRead());
    ControlInfo_t::const_iterator i;
    if (user.empty() || user == getOwner()) {
        // Owner and root can do anything
        return true;
    } else if ((i = m_controlInfo.find("perms:" + user)) != m_controlInfo.end()) {
        // Explicit permissions for this user
        return getPermissionsFromString(i->second).contains(p);
    } else if ((i = m_controlInfo.find("perms:*")) != m_controlInfo.end()) {
        // Wildcard permissions
        return getPermissionsFromString(i->second).contains(p);
    } else {
        // Refuse anyone else
        return false;
    }
}

// List all permissions.
void
server::file::DirectoryItem::listPermissions(std::vector<server::interface::FileBase::Permission>& result) const
{
    // ex UserDirectory::listPermissions
    // ASSERT(wasRead());
    for (ControlInfo_t::const_iterator i = m_controlInfo.begin(), e = m_controlInfo.end(); i != e; ++i) {
        if (i->first.size() > 6 && i->first.compare(0, 6, "perms:", 6) == 0) {
            result.push_back(server::interface::FileBase::Permission(i->first.substr(6), i->second));
        }
    }
}

// Set user permissions.
void
server::file::DirectoryItem::setPermission(String_t userId, String_t permission)
{
    // ex UserDirectory::setPermissions
    // ASSERT(wasRead());
    setProperty("perms:" + userId, getStringFromPermissions(getPermissionsFromString(permission)));
}

// Get visibility level for a directory.
int
server::file::DirectoryItem::getVisibilityLevel() const
{
    // ex UserDirectory::getVisibilityLevel
    // ASSERT(wasRead());
    // FIXME: this works correctly as long as we have no permissions that are lexically before '*'
    for (ControlInfo_t::const_iterator i = m_controlInfo.begin(), e = m_controlInfo.end(); i != e; ++i) {
        if (i->first.size() > 6 && i->first.compare(0, 6, "perms:", 6) == 0) {
            // It's a permission entry.
            if (!getPermissionsFromString(i->second).empty()) {
                if (i->first == "perms:*") {
                    return 2;
                } else {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Compute disk usage totals.
void
server::file::DirectoryItem::computeTotals(Root& root, int32_t& numFiles, int32_t& totalKBytes)
{
    // ex UserDirectory::computeTotals
    // Count ourselves
    ++numFiles;
    ++totalKBytes;

    // Count content
    readContent(root);
    for (size_t i = 0, n = m_subdirectories.size(); i < n; ++i) {
        m_subdirectories[i]->computeTotals(root, numFiles, totalKBytes);
    }
    for (size_t i = 0, n = m_files.size(); i < n; ++i) {
        ++numFiles;
        if (const int32_t* pSize = m_files[i]->getInfo().size.get()) {
            totalKBytes += (*pSize + 1023) / 1024;
        }
    }
}

// Access SnapshotHandler.
server::file::DirectoryHandler::SnapshotHandler*
server::file::DirectoryItem::getSnapshotHandler()
{
    return m_handler->getSnapshotHandler();
}

/** Load control file. */
void
server::file::DirectoryItem::loadControlFile()
{
    // ex UserDirectory::loadControlFile
    if (/*FileItem* p =*/ m_controlFile.get()) {
        // Read file as stream
        afl::base::Ref<afl::io::FileMapping> map = m_handler->getFile(m_controlFile->getInfo());
        afl::io::ConstMemoryStream stream(map->get());
        afl::io::TextFile tf(stream);
        tf.setCharsetNew(new afl::charset::Utf8Charset());

        // Parse
        String_t line;
        while (tf.readLine(line)) {
            String_t::size_type n = line.find('=');
            if (n != line.npos) {
                m_controlInfo.insert(std::make_pair(String_t(line, 0, n), String_t(line, n+1)));
            }
        }
    }
}

/** Save control file. */
void
server::file::DirectoryItem::saveControlFile()
{
    // ex UserDirectory::saveControlFile
    if (m_controlInfo.empty()) {
        // File should be empty
        if (m_controlFile.get() != 0) {
            m_handler->removeFile(m_controlFile->getName());
            m_controlFile.reset();
        }
    } else {
        // File should have content. Build that.
        afl::io::InternalStream out;
        for (ControlInfo_t::const_iterator i = m_controlInfo.begin(); i != m_controlInfo.end(); ++i) {
            out.fullWrite(afl::string::toBytes(i->first));
            out.fullWrite(afl::string::toBytes("="));
            out.fullWrite(afl::string::toBytes(i->second));
            out.fullWrite(afl::string::toBytes("\n"));
        }

        DirectoryHandler::Info info = m_handler->createFile(CONTROL_FILE, out.getContent());
        m_controlFile.reset(new FileItem(info));
    }
}

/** Update owner after a change. */
void
server::file::DirectoryItem::updateOwner()
{
    // ex UserDirectory::setOwner
    // FIXME: place this logic in getOwner()?
    // FIXME: must invalidate children!
    ControlInfo_t::const_iterator i = m_controlInfo.find("owner");
    if (i != m_controlInfo.end()) {
        m_owner = i->second;
    } else if (m_parent != 0) {
        m_owner = m_parent->getOwner();
    } else {
        m_owner.clear();
    }
}

/** Remove system content.
    This is the last thing to call on a user-perceived-empty directory.
    Its job is to remove things remaining in the directory which the user does not see. */
void
server::file::DirectoryItem::removeSystemContent(Root& root)
{
    // If it has unknown content, we cannot make it empty.
    if (m_hasUnknownContent) {
        root.log().write(afl::sys::LogListener::Trace, LOG_NAME, afl::string::Format("rejecting removal of %s because it has unknown content", m_handler->getName()));
        throw std::runtime_error(PERMISSION_DENIED);
    }

    // If it has a control file, get rid of that.
    if (FileItem* p = m_controlFile.get()) {
        m_handler->removeFile(p->getName());
        m_controlFile.reset();
    }
}

/** Convert string into Permissions_t.
    Invalid permissions will be removed. */
server::file::DirectoryItem::Permissions_t
server::file::DirectoryItem::getPermissionsFromString(const String_t& str)
{
    Permissions_t result;
    for (size_t i = 0, n = str.size(); i < n; ++i) {
        switch (str[i]) {
         case 'r':  result += AllowRead;   break;
         case 'w':  result += AllowWrite;  break;
         case 'l':  result += AllowList;   break;
         case 'a':  result += AllowAccess; break;
         default:                          break;
        }
    }
    return result;
}

/** Convert Permissions_t into string. */
String_t
server::file::DirectoryItem::getStringFromPermissions(Permissions_t p)
{
    String_t result;
    if (p.contains(AllowRead)) {
        result += 'r';
    }
    if (p.contains(AllowWrite)) {
        result += 'w';
    }
    if (p.contains(AllowList)) {
        result += 'l';
    }
    if (p.contains(AllowAccess)) {
        result += 'a';
    }
    if (result.empty()) {
        result += '0';
    }
    return result;
}
