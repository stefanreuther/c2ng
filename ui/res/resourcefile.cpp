/**
  *  \file ui/res/resourcefile.cpp
  *  \brief Class ui::res::ResourceFile
  */

#include "ui/res/resourcefile.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/limitedstream.hpp"

namespace {
    typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;
    typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;

    /// PCC 1.x resource file header.
    struct RawHeader {
        UInt16_t magic;
        UInt32_t dirPosition;
        UInt16_t numEntries;
    };
    static_assert(sizeof(RawHeader) == 8, "sizeof RawHeader");

    /// PCC 1.x resource file member (index entry).
    struct RawEntry {
        UInt16_t id;
        UInt32_t position;
        UInt32_t length;
    };
    static_assert(sizeof(RawEntry) == 10, "sizeof RawEntry");
}

// Constructor.
ui::res::ResourceFile::ResourceFile(afl::base::Ref<afl::io::Stream> file, afl::string::Translator& tx)
    : m_file(file),
      m_index()
{
    init(tx);
}

// Destructor.
ui::res::ResourceFile::~ResourceFile()
{ }

// Open a resource file member as stream.
afl::base::Ptr<afl::io::Stream>
ui::res::ResourceFile::openMember(uint16_t id)
{
    // ex ResourceStream::seekId
    for (size_t i = 0, n = m_index.size(); i < n; ++i) {
        if (id == m_index[i].id) {
            return new afl::io::LimitedStream(m_file->createChild(), m_index[i].position, m_index[i].length);
        }
    }
    return 0;
}

// Initialize.
void
ui::res::ResourceFile::init(afl::string::Translator& tx)
{
    // Read header
    RawHeader header;
    m_file->fullRead(afl::base::fromObject(header));
    if (header.magic != 0x5A52 /* 'RZ' */) {
        throw afl::except::FileFormatException(*m_file, tx("File is missing required signature"));
    }

    // Read index
    afl::base::GrowableMemory<RawEntry> rawIndex;
    const size_t numEntries = header.numEntries;
    rawIndex.resize(numEntries);
    m_file->setPos(header.dirPosition);
    m_file->fullRead(rawIndex.toBytes());

    // Parse index
    m_index.resize(numEntries);
    for (size_t i = 0; i < numEntries; ++i) {
        const RawEntry& in = *rawIndex.at(i);
        Entry& out = m_index[i];
        out.id       = in.id;
        out.position = in.position;
        out.length   = in.length;
    }
}
