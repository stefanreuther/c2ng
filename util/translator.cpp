/**
  *  \file util/translator.cpp
  *  \brief Class util::Translator
  */

#include "util/translator.hpp"

#include <cstring>

#include "afl/base/staticassert.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/string/languagecode.hpp"
#include "afl/sys/mutexguard.hpp"

namespace {
    /*
     *  File Format
     *
     *  This is the same format as in PCC2.
     *  A language file consists of a header containing pointers to two pointer arrays and two text arrays.
     *  The pointer arrays contain pairs of address, length.
     *  The address is relative to the corresponding text array.
     *  The length includes the mandatory final null byte.
     */

    using afl::bits::UInt32LE;
    typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;
    typedef UInt32LE::Bytes_t Bytes_t;

    const uint8_t SIGNATURE[] = {'C','C','l','a','n','g','0',26};

    struct Header {
        uint8_t signature[sizeof(SIGNATURE)];  ///< File magic number.
        UInt32_t count;                        ///< Number of strings.
        UInt32_t inPtr;                        ///< Address of "input pointers" array. Size is 2*count words.
        UInt32_t outPtr;                       ///< Address of "output pointers" array. Size is 2*count words.
        UInt32_t inText;                       ///< Address of "input text" array.
        UInt32_t inSize;                       ///< Size of "input text" array in bytes.
        UInt32_t outText;                      ///< Address of "output text" array.
        UInt32_t outSize;                      ///< Size of "output text" array in bytes.
    };
    static_assert(sizeof(Header) == 36, "sizeof Header");


    /* Punctuation characters that are not translated.
       For 'translate("foo:"), we look up "foo". */
    const char*const PUNCT_CHARS = "\n: ";

    size_t findSuffixLength(afl::string::ConstStringMemory_t mem)
    {
        size_t n = 0;
        while (const char* p = mem.atEnd(n)) {
            if (std::strchr(PUNCT_CHARS, *p) == 0) {
                break;
            }
            ++n;
        }
        return n;
    }

    String_t unpackString(afl::base::ConstBytes_t mem)
    {
        return afl::string::fromBytes(mem.subrange(0, mem.find(0)));
    }
}

util::Translator::Translator()
    : m_mutex(),
      m_map()
{ }

util::Translator::~Translator()
{ }

void
util::Translator::clear()
{
    // StringTranslator::clear
    afl::sys::MutexGuard g(m_mutex);
    m_map.clear();
}

void
util::Translator::addTranslation(const String_t& orig, const String_t& result)
{
    // ex StringTranslator::addTranslation
    if (!orig.empty()) {
        afl::sys::MutexGuard g(m_mutex);
        m_map.insert(std::make_pair(orig, result));
    }
}

void
util::Translator::loadFile(afl::io::Stream& s)
{
    // ex StringTranslator::loadLanguageFile(Stream& s)
    // The file was originally intended to be mmap-capable.
    // For simplicity, we just copy.
    afl::base::Ref<afl::io::FileMapping> map = s.createVirtualMapping();
    afl::base::ConstBytes_t mem = map->get();
    const size_t size = mem.size();

    Header hdr;
    afl::base::fromObject(hdr).copyFrom(mem);
    if (size < sizeof(Header) || !afl::base::ConstBytes_t(hdr.signature).equalContent(SIGNATURE)) {
        throw afl::except::FileFormatException(s, "File is missing required signature");
    }

    const uint32_t count = hdr.count;
    const uint32_t inPtr = hdr.inPtr;
    const uint32_t outPtr = hdr.outPtr;
    const uint32_t inText = hdr.inText;
    const uint32_t inSize = hdr.inSize;
    const uint32_t outText = hdr.outText;
    const uint32_t outSize = hdr.outSize;

    if (inPtr >= size || outPtr >= size || inText >= size || outText >= size
        || count > (size - inPtr)/8
        || count > (size - outPtr)/8
        || inSize > size - inText
        || outSize > size - outText)
    {
        throw afl::except::FileFormatException(s, "File is invalid");
    }

    // File is valid
    afl::base::ConstBytes_t in = mem.subrange(inPtr);
    afl::base::ConstBytes_t out = mem.subrange(outPtr);
    for (uint32_t i = 0; i < count; ++i) {
        const Bytes_t* rawInPtr = in.eatN<4>();
        const Bytes_t* rawInLen = in.eatN<4>();
        const Bytes_t* rawOutPtr = out.eatN<4>();
        const Bytes_t* rawOutLen = out.eatN<4>();
        if (rawInPtr == 0 || rawInLen == 0 || rawOutPtr == 0 || rawOutLen == 0) {
            // Cannot happen due to size check above
            throw afl::except::FileFormatException(s, "File is invalid");
        }

        const uint32_t thisInPtr  = UInt32LE::unpack(*rawInPtr);
        const uint32_t thisInLen  = UInt32LE::unpack(*rawInLen);
        const uint32_t thisOutPtr = UInt32LE::unpack(*rawOutPtr);
        const uint32_t thisOutLen = UInt32LE::unpack(*rawOutLen);
        if (thisInPtr > inSize || thisOutPtr > outSize
            || thisInLen > inSize - thisInPtr || thisOutLen > outSize - thisOutPtr)
        {
            throw afl::except::FileFormatException(s, "File is invalid");
        }

        addTranslation(unpackString(mem.subrange(inText + thisInPtr, thisInLen)),
                       unpackString(mem.subrange(outText + thisOutPtr, thisOutLen)));
    }
}

void
util::Translator::loadDefaultTranslation(afl::io::FileSystem& fs, afl::sys::Environment& env)
{
    // ex loadLanguageDatabase(const char* domain, const char* fn), initNLS()
    loadTranslation(fs, env, env.getUserLanguage());
}

void
util::Translator::loadTranslation(afl::io::FileSystem& fs, afl::sys::Environment& env, afl::string::LanguageCode code)
{
    try {
        clear();
        String_t resourceDir = fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "resource");
        while (code.isValid()) {
            afl::base::Ptr<afl::io::Stream> file = fs.openFileNT(fs.makePathName(resourceDir, afl::string::strLCase(code.getWithDelimiter('-')) + ".lang"), afl::io::FileSystem::OpenRead);
            if (file.get() != 0) {
                loadFile(*file);
                break;
            }
            if (!code.generalize()) {
                break;
            }
        }
    }
    catch (afl::except::FileProblemException& e) {
        // Can we log it?
    }
}

String_t
util::Translator::translate(afl::string::ConstStringMemory_t in) const
{
    // ex StringTranslator::translate, sort-of
    // Find possible suffix
    size_t n = findSuffixLength(in);
    size_t cutoff = in.size() - n;

    // Look up
    afl::sys::MutexGuard g(m_mutex);
    std::map<String_t, String_t>::const_iterator it = m_map.find(afl::string::fromMemory(in.subrange(0, cutoff)));
    if (it != m_map.end()) {
        // Found it
        return it->second + afl::string::fromMemory(in.subrange(cutoff));
    } else {
        // Not found
        // Remove possible metadata
        if (in.size() > 0 && *in.at(0) == '{') {
            size_t n = in.find('}');
            if (n < in.size()) {
                in.split(n + 1);
            }
        }
        return afl::string::fromMemory(in);
    }
}
