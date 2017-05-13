/**
  *  \file server/file/ca/objectid.hpp
  */
#ifndef C2NG_SERVER_FILE_CA_OBJECTID_HPP
#define C2NG_SERVER_FILE_CA_OBJECTID_HPP

#include "afl/base/types.hpp"
#include "afl/checksums/hash.hpp"

namespace server { namespace file { namespace ca {

    struct ObjectId {
        uint8_t m_bytes[20];

        static ObjectId fromHash(afl::checksums::Hash& hash);
        static ObjectId fromHex(const String_t& str);

        String_t toHex() const;

        bool operator==(const ObjectId& other) const;
        bool operator!=(const ObjectId& other) const;

        bool operator<(const ObjectId& other) const;

        // fromHex, toHex, etc.

        static const ObjectId nil;
    };

} } }

#endif
