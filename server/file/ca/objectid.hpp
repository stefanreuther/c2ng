/**
  *  \file server/file/ca/objectid.hpp
  *  \brief Structure server::file::ca::ObjectId
  */
#ifndef C2NG_SERVER_FILE_CA_OBJECTID_HPP
#define C2NG_SERVER_FILE_CA_OBJECTID_HPP

#include "afl/base/types.hpp"
#include "afl/checksums/hash.hpp"

namespace server { namespace file { namespace ca {

    /** Object Id, binary form.
        This is a structure to represent the 20-bytes (SHA-1-based) object Id.
        It is a structure to allow initialisation as static constant objects. */
    struct ObjectId {
        /** Content. */
        uint8_t m_bytes[20];

        /** Create ObjectId from hash result.
            @param hash Hash
            @return object Id */
        static ObjectId fromHash(afl::checksums::Hash& hash);

        /** Create ObjectId from hex-string representation.
            @param hash Hash
            @return object Id */
        static ObjectId fromHex(const String_t& str);

        /** Convert to hex-string representation.
            @return text */
        String_t toHex() const;

        /** Compare for equality.
            @param other Other ObjectId
            @return true if both are equal */
        bool operator==(const ObjectId& other) const;

        /** Compare for inequality.
            @param other Other ObjectId
            @return true if both are different */
        bool operator!=(const ObjectId& other) const;

        /** Compare lexicographically.
            @param other Other ObjectId
            @return true if this ObjectId sorts before the other one */
        bool operator<(const ObjectId& other) const;

        /** Null object Id.
            This is the object Id representing SHA-1(""). */
        static const ObjectId nil;
    };

    /** makePrintable for testing.
        \param id Point */
    inline String_t makePrintable(const ObjectId& id)
    {
        return id.toHex();
    }

} } }

#endif
