/**
  *  \file server/file/ca/internalobjectcache.cpp
  *  \brief Class server::file::ca::InternalObjectCache
  */

#include "server/file/ca/internalobjectcache.hpp"
#include "afl/except/fileproblemexception.hpp"

namespace {
    const char*const HASH_COLLISION = "500 Hash collision";
}

server::file::ca::InternalObjectCache::InternalObjectCache()
    : m_data(),
      m_newest(),
      m_numObjects(0),
      m_numBytes(0),
      m_maxObjects(10000),
      m_maxBytes(30*1000*1000)
{ }

server::file::ca::InternalObjectCache::~InternalObjectCache()
{ }

void
server::file::ca::InternalObjectCache::setLimits(size_t maxObjects, size_t maxBytes)
{
    m_maxObjects = maxObjects;
    m_maxBytes = maxBytes;
    trimCache();
}

void
server::file::ca::InternalObjectCache::addObject(const ObjectId& id, ObjectStore::Type type, afl::base::Ref<afl::io::FileMapping> content)
{
    // Remove old instance
    removeObject(id);

    // Add new instance
    Node& n = *m_data.insertNew(id, new Node(id, type, content));
    m_numBytes += n.m_size;
    m_numObjects++;
    n.link(m_newest);

    // Overflow handling
    trimCache();
}

void
server::file::ca::InternalObjectCache::addObjectSize(const ObjectId& id, ObjectStore::Type type, size_t size)
{
    Map_t::iterator it = m_data.find(id);
    if (it == m_data.end()) {
        // Add new instance
        Node& n = *m_data.insertNew(id, new Node(id, type, size));
        m_numObjects++;
        n.link(m_newest);
        trimCache();
    } else {
        Node& n = *it->second;
        n.unlink();
        n.link(m_newest);
    }
}

void
server::file::ca::InternalObjectCache::removeObject(const ObjectId& id)
{
    Map_t::iterator it = m_data.find(id);
    if (it != m_data.end() && it->second != 0) {
        Node& n = *it->second;
        m_numBytes -= n.releaseMemory();
        m_numObjects--;
        m_data.erase(it);
    }
}

afl::base::Ptr<afl::io::FileMapping>
server::file::ca::InternalObjectCache::getObject(const ObjectId& id, ObjectStore::Type type)
{
    Map_t::iterator it = m_data.find(id);
    if (it != m_data.end() && it->second != 0) {
        Node& n = *it->second;
        n.checkType(type);
        n.unlink();
        n.link(m_newest);
        return n.m_content;
    } else {
        return 0;
    }
}

afl::base::Optional<size_t>
server::file::ca::InternalObjectCache::getObjectSize(const ObjectId& id, ObjectStore::Type type)
{
    Map_t::iterator it = m_data.find(id);
    if (it != m_data.end() && it->second != 0) {
        Node& n = *it->second;
        n.checkType(type);
        n.unlink();
        n.link(m_newest);
        return n.m_size;
    } else {
        return afl::base::Nothing;
    }
}

void
server::file::ca::InternalObjectCache::trimCache()
{
    if (m_numObjects > m_maxObjects || m_numBytes > m_maxBytes) {
        size_t limitObjects = m_maxObjects * 3/4;
        size_t limitBytes = m_maxBytes * 3/4;
        size_t didObjects = 0;
        size_t didBytes = 0;
        Node** p = &m_newest;
        while (Node* node = *p) {
            if (didObjects >= limitObjects) {
                // Must remove this node
                removeObject(node->m_id);
            } else if (didBytes >= limitBytes) {
                // Can keep this node but must remove its content
                didObjects++;
                m_numBytes -= node->releaseMemory();
                p = &node->m_next;
            } else {
                // Can keep this node and content
                didObjects++;
                if (node->m_content.get() != 0) {
                    didBytes += node->m_content->get().size();
                }
                p = &node->m_next;
            }
        }
    }
}

server::file::ca::InternalObjectCache::Node::Node(const ObjectId& id, ObjectStore::Type type, afl::base::Ref<afl::io::FileMapping> content)
    : m_id(id),
      m_type(type),
      m_content(content.asPtr()),
      m_size(content->get().size()),
      m_next(0),
      m_pThis(0)
{ }

server::file::ca::InternalObjectCache::Node::Node(const ObjectId& id, ObjectStore::Type type, size_t size)
    : m_id(id),
      m_type(type),
      m_content(),
      m_size(size),
      m_next(0),
      m_pThis(0)
{ }


server::file::ca::InternalObjectCache::Node::~Node()
{
    unlink();
}

void
server::file::ca::InternalObjectCache::Node::unlink()
{
    if (m_pThis) {
        *m_pThis = m_next;
        if (m_next) {
            m_next->m_pThis = m_pThis;
        }
        m_next = 0;
        m_pThis = 0;
    }
}

void
server::file::ca::InternalObjectCache::Node::link(Node*& first)
{
    // Prevent bad things by not allowing this if already was executed previously
    if (m_pThis == 0) {
        m_next = first;
        m_pThis = &first;
        if (m_next) {
            m_next->m_pThis = &m_next;
        }
        first = this;
    }
}

size_t
server::file::ca::InternalObjectCache::Node::releaseMemory()
{
    size_t result = 0;
    if (m_content.get() != 0) {
        result = m_content->get().size();
        m_content.reset();
    }
    return result;
}

void
server::file::ca::InternalObjectCache::Node::checkType(ObjectStore::Type type)
{
    if (type != m_type) {
        throw afl::except::FileProblemException(m_id.toHex(), HASH_COLLISION);
    }
}
