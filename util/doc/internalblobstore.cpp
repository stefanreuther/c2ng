/**
  *  \file util/doc/internalblobstore.cpp
  *  \brief Class util::doc::InternalBlobStore
  */

#include "util/doc/internalblobstore.hpp"
#include "afl/checksums/sha1.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilemapping.hpp"
#include "afl/string/messages.hpp"

using afl::base::GrowableBytes_t;
using afl::checksums::SHA1;
using afl::except::FileProblemException;
using afl::io::InternalFileMapping;
using afl::string::Messages;

util::doc::InternalBlobStore::InternalBlobStore()
    : m_content()
{ }

util::doc::InternalBlobStore::~InternalBlobStore()
{ }

util::doc::BlobStore::ObjectId_t
util::doc::InternalBlobStore::addObject(afl::base::ConstBytes_t data)
{
    SHA1 hasher;
    hasher.add(data);
    const ObjectId_t id = hasher.getHashAsHexString();

    m_content[id] = afl::string::fromBytes(data);
    return id;
}

afl::base::Ref<afl::io::FileMapping>
util::doc::InternalBlobStore::getObject(const ObjectId_t& id) const
{
    std::map<String_t, String_t>::const_iterator it = m_content.find(id);
    if (it == m_content.end()) {
        throw FileProblemException(id, Messages::fileNotFound());
    }

    GrowableBytes_t data;
    data.append(afl::string::toBytes(it->second));
    return *new InternalFileMapping(data);
}
