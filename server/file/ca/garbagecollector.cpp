/**
  *  \file server/file/ca/garbagecollector.cpp
  *  \brief Class server::file::ca::GarbageCollector
  */

#include <vector>
#include "server/file/ca/garbagecollector.hpp"
#include "afl/charset/hexencoding.hpp"
#include "afl/string/format.hpp"
#include "server/file/ca/directoryentry.hpp"
#include "server/file/ca/objectstore.hpp"

using afl::string::Format;
using afl::sys::LogListener;

namespace {
    const char*const LOG_NAME = "file.ca";
}

server::file::ca::GarbageCollector::GarbageCollector(ObjectStore& objStore, afl::sys::LogListener& log)
    : m_objectStore(objStore),
      m_log(log),
      m_objectsToKeep(),
      m_treesToCheck(),
      m_nextPrefixToCheck(0),
      m_numObjectsRemoved(0),
      m_numErrors(0)
{ }

server::file::ca::GarbageCollector::~GarbageCollector()
{ }

void
server::file::ca::GarbageCollector::addCommit(const ObjectId& id)
{
    if (id != ObjectId::nil) {
        try {
            if (m_objectsToKeep.insert(id).second) {
                addTree(m_objectStore.getCommit(id));
            }
        }
        catch (std::exception& e) {
            m_log.write(LogListener::Error, LOG_NAME, Format("%s: error resolving as commit, ignoring", id.toHex()), e);
            ++m_numErrors;
        }
    }
}

void
server::file::ca::GarbageCollector::addTree(const ObjectId& id)
{
    // Register object for checking if we haven't already registered it for keeping
    if (m_objectsToKeep.find(id) == m_objectsToKeep.end()) {
        m_treesToCheck.insert(id);
    }
}

void
server::file::ca::GarbageCollector::addFile(const ObjectId& id)
{
    m_objectsToKeep.insert(id);
}

bool
server::file::ca::GarbageCollector::checkObject()
{
    if (!m_treesToCheck.empty()) {
        // TODO: when we have C++17, using merge(extract()) maybe saves some memory/cycles here
        const ObjectId id = *m_treesToCheck.begin();
        m_treesToCheck.erase(m_treesToCheck.begin());
        m_objectsToKeep.insert(id);

        try {
            afl::base::Ref<afl::io::FileMapping> content = m_objectStore.getObject(id, ObjectStore::TreeObject);
            afl::base::ConstBytes_t contentBytes = content->get();
            DirectoryEntry e;
            while (e.parse(contentBytes)) {
                switch (e.getType()) {
                 case DirectoryHandler::IsUnknown:
                    m_log.write(LogListener::Error, LOG_NAME, Format("%s: unrecognized child element '%s'", id.toHex(), e.getName()));
                    addFile(e.getId());
                    ++m_numErrors;
                    break;

                 case DirectoryHandler::IsFile:
                    addFile(e.getId());
                    break;

                 case DirectoryHandler::IsDirectory:
                    addTree(e.getId());
                    break;
                }
            }
        }
        catch (std::exception& e) {
            m_log.write(LogListener::Error, LOG_NAME, Format("%s: error resolving as tree, ignoring", id.toHex()), e);
            ++m_numErrors;
        }

        m_nextPrefixToCheck = 0;
        return true;
    } else {
        return false;
    }
}

bool
server::file::ca::GarbageCollector::removeGarbageObjects()
{
    if (!m_treesToCheck.empty()) {
        // Fail-safe! Must not remove anything in this case.
        // User should not have called this; try to give him a hint to not call us again.
        return false;
    } else if (m_nextPrefixToCheck < 256) {
        // Check one prefix
        class Collector : public DirectoryHandler::Callback {
         public:
            Collector(const GarbageCollector& parent, uint8_t firstByte)
                : m_parent(parent), m_firstByte(firstByte), m_filesToDelete()
                { }
            virtual void addItem(const DirectoryHandler::Info& info)
                {
                    String_t asHex = afl::charset::HexEncoding().decode(afl::string::toBytes(info.name));
                    bool ok = false;
                    if (info.type == DirectoryHandler::IsFile && asHex.size() == sizeof(ObjectId)-1) {
                        ObjectId id;
                        id.m_bytes[0] = m_firstByte;
                        afl::base::Bytes_t(id.m_bytes).subrange(1).copyFrom(afl::string::toBytes(asHex));
                        if (id.toHex().substr(2) == info.name) {
                            ok = true;
                            if (m_parent.m_objectsToKeep.find(id) == m_parent.m_objectsToKeep.end()) {
                                // Remember file for deletion.
                                // Do not immediately delete it now, to not confuse the DirectoryHandler (modification during directory reading).
                                // For now, assume that we can store all files to delete easily:
                                // Assuming around 300000 files, 20% garbage, we get <250 garbage files per directory.
                                // (PlanetsCentral.com accumulated <2.5% garbage after running without GC for 4 years.)
                                m_filesToDelete.push_back(info.name);
                            }
                        }
                    }
                    if (!ok) {
                        m_parent.m_log.write(LogListener::Warn, LOG_NAME, Format("%02x/%s: unrecognized file, ignoring", m_firstByte, info.name));
                    }
                }
            void removeGarbageFiles(DirectoryHandler& hdl)
                {
                    for (size_t i = 0, n = m_filesToDelete.size(); i < n; ++i) {
                        hdl.removeFile(m_filesToDelete[i]);
                    }
                }
            size_t getNumObjectsRemoved() const
                {
                    return m_filesToDelete.size();
                }
         private:
            const GarbageCollector& m_parent;
            const uint8_t m_firstByte;
            std::vector<String_t> m_filesToDelete;
        };

        if (DirectoryHandler* hdl = m_objectStore.getObjectDirectory(m_nextPrefixToCheck)) {
            try {
                Collector c(*this, static_cast<uint8_t>(m_nextPrefixToCheck));
                hdl->readContent(c);
                c.removeGarbageFiles(*hdl);
                m_numObjectsRemoved += c.getNumObjectsRemoved();
            }
            catch (std::exception& e) {
                m_log.write(LogListener::Warn, LOG_NAME, Format("%02x: error cleaning up", m_nextPrefixToCheck), e);
            }
        }

        ++m_nextPrefixToCheck;
        return true;
    } else {
        // Completed
        return false;
    }
}

size_t
server::file::ca::GarbageCollector::getNumObjectsToKeep() const
{
    return m_objectsToKeep.size();
}

size_t
server::file::ca::GarbageCollector::getNumObjectsToCheck() const
{
    return m_treesToCheck.size();
}

size_t
server::file::ca::GarbageCollector::getNumObjectsRemoved() const
{
    return m_numObjectsRemoved;
}

size_t
server::file::ca::GarbageCollector::getNumErrors() const
{
    return m_numErrors;
}
