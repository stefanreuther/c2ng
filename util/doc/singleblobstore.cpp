/**
  *  \file util/doc/singleblobstore.cpp
  *  \brief Class util::doc::SingleBlobStore
  *
  *  Derived (and totally stripped down) from the implementation in afl.
  *  We only need very little meta-information about the members, so we can optimize size.
  *  We do not need to support un-seekable files.
  *  However, we can extend the files.
  */

#include "util/doc/singleblobstore.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/checksums/sha1.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/except/unsupportedexception.hpp"
#include "afl/string/char.hpp"
#include "afl/string/hex.hpp"
#include "afl/string/messages.hpp"

using afl::checksums::ByteSum;
using afl::io::Stream;
using afl::string::ConstStringMemory_t;
using afl::string::StringMemory_t;

typedef afl::checksums::SHA1 Hash_t;

namespace {
    /* Get string field.
       The standard says names are ASCII only.
       We are therefore just passing them through. */
    String_t getString(afl::string::ConstStringMemory_t m)
    {
        m.trim(m.find('\0'));
        return afl::string::fromMemory(m);
    }

    /* Get number field.
       The standard says leading-zero-filled; we allow for a little more flexibility. */
    uint32_t getNumber(afl::string::ConstStringMemory_t m, Stream& stream)
    {
        // Skip leading whitespace
        const char* p = m.eat();
        while (p != 0 && afl::string::charIsSpace(*p)) {
            p = m.eat();
        }

        // Parse number
        uint32_t result = 0;
        while (p != 0 && (*p >= '0' && *p <= '7')) {
            result = 8*result + (*p - '0');
            p = m.eat();
        }

        // Skip trailing whitespace
        while (p != 0 && afl::string::charIsSpace(*p)) {
            p = m.eat();
        }

        // If still not at end, fail
        if (p != 0 && *p != '\0') {
            throw afl::except::FileFormatException(stream, afl::string::Messages::invalidNumber());
        }
        return result;
    }

    /* Store number field. */
    void storeNumber(StringMemory_t m, uint32_t num)
    {
        // Drop last
        m.removeEnd(1);

        // Fill digits
        size_t i = 0;
        while (char* p = m.atEnd(i)) {
            *p = char('0' + (num % 8));
            num /= 8;
            ++i;
        }
    }

    const char MAGIC[] = "ustar";
}



/*
 *  "ustar" header.
 *
 *  Specified on http://pubs.opengroup.org/onlinepubs/9699919799/utilities/pax.html
 *
 *  String fields are zero-terminated if they are less than their total size long.
 *  Numeric fields contain an octal number.
 */
struct util::doc::SingleBlobStore::UstarHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char unused[12];
};

bool
util::doc::SingleBlobStore::Key::operator<(const Key& other) const
{
    return std::memcmp(data, other.data, sizeof(data)) < 0;
}

bool
util::doc::SingleBlobStore::Key::operator==(const Key& other) const
{
    return std::memcmp(data, other.data, sizeof(data)) == 0;
}


/*
 *  SingleBlobStore
 */

util::doc::SingleBlobStore::SingleBlobStore(afl::base::Ref<afl::io::Stream> file)
    : m_file(file),
      m_index(),
      m_endPos(0)
{
    readFile();
}

util::doc::SingleBlobStore::~SingleBlobStore()
{ }

util::doc::BlobStore::ObjectId_t
util::doc::SingleBlobStore::addObject(afl::base::ConstBytes_t data)
{
    // Compute object Id.
    static_assert(Hash_t::HASH_SIZE == KEY_SIZE, "KEY_SIZE");
    Hash_t hasher;
    hasher.add(data);

    Key k;
    hasher.getHash(k.data);

    // Create if not already present
    ObjectId_t id = formatObjectId(k);
    if (m_index.find(k) == m_index.end()) {
        // Check size
        uint32_t size32 = static_cast<uint32_t>(data.size());
        if (size32 != data.size()) {
            throw afl::except::UnsupportedException("<blob too large>");
        }

        // Build name
        String_t fileName = id;
        fileName.insert(2, "/");

        // Build header
        UstarHeader h;
        afl::base::fromObject(h).fill(0);
        StringMemory_t(h.name).copyFrom(afl::string::toMemory(fileName));
        storeNumber(h.mode, 0644);
        storeNumber(h.uid, 0);
        storeNumber(h.gid, 0);
        storeNumber(h.size, size32);
        storeNumber(h.mtime, 0);
        StringMemory_t(h.chksum).fill(' ');
        h.typeflag = '0';
        StringMemory_t(h.magic).copyFrom(MAGIC);
        h.version[0] = '0';
        h.version[1] = '0';
        StringMemory_t(h.uname).copyFrom("root");
        StringMemory_t(h.gname).copyFrom("root");
        storeNumber(h.chksum, ByteSum().add(afl::base::fromObject(h), 0));

        // Compute size of padding
        uint32_t partial = size32 % 512;
        uint32_t pad = (partial != 0 ? 512 - partial : 0);

        // Write
        m_file->setPos(m_endPos);
        m_file->fullWrite(afl::base::fromObject(h));
        m_endPos += sizeof(h);
        Stream::FileSize_t pos = m_endPos;

        m_file->fullWrite(data);
        m_endPos += size32;

        afl::base::fromObject(h).fill(0);
        m_file->fullWrite(afl::base::fromObject(h).trim(pad));
        m_endPos += pad;

        // Remember index
        m_index.insert(std::make_pair(k, Address(pos, size32)));
    }
    return formatObjectId(k);
}

afl::base::Ref<afl::io::FileMapping>
util::doc::SingleBlobStore::getObject(const ObjectId_t& id) const
{
    // Parse
    Key k;
    if (!parseObjectId(id, k)) {
        throw afl::except::FileProblemException(id, afl::string::Messages::fileNotFound());
    }

    // Look up
    std::map<Key, Address>::const_iterator it = m_index.find(k);
    if (it == m_index.end()) {
        throw afl::except::FileProblemException(id, afl::string::Messages::fileNotFound());
    }

    // Produce result
    m_file->setPos(it->second.pos);
    return m_file->createVirtualMapping(it->second.length);
}

void
util::doc::SingleBlobStore::readFile()
{
    static_assert(sizeof(UstarHeader) == 512, "sizeof UstarHeader");
    UstarHeader hdr;
    while (m_file->read(afl::base::fromObject(hdr)) == sizeof(UstarHeader)) {
        // Check name
        String_t name = getString(hdr.name);
        if (name.empty()) {
            // End of file reached. tar ends with an all-0 block
            break;
        }

        // Check magic. We guarantee to read only what we wrote, and since we'll update it, avoid poking in totally different files
        if (std::memcmp(hdr.magic, MAGIC, 5) != 0) {
            throw afl::except::FileFormatException(*m_file, "<bad magic>");
        }

        // Parse prefix, for completeness (we never set it)
        String_t prefix = getString(hdr.prefix);
        if (!prefix.empty()) {
            name = prefix + '/' + name;
        }

        // Remember positions
        const uint32_t size = getNumber(hdr.size, *m_file);
        const Stream::FileSize_t thisEntryPos = m_endPos + sizeof(hdr);
        const Stream::FileSize_t nextEntryPos = thisEntryPos + ((size + 511) & ~511);

        Key k;
        if ((hdr.typeflag == '0' || hdr.typeflag == '\0' || hdr.typeflag == '7') && parseMemberName(name, k)) {
            m_index.insert(std::make_pair(k, Address(thisEntryPos, size)));
        }

        // Next position
        m_file->setPos(nextEntryPos);
        m_endPos = nextEntryPos;
    }
}

bool
util::doc::SingleBlobStore::parseObjectId(const ObjectId_t& objId, Key& key)
{
    // Check size
    if (objId.size() != KEY_SIZE*2) {
        return false;
    }

    // Check and convert characters. Must be HEX_DIGITS_LOWER.
    for (size_t i = 0; i < objId.size(); ++i) {
        char ch = objId[i];
        int val = afl::string::getHexDigitValue(ch);
        if (val < 0 || ch != afl::string::HEX_DIGITS_LOWER[val]) {
            return false;
        }
        if ((i % 2) != 0) {
            key.data[i/2] |= uint8_t(val);
        } else {
            key.data[i/2] = uint8_t(val << 4);
        }
    }
    return true;
}

bool
util::doc::SingleBlobStore::parseMemberName(String_t name, Key& key)
{
    if (name.size() < 3 || name[2] != '/') {
        return false;
    } else {
        name.erase(2, 1);
        return parseObjectId(name, key);
    }
}

util::doc::BlobStore::ObjectId_t
util::doc::SingleBlobStore::formatObjectId(const Key& k)
{
    ObjectId_t result;
    for (size_t i = 0; i < KEY_SIZE; ++i) {
        afl::string::putHexByte(result, k.data[i], afl::string::HEX_DIGITS_LOWER);
    }
    return result;
}
