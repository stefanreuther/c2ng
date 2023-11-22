/**
  *  \file util/resourcefilewriter.cpp
  *  \brief Class util::ResourceFileWriter
  */

#include "util/resourcefilewriter.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/limitedstream.hpp"
#include "util/resourcefile.hpp"

util::ResourceFileWriter::ResourceFileWriter(afl::base::Ref<afl::io::Stream> file, afl::string::Translator& tx)
    : m_file(file),
      m_translator(tx),
      m_fileOpen(false),
      m_memberOpen(false),
      m_index()
{
    // Dummy header
    ResourceFile::Header h;
    afl::base::Bytes_t bytes(afl::base::fromObject(h));
    bytes.fill(0);

    // Write at position 0
    m_file->setPos(0);
    m_file->fullWrite(bytes);

    // File is now open
    m_fileOpen = true;
}

util::ResourceFileWriter::~ResourceFileWriter()
{ }

afl::base::Ref<afl::io::Stream>
util::ResourceFileWriter::createMember(uint16_t id)
{
    // Check preconditions
    validateFileMustBeOpen();
    finishMember();
    validateMustHaveRoom();

    // Check current file position
    afl::io::Stream::FileSize_t pos = m_file->getSize();
    m_file->setPos(pos);

    // OK, do it
    m_index.push_back(Entry(id, validateFileSize(pos), 0));
    m_memberOpen = true;
    return *new afl::io::LimitedStream(m_file, pos, afl::io::LimitedStream::nil);
}

void
util::ResourceFileWriter::finishMember()
{
    if (m_memberOpen && !m_index.empty()) {
        // Remember the size
        Entry& e = m_index.back();
        afl::io::Stream::FileSize_t entrySize = m_file->getSize() - e.position;
        e.length = validateFileSize(entrySize);
    }

    // Mark closed
    // (We do not enforce that user now keeps their hands off the stream.)
    m_memberOpen = false;
}

bool
util::ResourceFileWriter::hasMember(uint16_t id) const
{
    return findMember(id) != 0;
}

bool
util::ResourceFileWriter::createHardlink(uint16_t oldId, uint16_t newId)
{
    // Check preconditions
    validateFileMustBeOpen();
    finishMember();
    validateMustHaveRoom();

    // Create if possible
    if (const Entry* e = findMember(oldId)) {
        m_index.push_back(Entry(newId, e->position, e->length));
        return true;
    } else {
        return false;
    }
}

void
util::ResourceFileWriter::finishFile()
{
    finishMember();

    if (m_fileOpen) {
        // Build directory
        afl::base::GrowableMemory<ResourceFile::Entry> rawIndex;
        const size_t n = m_index.size();
        rawIndex.resize(n);
        for (size_t i = 0; i < n; ++i) {
            ResourceFile::Entry& out = *rawIndex.at(i);
            const Entry& in = m_index[i];
            out.id       = in.id;
            out.position = in.position;
            out.length   = in.length;
        }

        // Write directory
        const afl::io::Stream::FileSize_t size = m_file->getSize();
        m_file->setPos(size);
        m_file->fullWrite(rawIndex.toBytes());

        // Write primary header
        ResourceFile::Header header;
        header.magic = ResourceFile::HEADER_MAGIC;
        header.numEntries = static_cast<uint16_t>(n);
        header.dirPosition = validateFileSize(size);
        m_file->setPos(0);
        m_file->fullWrite(afl::base::fromObject(header));

        // We're done now
        m_fileOpen = 0;
    }
}

const util::ResourceFileWriter::Entry*
util::ResourceFileWriter::findMember(uint16_t id) const
{
    for (size_t i = 0, n = m_index.size(); i < n; ++i) {
        if (m_index[i].id == id) {
            return &m_index[i];
        }
    }
    return 0;
}

void
util::ResourceFileWriter::validateFileMustBeOpen() const
{
    /* Failing this check is a programming error. */
    afl::except::checkAssertion(m_fileOpen, "<ResourceFileWriter.m_fileOpen>");
}

void
util::ResourceFileWriter::validateMustHaveRoom() const
{
    /* We limit the directory to 64k bytes.
       The hard file format limit is 64k entries, but these files are read by 16-bit programs
       that try to slurp the entire directory into one allocation. */
    const size_t MAX_ENTRIES = 65520 / sizeof(ResourceFile::Entry);
    if (m_index.size() >= MAX_ENTRIES) {
        throw afl::except::FileProblemException(*m_file, m_translator("Too many entries in file"));
    }
}

uint32_t
util::ResourceFileWriter::validateFileSize(afl::io::Stream::FileSize_t size) const
{
    /* The file format cannot represent offsets or sizes greater than 2G.
       (Hard file format limit would be 4G, but our consumers don't have u32.) */
    if (size > 0x7FFFFFFF) {
        throw afl::except::FileProblemException(*m_file, m_translator("File too large"));
    }
    return static_cast<uint32_t>(size);
}
