/**
  *  \file server/file/ca/indexfile.cpp
  *  \brief Class server::file::ca::IndexFile
  */

#include "server/file/ca/indexfile.hpp"

#include "afl/bits/uint32be.hpp"
#include "afl/bits/uint64be.hpp"
#include "afl/bits/value.hpp"
#include "afl/checksums/sha1.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/datasink.hpp"
#include "afl/string/messages.hpp"

using afl::base::Memory;
using server::file::ca::IndexFile;
using server::file::ca::ObjectId;

namespace {
    /*
     *  File Format
     *
     *  UInt32_t     magic         (='\xfftOc')
     *  UInt32_t     version       (=2)
     *  UInt32_t     fanout[256]   (=one-past-last object whose Id starts with this byte; [255] gives number of objects)
     *  ObjectId     ids[N]        (=object Ids, sorted lexicographically)
     *  UInt32_t     crc[N]        (=CRCs of objects in pack file)
     *  UInt32_t     pos[N]        (=positions of objects in pack file. If position is >2G, value is 0x80000000+X, where X is index into overflow list below)
     *  UInt64_t     overflow[X]   (=positions of objects in pack file. Index given by maximum X appearing in pos[], plus 1)
     *  ObjectId     packId        (=hash of pack file)
     *  ObjectId     indexId       (=hash of everything before this field)
     */
    typedef afl::bits::Value<afl::bits::UInt32BE> UInt32_t;
    typedef afl::bits::Value<afl::bits::UInt64BE> UInt64_t;

    struct Header {
        UInt32_t magic;
        UInt32_t version;
        UInt32_t fanout[256];
    };
    static_assert(sizeof(Header) == 1032, "sizeof Header");

    const uint32_t HEADER_MAGIC = 0xFF744F63;
    const uint32_t HEADER_VERSION = 2;
    const uint32_t OVERFLOW_SIZE_MARK = 0x80000000;

    /*
     *  Parameters
     */

    /* Maximum number of unsorted elements.
       This tries to balance speed between having objects in the sorted list (O(logn) access, O(n) insert)
       and in the unsorted list (O(n) access, O(1) insert). */
    const size_t MAX_UNSORTED = 1024;

    /* Target chunk size, bytes.
       We try to produce I/O of this size, as a compromise of "safe to put on stack/cache" and "large I/O".
       As an exception, the "size overflow" field is always built as a whole, and on the heap,
       because breaking it into chunks would require more complex data structures. */
    const size_t CHUNK_SIZE = 20480;

    /* A DataSink that writes into another sink, and hashes the data at the same time */
    class HashingSink : public afl::io::DataSink {
     public:
        HashingSink(afl::checksums::Hash& hash, afl::io::DataSink& sink)
            : m_hash(hash), m_sink(sink)
            { }
        bool handleData(afl::base::ConstBytes_t& data)
            {
                m_hash.add(data);
                return m_sink.handleData(data);
            }
     private:
        afl::checksums::Hash& m_hash;
        afl::io::DataSink& m_sink;
    };

    /*
     *  Reading
     */

    void readObjectIds(afl::io::Stream& in, std::vector<IndexFile::Item>& items, size_t numObjects)
    {
        size_t index = 0;
        while (index < numObjects) {
            const size_t CHUNK = CHUNK_SIZE / sizeof(ObjectId);
            ObjectId buffer[CHUNK];

            size_t todo = std::min(numObjects - index, CHUNK);
            in.fullRead(Memory<ObjectId>(buffer).trim(todo).toBytes());

            for (size_t i = 0; i < todo; ++i) {
                if (index != 0 && !(items[index-1].id < buffer[i])) {
                    throw afl::except::FileFormatException(in, "File format error: objects not sorted");
                }
                items[index++].id = buffer[i];
            }
        }
    }

    void readObjectCRCs(afl::io::Stream& in, std::vector<IndexFile::Item>& items, size_t numObjects)
    {
        size_t index = 0;
        while (index < numObjects) {
            const size_t CHUNK = CHUNK_SIZE / sizeof(UInt32_t);
            UInt32_t buffer[CHUNK];

            size_t todo = std::min(numObjects - index, CHUNK);
            in.fullRead(Memory<UInt32_t>(buffer).trim(todo).toBytes());

            for (size_t i = 0; i < todo; ++i) {
                items[index++].crc = buffer[i];
            }
        }
    }

    void readObjectPositions(afl::io::Stream& in, std::vector<IndexFile::Item>& items, size_t numObjects)
    {
        size_t index = 0;
        uint32_t maxOverflow = 0;

        // Plain
        while (index < numObjects) {
            const size_t CHUNK = CHUNK_SIZE / sizeof(UInt32_t);
            UInt32_t buffer[CHUNK];

            size_t todo = std::min(numObjects - index, CHUNK);
            in.fullRead(Memory<UInt32_t>(buffer).trim(todo).toBytes());

            for (size_t i = 0; i < todo; ++i) {
                uint32_t me = buffer[i];
                items[index++].pos = me;
                if ((me & OVERFLOW_SIZE_MARK) != 0) {
                    me &= ~OVERFLOW_SIZE_MARK;
                    maxOverflow = std::max(me+1, maxOverflow);
                }
            }
        }

        // Overflow
        if (maxOverflow != 0) {
            std::vector<UInt64_t> buffer(maxOverflow);
            in.fullRead(Memory<UInt64_t>(buffer).toBytes());

            for (size_t i = 0; i < numObjects; ++i) {
                if ((items[i].pos & OVERFLOW_SIZE_MARK) != 0) {
                    items[i].pos = buffer[items[i].pos & ~OVERFLOW_SIZE_MARK];
                }
            }
        }
    }

    /*
     *  Writing
     */

    void buildFanout(Header& out, const std::vector<IndexFile::Item>& vec)
    {
        size_t index = 0;
        size_t max = vec.size();
        for (int i = 0; i < 256; ++i) {
            while (index < max && vec[index].id.m_bytes[0] == i) {
                ++index;
            }
            out.fanout[i] = static_cast<uint32_t>(index);
        }
    }

    void saveObjectIds(afl::io::DataSink& sink, const std::vector<IndexFile::Item>& vec)
    {
        size_t index = 0;
        const size_t numObjects = vec.size();
        while (index < numObjects) {
            const size_t CHUNK = CHUNK_SIZE / sizeof(ObjectId);
            ObjectId buffer[CHUNK];

            size_t todo = std::min(numObjects - index, CHUNK);
            for (size_t i = 0; i < todo; ++i) {
                buffer[i] = vec[index++].id;
            }
            sink.handleFullData(Memory<ObjectId>(buffer).trim(todo).toBytes());
        }
    }

    void saveObjectCRCs(afl::io::DataSink& sink, const std::vector<IndexFile::Item>& vec)
    {
        size_t index = 0;
        const size_t numObjects = vec.size();
        while (index < numObjects) {
            const size_t CHUNK = CHUNK_SIZE / sizeof(UInt32_t);
            UInt32_t buffer[CHUNK];

            size_t todo = std::min(numObjects - index, CHUNK);
            for (size_t i = 0; i < todo; ++i) {
                buffer[i] = vec[index++].crc;
            }
            sink.handleFullData(Memory<UInt32_t>(buffer).trim(todo).toBytes());
        }
    }

    void saveObjectPositions(afl::io::DataSink& sink, const std::vector<IndexFile::Item>& vec)
    {
        size_t index = 0;
        const size_t numObjects = vec.size();

        std::vector<UInt64_t> overflow;
        while (index < numObjects) {
            const size_t CHUNK = CHUNK_SIZE / sizeof(UInt32_t);
            UInt32_t buffer[CHUNK];

            size_t todo = std::min(numObjects - index, CHUNK);
            for (size_t i = 0; i < todo; ++i) {
                uint64_t pos = vec[index++].pos;
                if (pos > 0x7FFFFFFF) {
                    buffer[i] = static_cast<uint32_t>(OVERFLOW_SIZE_MARK + overflow.size());
                    overflow.push_back(UInt64_t());
                    overflow.back() = pos;
                } else {
                    buffer[i] = static_cast<uint32_t>(pos);
                }
            }
            sink.handleFullData(Memory<UInt32_t>(buffer).trim(todo).toBytes());
        }
        sink.handleFullData(Memory<UInt64_t>(overflow).toBytes());
    }

    /*
     *  Access
     */

    // Find in sorted vector
    const IndexFile::Item* findSorted(const std::vector<IndexFile::Item>& vec, const ObjectId& id)
    {
        // Binary intersection
        size_t min = 0, max = vec.size();
        while (max - min > 5) {
            size_t mid = min + (max-min)/2;
            if (id < vec[mid].id) {
                max = mid;
            } else {
                min = mid;
            }
        }

        // Pinpoint result
        for (size_t i = min; i < max; ++i) {
            if (vec[i].id == id) {
                return &vec[i];
            }
        }
        return 0;
    }

    // Find in unsorted vector
    const IndexFile::Item* findUnsorted(const std::vector<IndexFile::Item>& vec, const ObjectId& id)
    {
        for (size_t i = 0; i < vec.size(); ++i) {
            if (vec[i].id == id) {
                return &vec[i];
            }
        }
        return 0;
    }
}


/*
 *  IndexFile
 */

server::file::ca::IndexFile::IndexFile()
    : m_sortedItems(),
      m_unsortedItems()
{ }

server::file::ca::IndexFile::~IndexFile()
{ }

server::file::ca::ObjectId
server::file::ca::IndexFile::load(afl::io::Stream& in)
{
    // Header
    Header header;
    in.fullRead(afl::base::fromObject(header));
    if (header.magic != HEADER_MAGIC || header.version != HEADER_VERSION) {
        throw afl::except::FileFormatException(in, "Unsupported file format");
    }

    // Number of objects
    const uint32_t numObjects = header.fanout[255];
    m_sortedItems.clear();
    m_unsortedItems.clear();
    m_sortedItems.resize(numObjects);

    // Read content
    readObjectIds(in, m_sortedItems, numObjects);
    readObjectCRCs(in, m_sortedItems, numObjects);
    readObjectPositions(in, m_sortedItems, numObjects);

    // Pack Id
    ObjectId result;
    in.fullRead(afl::base::fromObject(result));
    // Ignored: index file Id

    return result;
}

void
server::file::ca::IndexFile::save(afl::io::DataSink& out, const server::file::ca::ObjectId& packId)
{
    // Merge unprocessed data into sorted list
    merge();

    // Compute hash while saving
    afl::checksums::SHA1 sha1;
    HashingSink sink(sha1, out);

    // Header
    Header header;
    header.magic = HEADER_MAGIC;
    header.version = HEADER_VERSION;
    buildFanout(header, m_sortedItems);
    sink.handleFullData(afl::base::fromObject(header));

    // Content
    saveObjectIds(sink, m_sortedItems);
    saveObjectCRCs(sink, m_sortedItems);
    saveObjectPositions(sink, m_sortedItems);

    // Pack Id
    sink.handleFullData(afl::base::fromObject(packId));

    // Index file Id
    ObjectId indexId = ObjectId::fromHash(sha1);
    sink.handleFullData(afl::base::fromObject(indexId));
}

const server::file::ca::IndexFile::Item*
server::file::ca::IndexFile::findItem(const ObjectId& id) const
{
    // Binary search in m_sortedItems
    if (const Item* p = findSorted(m_sortedItems, id)) {
        return p;
    }

    // Linear search in m_unsortedItems
    if (const Item* p = findUnsorted(m_unsortedItems, id)) {
        return p;
    }

    // Not found
    return 0;
}

void
server::file::ca::IndexFile::addItem(const ObjectId& id, uint32_t crc, uint64_t pos)
{
    // Create item
    Item it;
    it.id = id;
    it.crc = crc;
    it.pos = pos;
    m_unsortedItems.push_back(it);

    // Drain overflow
    if (m_unsortedItems.size() >= MAX_UNSORTED) {
        merge();
    }
}

void
server::file::ca::IndexFile::merge()
{
    if (!m_unsortedItems.empty()) {
        m_sortedItems.reserve(m_sortedItems.size() + m_unsortedItems.size());
        m_sortedItems.insert(m_sortedItems.end(), m_unsortedItems.begin(), m_unsortedItems.end());
        m_unsortedItems.clear();
        std::sort(m_sortedItems.begin(), m_sortedItems.end());
    }
}
