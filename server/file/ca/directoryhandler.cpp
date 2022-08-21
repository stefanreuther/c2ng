/**
  *  \file server/file/ca/directoryhandler.cpp
  *  \brief Class server::file::ca::DirectoryHandler
  */

#include <stdexcept>
#include "server/file/ca/directoryhandler.hpp"
#include "server/file/ca/objectstore.hpp"
#include "server/errors.hpp"
#include "server/file/ca/directoryentry.hpp"
#include "afl/string/format.hpp"
#include "afl/except/fileproblemexception.hpp"

namespace {
    const char TYPE_MISMATCH[] = "405 Type mismatch";
    const char DIR_NOT_EMPTY[] = "405 Directory not empty";
}


class server::file::ca::DirectoryHandler::ContentUpdater : public server::file::ca::ReferenceUpdater {
 public:
    ContentUpdater(ObjectStore& store, const ObjectId& id, const String_t& name, afl::base::Ref<ReferenceUpdater> updater);

    ObjectStore& store();
    const ObjectId& getId() const;
    const String_t& getName() const;
    String_t getChildName(const String_t& child) const;

    afl::base::Ref<afl::io::FileMapping> getTreeObject();

    virtual void updateDirectoryReference(const String_t& name, const ObjectId& newId);

    void updateDirectoryEntry(const String_t& name, const ObjectId& newId, Type type, bool allowReplace);
    void removeDirectoryEntry(const String_t& name, Type type);

 private:
    void replaceDirectory(afl::base::ConstBytes_t newBytes);

    ObjectStore& m_store;
    ObjectId m_id;
    const String_t m_name;
    const afl::base::Ref<ReferenceUpdater> m_updater;
};

/***************************** ContentUpdater ****************************/

server::file::ca::DirectoryHandler::ContentUpdater::ContentUpdater(ObjectStore& store, const ObjectId& id, const String_t& name, afl::base::Ref<ReferenceUpdater> updater)
    : m_store(store),
      m_id(id),
      m_name(name),
      m_updater(updater)
{ }

server::file::ca::ObjectStore&
server::file::ca::DirectoryHandler::ContentUpdater::store()
{
    return m_store;
}

const server::file::ca::ObjectId&
server::file::ca::DirectoryHandler::ContentUpdater::getId() const
{
    return m_id;
}

const String_t&
server::file::ca::DirectoryHandler::ContentUpdater::getName() const
{
    return m_name;
}

String_t
server::file::ca::DirectoryHandler::ContentUpdater::getChildName(const String_t& child) const
{
    return afl::string::Format("%s in %s '%s'", child, m_id.toHex(), m_name);
}

afl::base::Ref<afl::io::FileMapping>
server::file::ca::DirectoryHandler::ContentUpdater::getTreeObject()
{
    return m_store.getObject(m_id, ObjectStore::TreeObject);
}

void
server::file::ca::DirectoryHandler::ContentUpdater::updateDirectoryReference(const String_t& name, const ObjectId& newId)
{
    if (newId != m_id) {
        updateDirectoryEntry(name, newId, IsDirectory, true);
    }
}

void
server::file::ca::DirectoryHandler::ContentUpdater::updateDirectoryEntry(const String_t& name, const ObjectId& newId, Type type, bool allowReplace)
{
    // Prepare blobs
    afl::base::Ref<afl::io::FileMapping> oldContent = getTreeObject();
    afl::base::ConstBytes_t oldBytes(oldContent->get());
    afl::base::GrowableMemory<uint8_t> newBytes;

    // New entry
    const DirectoryEntry newEntry(name, newId, type);

    // Copy directory entries
    DirectoryEntry e;
    bool did = false;
    while (e.parse(oldBytes)) {
        if (did) {
            // We already did that; just copy.
            e.store(newBytes);
            m_store.linkObject(e.getId());
        } else if (e.getName() == name) {
            // Replacing an entry
            // - check for type conflict
            if (e.getType() != type || !allowReplace) {
                // FIXME: must undo reference count operations!
                throw afl::except::FileProblemException(getChildName(name), ALREADY_EXISTS);
            }

            // - store new object
            newEntry.store(newBytes);
            did = true;
        } else if (newEntry.isBefore(e)) {
            // Inserting an entry
            newEntry.store(newBytes);
            did = true;
            e.store(newBytes);
            m_store.linkObject(e.getId());
        } else {
            // Not inserting
            e.store(newBytes);
            m_store.linkObject(e.getId());
        }
    }

    if (!did) {
        newEntry.store(newBytes);
    }

    // Create new object
    replaceDirectory(newBytes);
}

void
server::file::ca::DirectoryHandler::ContentUpdater::removeDirectoryEntry(const String_t& name, Type type)
{
    // Prepare blobs
    afl::base::Ref<afl::io::FileMapping> oldContent = getTreeObject();
    afl::base::ConstBytes_t oldBytes(oldContent->get());
    afl::base::GrowableMemory<uint8_t> newBytes;

    // Copy directory entries
    DirectoryEntry e;
    bool did = false;
    while (e.parse(oldBytes)) {
        if (e.getName() == name) {
            // Remove
            if (e.getType() != type) {
                // FIXME: undo reference counts on throw
                throw afl::except::FileProblemException(getChildName(name), TYPE_MISMATCH);
            }
            if (type == IsDirectory && m_store.getObjectSize(e.getId(), ObjectStore::TreeObject) != 0) {
                // FIXME: undo reference counts on throw
                throw afl::except::FileProblemException(getChildName(name), DIR_NOT_EMPTY);
            }
            did = true;
        } else {
            // Copy
            e.store(newBytes);
            m_store.linkObject(e.getId());
        }
    }

    if (!did) {
        // FIXME: undo reference counts on throw
        throw afl::except::FileProblemException(getChildName(name), FILE_NOT_FOUND);
    }

    // Create new object
    replaceDirectory(newBytes);
}

void
server::file::ca::DirectoryHandler::ContentUpdater::replaceDirectory(afl::base::ConstBytes_t newBytes)
{
    // There is no need to unlink the previous tree object.
    // That one is still referenced by the parent, up to the root commit.
    // The root ReferenceUpdater can decide whether to keep or unlink it.
    const ObjectId newDirId = m_store.addObject(ObjectStore::TreeObject, newBytes);
    m_updater->updateDirectoryReference(m_name, newDirId);
    m_id = newDirId;
}

/**************************** DirectoryHandler ***************************/

server::file::ca::DirectoryHandler::DirectoryHandler(ObjectStore& store, const ObjectId& id, const String_t& name, afl::base::Ref<ReferenceUpdater> updater)
    : m_content(*new ContentUpdater(store, id, name, updater))
{ }

server::file::ca::DirectoryHandler::~DirectoryHandler()
{ }

String_t
server::file::ca::DirectoryHandler::getName()
{
    return afl::string::Format("%s '%s'", m_content->getId().toHex(), m_content->getName());
}

afl::base::Ref<afl::io::FileMapping>
server::file::ca::DirectoryHandler::getFile(const Info& info)
{
    // If we have a contentId, use that.
    if (const String_t* p = info.contentId.get()) {
        // The Id must be syntactically valid i.e. equal to the stringified form.
        ObjectId id = ObjectId::fromHex(*p);
        if (id.toHex() == *p) {
            return m_content->store().getObject(id, ObjectStore::DataObject);
        }
    }

    // No contentId, look up by name.
    return getFileByName(info.name);
}

afl::base::Ref<afl::io::FileMapping>
server::file::ca::DirectoryHandler::getFileByName(String_t name)
{
    afl::base::Ref<afl::io::FileMapping> self = m_content->getTreeObject();
    afl::base::ConstBytes_t bytes(self->get());
    DirectoryEntry entry;
    while (entry.parse(bytes)) {
        if (entry.getName() == name && entry.getType() == IsFile) {
            return m_content->store().getObject(entry.getId(), ObjectStore::DataObject);
        }
    }
    throw afl::except::FileProblemException(m_content->getChildName(name), FILE_NOT_FOUND);
}

server::file::DirectoryHandler::Info
server::file::ca::DirectoryHandler::createFile(String_t name, afl::base::ConstBytes_t content)
{
    ObjectId id = m_content->store().addObject(ObjectStore::DataObject, content);
    m_content->updateDirectoryEntry(name, id, IsFile, true);

    Info result(name, IsFile);
    result.contentId = id.toHex();
    result.size = convertSize(content.size());
    return result;
}

void
server::file::ca::DirectoryHandler::removeFile(String_t name)
{
    m_content->removeDirectoryEntry(name, IsFile);
}

afl::base::Optional<server::file::DirectoryHandler::Info>
server::file::ca::DirectoryHandler::copyFile(ReadOnlyDirectoryHandler& source, const Info& sourceInfo, String_t name)
{
    // The other side must work on the same ObjectStore
    DirectoryHandler* dh = dynamic_cast<DirectoryHandler*>(&source);
    if (dh == 0 || &m_content->store() != &dh->m_content->store()) {
        return afl::base::Nothing;
    }

    // Verify that we have a valid contentId
    const String_t* p = sourceInfo.contentId.get();
    if (p == 0) {
        return afl::base::Nothing;
    }

    // The Id must be syntactically valid i.e. equal to the stringified form.
    ObjectId id = ObjectId::fromHex(*p);
    if (id.toHex() != *p) {
        return afl::base::Nothing;
    }

    // Must be a file
    if (sourceInfo.type != IsFile) {
        return afl::base::Nothing;
    }

    // All preconditions fulfilled, do it
    m_content->store().linkObject(id);
    m_content->updateDirectoryEntry(name, id, IsFile, true);

    Info result(name, IsFile);
    result.contentId = sourceInfo.contentId;
    result.size = sourceInfo.size;
    return result;
}

void
server::file::ca::DirectoryHandler::readContent(Callback& callback)
{
    afl::base::Ref<afl::io::FileMapping> self = m_content->getTreeObject();
    afl::base::ConstBytes_t bytes(self->get());
    DirectoryEntry entry;
    while (entry.parse(bytes)) {
        Info info(entry.getName(), entry.getType());
        if (entry.getType() == IsFile) {
            info.size = convertSize(m_content->store().getObjectSize(entry.getId(), ObjectStore::DataObject));
        }
        if (entry.getType() != IsDirectory) {
            info.contentId = entry.getId().toHex();
        }
        callback.addItem(info);
    }
}

server::file::ca::DirectoryHandler*
server::file::ca::DirectoryHandler::getDirectory(const Info& info)
{
    afl::base::Ref<afl::io::FileMapping> self = m_content->getTreeObject();
    afl::base::ConstBytes_t bytes(self->get());
    DirectoryEntry entry;
    while (entry.parse(bytes)) {
        if (entry.getName() == info.name && entry.getType() == IsDirectory) {
            return new DirectoryHandler(m_content->store(), entry.getId(), entry.getName(), m_content);
        }
    }
    throw afl::except::FileProblemException(m_content->getChildName(info.name), FILE_NOT_FOUND);
}

server::file::DirectoryHandler::Info
server::file::ca::DirectoryHandler::createDirectory(String_t name)
{
    ObjectId id = m_content->store().addObject(ObjectStore::TreeObject, afl::base::Nothing);
    m_content->updateDirectoryEntry(name, id, IsDirectory, false);

    return Info(name, IsDirectory);
}

void
server::file::ca::DirectoryHandler::removeDirectory(String_t name)
{
    m_content->removeDirectoryEntry(name, IsDirectory);
}
