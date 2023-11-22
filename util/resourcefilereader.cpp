/**
  *  \file util/resourcefilereader.cpp
  *  \brief Class util::ResourceFileReader
  */

#include "util/resourcefilereader.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/limitedstream.hpp"
#include "util/resourcefile.hpp"
#include "afl/base/growablememory.hpp"

// Constructor.
util::ResourceFileReader::ResourceFileReader(afl::base::Ref<afl::io::Stream> file, afl::string::Translator& tx)
    : m_file(file),
      m_index()
{
    init(tx);
}

// Destructor.
util::ResourceFileReader::~ResourceFileReader()
{ }

// Open a resource file member as stream.
afl::base::Ptr<afl::io::Stream>
util::ResourceFileReader::openMember(uint16_t id)
{
    // ex ResourceStream::seekId
    for (size_t i = 0, n = m_index.size(); i < n; ++i) {
        if (id == m_index[i].id) {
            return openMemberByIndex(i);
        }
    }
    return 0;
}

// Open a resource file member as stream, by index.
afl::base::Ptr<afl::io::Stream>
util::ResourceFileReader::openMemberByIndex(size_t index)
{
    if (index < m_index.size()) {
        return new afl::io::LimitedStream(m_file->createChild(), m_index[index].position, m_index[index].length);
    } else {
        return 0;
    }
}

// Initialize.
void
util::ResourceFileReader::init(afl::string::Translator& tx)
{
    // Read header
    ResourceFile::Header header;
    m_file->fullRead(afl::base::fromObject(header));
    if (header.magic != ResourceFile::HEADER_MAGIC) {
        throw afl::except::FileFormatException(*m_file, tx("File is missing required signature"));
    }

    // Read index
    afl::base::GrowableMemory<ResourceFile::Entry> rawIndex;
    const size_t numEntries = header.numEntries;
    rawIndex.resize(numEntries);
    m_file->setPos(header.dirPosition);
    m_file->fullRead(rawIndex.toBytes());

    // Parse index
    m_index.resize(numEntries);
    for (size_t i = 0; i < numEntries; ++i) {
        const ResourceFile::Entry& in = *rawIndex.at(i);
        Entry& out = m_index[i];
        out.id       = in.id;
        out.position = in.position;
        out.length   = in.length;
    }
}

// Get number of members.
size_t
util::ResourceFileReader::getNumMembers() const
{
    return m_index.size();
}

// Get Member Id, given an index.
uint16_t
util::ResourceFileReader::getMemberIdByIndex(size_t index) const
{
    if (index < m_index.size()) {
        return m_index[index].id;
    } else {
        return 0;
    }
}

// Find primary member Id, given an index.
uint16_t
util::ResourceFileReader::findPrimaryIdByIndex(size_t index) const
{
    // ex rxall.pas:GetEntry (sort-of)
    if (index < m_index.size()) {
        uint32_t position = m_index[index].position;
        uint32_t length   = m_index[index].length;
        size_t i = 0;
        while (i < index && (m_index[i].position != position || m_index[i].length != length)) {
            ++i;
        }
        return m_index[i].id;
    } else {
        return 0;
    }
}
