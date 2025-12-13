/**
  *  \file server/file/ca/packfile.cpp
  *  \brief Class server::file::ca::PackFile
  */

#include "server/file/ca/packfile.hpp"

#include "afl/base/growablememory.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/io/inflatetransform.hpp"
#include "afl/io/internalfilemapping.hpp"

const uint32_t server::file::ca::PackFile::MAGIC;
const uint32_t server::file::ca::PackFile::VERSION;

const uint8_t server::file::ca::PackFile::OBJ_COMMIT;
const uint8_t server::file::ca::PackFile::OBJ_TREE;
const uint8_t server::file::ca::PackFile::OBJ_BLOB;
const uint8_t server::file::ca::PackFile::OBJ_TAG;
const uint8_t server::file::ca::PackFile::OBJ_OFS_DELTA;
const uint8_t server::file::ca::PackFile::OBJ_REF_DELTA;

using afl::base::Ref;
using afl::io::FileMapping;
using afl::io::FileSystem;
using afl::io::Stream;
using server::file::ca::IndexFile;
using server::file::ca::ObjectId;

namespace {
    static_assert(sizeof(server::file::ca::PackFile::Header) == 12, "sizeof Header");

    ObjectId loadIndexFile(IndexFile& index, afl::io::Directory& dir, const String_t& baseName)
    {
        Ref<Stream> indexFile = dir.openFile(baseName + ".idx", FileSystem::OpenRead);
        return index.load(*indexFile);
    }
}

/*
 *  VarInt
 */

bool
server::file::ca::PackFile::VarInt::acceptByte(uint8_t byte)
{
    value |= static_cast<uint64_t>(byte & 0x7F) << shift;
    shift += 7;
    return (byte & 0x80) == 0;
}


/*
 *  OfsInt
 */

bool
server::file::ca::PackFile::OfsInt::acceptByte(uint8_t byte)
{
    value <<= 7;
    value |= (byte & 0x7F);
    if ((byte & 0x80) == 0) {
        return true;
    } else {
        ++value;
        return false;
    }
}


/*
 *  DeltaExpander
 */

server::file::ca::PackFile::DeltaExpander::DeltaExpander(const String_t& fileName, afl::base::Ref<afl::io::FileMapping> refObject, afl::base::GrowableBytes_t& result)
    : m_fileName(fileName),
      m_refObject(refObject),
      m_result(result),
      m_refObjectSize(),
      m_resultObjectSize(),
      m_state(ReadRefObjectSize),
      m_opcode(0),
      m_copyIndex(0)
{ }

bool
server::file::ca::PackFile::DeltaExpander::acceptBytes(afl::base::ConstBytes_t mem)
{
    while (1) {
        switch (m_state) {
         case ReadRefObjectSize:
            if (const uint8_t* p = mem.eat()) {
                if (m_refObjectSize.acceptByte(*p)) {
                    if (m_refObjectSize.value != m_refObject->get().size()) {
                        throw afl::except::FileFormatException(m_fileName, "Pack File: reference object size mismatch");
                    }
                    m_state = ReadResultObjectSize;
                }
                break;
            } else {
                return false;
            }

         case ReadResultObjectSize:
            if (const uint8_t* p = mem.eat()) {
                if (m_resultObjectSize.acceptByte(*p)) {
                    size_t resultSize = static_cast<size_t>(m_resultObjectSize.value);
                    if (resultSize != m_resultObjectSize.value) {
                        throw afl::except::FileFormatException(m_fileName, "Pack File: unsupported size");
                    }
                    m_result.reserve(resultSize);
                    m_state = ReadOpcode;
                }
                break;
            } else {
                return false;
            }

         case ReadOpcode:
            if (m_result.size() >= m_resultObjectSize.value) {
                m_state = End;
                break;
            } else {
                if (const uint8_t* p = mem.eat()) {
                    m_opcode = *p;
                    if (m_opcode == 0) {
                        throw afl::except::FileFormatException(m_fileName, "Pack File: invalid 0 opcode");
                    } else if ((m_opcode & 0x80) != 0) {
                        m_copyIndex = 0;
                        m_state = CopyData;
                    } else {
                        m_toAdd = m_opcode;
                        m_state = AddData;
                    }
                    break;
                } else {
                    return false;
                }
            }

         case CopyData:
            if (m_copyIndex >= 7) {
                uint32_t ofs = m_copyParameters[0] + 256*m_copyParameters[1] + 65536*m_copyParameters[2] + 16777216*m_copyParameters[3];
                uint32_t len = m_copyParameters[4] + 256*m_copyParameters[5] + 65536*m_copyParameters[6];
                if (len == 0) {
                    len = 0x10000;
                }
                afl::base::ConstBytes_t refData = m_refObject->get();
                if (ofs > refData.size() || len > refData.size() - ofs) {
                    throw afl::except::FileFormatException(m_fileName, "Pack File: invalid copy instruction");
                }
                m_result.append(refData.subrange(ofs, len));
                m_state = ReadOpcode;
                break;
            } else {
                if ((m_opcode & (1 << m_copyIndex)) != 0) {
                    if (const uint8_t* p = mem.eat()) {
                        m_copyParameters[m_copyIndex] = *p;
                        ++m_copyIndex;
                        break;
                    } else {
                        return false;
                    }
                } else {
                    m_copyParameters[m_copyIndex] = 0;
                    ++m_copyIndex;
                    break;
                }
            }

         case AddData:
            if (m_toAdd == 0) {
                m_state = ReadOpcode;
                break;
            } else {
                if (mem.empty()) {
                    return false;
                } else {
                    afl::base::ConstBytes_t now = mem.split(m_toAdd);
                    m_toAdd -= now.size();
                    m_result.append(now);
                    break;
                }
            }

         case End:
            return true;
        }
    }
}


/*
 *  PackFile
 */

server::file::ca::PackFile::PackFile(afl::io::Directory& dir, String_t baseName)
    : m_index(),
      m_file(dir.openFile(baseName + ".pack", FileSystem::OpenRead))
{
    // Load index
    ObjectId indexId = loadIndexFile(m_index, dir, baseName);

    // Validate header
    Header head;
    m_file->fullRead(afl::base::fromObject(head));
    if (head.magic != MAGIC || head.version != VERSION) {
        throw afl::except::FileFormatException(*m_file, "Unsupported file format");
    }

    // Pack must end in pack Id
    Stream::FileSize_t len = m_file->getSize();
    if (len < sizeof(Header) + sizeof(ObjectId)) {
        throw afl::except::FileTooShortException(*m_file);
    }
    ObjectId packId;
    m_file->setPos(len - sizeof(ObjectId));
    m_file->fullRead(packId.m_bytes);
    if (packId != indexId) {
        throw afl::except::FileFormatException(*m_file, "Index and pack file mismatch");
    }
}

server::file::ca::PackFile::~PackFile()
{ }

afl::base::Ptr<afl::io::FileMapping>
server::file::ca::PackFile::getObject(const ObjectId& id, ObjectRequester& req, size_t maxLevel)
{
    const IndexFile::Item* p = m_index.findItem(id);
    if (p == 0) {
        return 0;
    }

    return loadObject(p->pos, req, maxLevel).asPtr();
}

afl::base::Ref<afl::io::FileMapping>
server::file::ca::PackFile::loadObject(afl::io::Stream::FileSize_t pos, ObjectRequester& req, size_t maxLevel)
{
    m_file->setPos(pos);

    enum State {
        ReadTypeAndSize,
        DeflatePlain,
        ReadOffset,
        ReadObjectId,
        ExpandDelta,
        End
    };

    afl::base::GrowableBytes_t out;

    uint8_t inBytes[20480];
    afl::base::ConstBytes_t in;

    State st = ReadTypeAndSize;
    VarInt typeAndSize;
    uint8_t type = 0;
    uint64_t size = 0;

    OfsInt refOffset;
    ObjectId refObjectId;
    size_t refObjectIndex = 0;

    afl::io::InflateTransform inflater(afl::io::InflateTransform::Zlib);
    std::auto_ptr<DeltaExpander> expander;

    while (st != End) {
        // Fill buffer
        if (in.empty()) {
            afl::base::Bytes_t readBuf(inBytes);
            if (st == ReadTypeAndSize) {
                readBuf.trim(1024);
            }
            readBuf.trim(m_file->read(readBuf));
            in = readBuf;
            if (in.empty()) {
                throw afl::except::FileTooShortException(*m_file);
            }
        }

        // Process state
        switch (st) {
         case ReadTypeAndSize:
            if (typeAndSize.acceptByte(*in.eat())) {
                uint64_t size64 = static_cast<uint64_t>(((typeAndSize.value >> 7) << 4) + (typeAndSize.value & 15));
                type = static_cast<uint8_t>((typeAndSize.value >> 4) & 7);
                size = static_cast<size_t>(size64);
                if (size != size64) {
                    throw afl::except::FileFormatException(*m_file, "Pack File: unsupported size");
                }
                switch (type) {
                 case OBJ_COMMIT:
                 case OBJ_TREE:
                 case OBJ_BLOB:
                 case OBJ_TAG:
                    // Plain object
                    out.reserve(size);
                    st = DeflatePlain;
                    break;

                 case OBJ_OFS_DELTA:
                    // Offset, then delta
                    st = ReadOffset;
                    if (maxLevel == 0) {
                        throw afl::except::FileFormatException(*m_file, "Pack File: too many nested objects");
                    }
                    break;

                 case OBJ_REF_DELTA:
                    // Object ID, then delta
                    st = ReadObjectId;
                    if (maxLevel == 0) {
                        throw afl::except::FileFormatException(*m_file, "Pack File: too many nested objects");
                    }
                    break;

                 default:
                    throw afl::except::FileFormatException(*m_file, "Pack File: unsupported object encoding");
                }
            }
            break;

         case DeflatePlain: {
            // Deflate
            uint8_t inflateBytes[4096];
            afl::base::Bytes_t inflateBuffer(inflateBytes);
            size_t inputSize = in.size();
            inflateBuffer.trim(out.size() - size);
            inflater.transform(in, inflateBuffer);
            out.append(inflateBuffer);
            if (out.size() >= size) {
                st = End;
            } else {
                if (inflateBuffer.empty() && in.size() == inputSize) {
                    throw afl::except::FileFormatException(*m_file, "Pack File: unexpected compression end 1");
                }
            }
            break;
         }

         case ReadOffset:
            // Read offset, then delta
            if (refOffset.acceptByte(*in.eat())) {
                // Valid reference?
                if (refOffset.value >= pos) {
                    throw afl::except::FileFormatException(*m_file, "Pack File: bad reference offset");
                }

                // Load reference object, preserving file position
                Stream::FileSize_t savedPos = m_file->getPos();
                Ref<FileMapping> refObject = loadObject(pos - refOffset.value, req, maxLevel-1);
                m_file->setPos(savedPos);

                // Build expander
                expander.reset(new DeltaExpander(m_file->getName(), refObject, out));
                st = ExpandDelta;
            }
            break;

         case ReadObjectId: {
            const uint8_t* p;
            while (refObjectIndex < sizeof(refObjectId.m_bytes) && (p = in.eat()) != 0) {
                refObjectId.m_bytes[refObjectIndex++] = *p;
            }
            if (refObjectIndex >= sizeof(refObjectId.m_bytes)) {
                // Found the object Id; try to load it
                Stream::FileSize_t savedPos = m_file->getPos();
                Ref<FileMapping> refObject = req.getObject(refObjectId, maxLevel-1);
                m_file->setPos(savedPos);

                // Build expander
                expander.reset(new DeltaExpander(m_file->getName(), *refObject, out));
                st = ExpandDelta;
            }
            break;
         }

         case ExpandDelta: {
            // Deflate and expand
            uint8_t inflateBytes[4096];
            afl::base::Bytes_t inflateBuffer(inflateBytes);
            size_t inputSize = in.size();
            inflater.transform(in, inflateBuffer);
            if (expander->acceptBytes(inflateBuffer)) {
                st = End;
            } else {
                if (inflateBuffer.empty() && in.size() == inputSize) {
                    throw afl::except::FileFormatException(*m_file, "Pack File: unexpected compression end 2");
                }
            }
            break;
         }

         case End:
            // Cannot happen
            break;
        }
    }

    return *new afl::io::InternalFileMapping(out);
}
