/**
  *  \file interpreter/vmio/chunkfile.cpp
  *  \brief Class interpreter::vmio::ChunkFile
  */

#include "interpreter/vmio/chunkfile.hpp"

#include "afl/except/fileformatexception.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/io/limitedstream.hpp"
#include "interpreter/vmio/structures.hpp"


/*
 *  Loader
 */

interpreter::vmio::ChunkFile::Loader::Loader(const afl::base::Ref<afl::io::Stream>& s, afl::string::Translator& tx)
    : m_stream(s),
      m_translator(tx),
      m_objectSize(0),
      m_propertyStream(),
      m_nextProperty(0),
      m_propertyId(0),
      m_nextObject(s->getPos()),
      m_properties()
{ }

void
interpreter::vmio::ChunkFile::Loader::consumeObjectSize(uint32_t needed)
{
    // ex ObjectLoader::checkObjSize
    if (needed > m_objectSize) {
        throw afl::except::FileFormatException(*m_stream, m_translator("Invalid size"));
    }
    m_objectSize -= needed;
}

bool
interpreter::vmio::ChunkFile::Loader::readObject(uint32_t& type, uint32_t& id)
{
    // Read header
    structures::ObjectHeader header;

    m_stream->setPos(m_nextObject);
    const size_t n = m_stream->read(afl::base::fromObject(header));
    if (n == 0) {
        return false;
    }
    if (n != sizeof(header)) {
        throw afl::except::FileTooShortException(*m_stream);
    }

    const uint32_t objectType  = header.type;
    const uint32_t objectId    = header.id;
    m_objectSize  = header.size;
    const uint32_t numProperties = header.numProperties;
    m_nextObject += sizeof(header) + m_objectSize;

    /* Validate */
    consumeObjectSize(numProperties * 8);

    // Read property headers
    m_properties.resize(2 * numProperties);
    m_stream->fullRead(m_properties.toBytes());

    // Initialize properties and skip first one
    m_nextProperty = m_stream->getPos();
    m_propertyId = 0;

    uint32_t tmp;
    readProperty(tmp, tmp);

    type = objectType;
    id = objectId;
    return true;
}

afl::io::Stream*
interpreter::vmio::ChunkFile::Loader::readProperty(uint32_t& id, uint32_t& count)
{
    // Do we have another property?
    UInt32_t* pCount = m_properties.at(2 * m_propertyId);
    UInt32_t* pSize  = m_properties.at(2 * m_propertyId + 1);
    if (!pCount || !pSize) {
        return 0;
    }

    // Check property
    uint32_t propertySize = *pSize;
    uint32_t propertyId = m_propertyId++;
    uint32_t propertyCount = *pCount;
    consumeObjectSize(propertySize);

    // Initialize content
    m_propertyStream = new afl::io::LimitedStream(m_stream, m_nextProperty, propertySize);
    m_nextProperty += propertySize;

    // Produce result
    id = propertyId;
    count = propertyCount;
    return m_propertyStream.get();
}

uint32_t
interpreter::vmio::ChunkFile::Loader::getNumProperties() const
{
    // m_properties contains two words per property ("/ 2"),
    // including the dummy 0 property ("- 1").
    return static_cast<uint32_t>(m_properties.size() / 2 - 1);
}

uint32_t
interpreter::vmio::ChunkFile::Loader::getPropertySize(uint32_t id) const
{
    UInt32_t* pSize  = m_properties.at(2 * id + 1);
    return pSize != 0 ? *pSize : 0;
}

uint32_t
interpreter::vmio::ChunkFile::Loader::getPropertyCount(uint32_t id) const
{
    UInt32_t* pCount = m_properties.at(2 * id);
    return pCount != 0 ? *pCount : 0;
}

/*
 *  Writer
 */

interpreter::vmio::ChunkFile::Writer::Writer(afl::io::Stream& s)
    : m_stream(s),
      m_header(),
      m_headerPosition(0),
      m_propertyIndex(0),
      m_thisPropertyPosition(0),
      m_properties()
{ }

void
interpreter::vmio::ChunkFile::Writer::writeHeader()
{
    m_stream.fullWrite(afl::base::fromObject(m_header));
    m_stream.fullWrite(m_properties.toBytes());
}

void
interpreter::vmio::ChunkFile::Writer::start(uint32_t type, uint32_t id, uint32_t nprop)
{
    ++nprop;
    m_header.type = type;
    m_header.id = id;
    m_header.size = 0;
    m_header.numProperties = nprop;
    m_headerPosition = m_stream.getPos();
    m_properties.clear();
    m_properties.resize(2*nprop);
    m_properties.fill(UInt32_t());
    m_propertyIndex = 1;
    writeHeader();
}

void
interpreter::vmio::ChunkFile::Writer::end()
{
    afl::io::Stream::FileSize_t end_pos = m_stream.getPos();
    m_header.size = uint32_t(end_pos - m_headerPosition - 4*4);
    m_stream.setPos(m_headerPosition);
    writeHeader();
    m_stream.setPos(end_pos);
}

void
interpreter::vmio::ChunkFile::Writer::startProperty(uint32_t count)
{
    m_thisPropertyPosition = m_stream.getPos();
    if (UInt32_t* p = m_properties.at(2*m_propertyIndex)) {
        *p = count;
    }
}

void
interpreter::vmio::ChunkFile::Writer::endProperty()
{
    if (UInt32_t* p = m_properties.at(2*m_propertyIndex + 1)) {
        *p = uint32_t(m_stream.getPos() - m_thisPropertyPosition);
    }
    ++m_propertyIndex;
}

/*
 *  ChunkFile
 */

uint32_t
interpreter::vmio::ChunkFile::loadObjectFileHeader(afl::base::Ref<afl::io::Stream> s, afl::string::Translator& tx)
{
    // Read header
    structures::ObjectFileHeader header;
    s->fullRead(afl::base::fromObject(header));
    if (std::memcmp(header.magic, structures::OBJECT_FILE_MAGIC, sizeof(header.magic)) != 0
        || header.version != structures::OBJECT_FILE_VERSION
        || header.zero != 0
        || header.headerSize < structures::OBJECT_FILE_HEADER_SIZE)
    {
        throw afl::except::FileFormatException(*s, tx("Invalid file header"));
    }

    // Adjust file pointer
    s->setPos(s->getPos() + header.headerSize - structures::OBJECT_FILE_HEADER_SIZE);

    return header.entry;
}

void
interpreter::vmio::ChunkFile::writeObjectFileHeader(afl::io::Stream& s, uint32_t entry)
{
    structures::ObjectFileHeader header;
    std::memcpy(header.magic, structures::OBJECT_FILE_MAGIC, sizeof(header.magic));
    header.version = structures::OBJECT_FILE_VERSION;
    header.zero = 0;
    header.headerSize = structures::OBJECT_FILE_HEADER_SIZE;
    header.entry = entry;
    s.fullWrite(afl::base::fromObject(header));
}
