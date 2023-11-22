/**
  *  \file util/resourcefile.hpp
  *  \brief Structure util::ResourceFile
  */
#ifndef C2NG_UTIL_RESOURCEFILE_HPP
#define C2NG_UTIL_RESOURCEFILE_HPP

#include "afl/base/staticassert.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"

namespace util {

    /** PCC 1.x resource file ("*.res") structure definitions.

        A resource file contains a variety of data,
        as elements addressed by a 16-bit number.

        The file consists of a header at offset 0,
        which gives the position of the directory,
        which gives the positions of the entries.

        No particular relation between directory and entry positions,
        or positions of the entries relative to each others, is required.
        Typically, entries appear sequentially, with the directory at the end. */
    struct ResourceFile {
        typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;
        typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;

        /// PCC 1.x resource file header.
        struct Header {
            UInt16_t magic;                 ///< HEADER_MAGIC.
            UInt32_t dirPosition;           ///< Position of directory (0-based).
            UInt16_t numEntries;            ///< Number of directory entries.
        };
        static const uint16_t HEADER_MAGIC = 0x5A52; /* 'RZ' */

        /// PCC 1.x resource file member (index entry).
        struct Entry {
            UInt16_t id;                    ///< ID of entry.
            UInt32_t position;              ///< Position in file (0-based).
            UInt32_t length;                ///< Size in bytes.
        };
    };

    static_assert(sizeof(ResourceFile::Header) == 8, "sizeof Header");
    static_assert(sizeof(ResourceFile::Entry) == 10, "sizeof Entry");
}

#endif
