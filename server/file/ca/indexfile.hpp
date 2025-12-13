/**
  *  \file server/file/ca/indexfile.hpp
  *  \brief Class server::file::ca::IndexFile
  */
#ifndef C2NG_SERVER_FILE_CA_INDEXFILE_HPP
#define C2NG_SERVER_FILE_CA_INDEXFILE_HPP

#include "afl/base/staticassert.hpp"
#include "afl/io/datasink.hpp"
#include "afl/io/stream.hpp"
#include "server/file/ca/objectid.hpp"

namespace server { namespace file { namespace ca {

    /** Index file for an object pack.
        An index file contains positions and CRCs for a set of objects; one such metadata element is an "Item".
        This class supports reading and writing of the pack file. */
    class IndexFile {
     public:
        struct Item {
            ObjectId id;
            uint32_t crc;
            uint64_t pos;

            bool operator<(const Item& other) const
                { return id < other.id; }
        };

        /** Constructor. Make an empty index file. */
        IndexFile();

        /** Destructor. */
        ~IndexFile();

        /** Load from stream.
            @param in Input stream
            @return "pack Id" (=SHA1 of pack file content) found in trailer, must match pack file */
        server::file::ca::ObjectId load(afl::io::Stream& in);

        /** Save to stream.
            @param out Output stream
            @param packId "pack Id" (=SHA1 of pack file content) to store in trailer */
        void save(afl::io::DataSink& out, const server::file::ca::ObjectId& packId);

        /** Find an item, given an object Id.
            @param id Object Id
            @return Item; null if not found. The return value is valid until the next function call on this object. */
        const Item* findItem(const ObjectId& id) const;

        /** Add an item.
            @param id Object Id
            @param crc  CRC of object data
            @param pos  Position in pack file
            @pre findItem(id) == null */
        void addItem(const ObjectId& id, uint32_t crc, uint64_t pos);

     private:
        /** Sorted items.
            This is used for most items for fast O(logn) access.
            A loaded file will have all items in this field as a single allocation.

            At 500k objects, this will be a permanent 16M allocation. */
        std::vector<Item> m_sortedItems;

        /** Unsorted items.
            This is used to collect new items, and is eventually merged with m_sortedItems. */
        std::vector<Item> m_unsortedItems;

        /** Merge item vectors.
            Add all elements of m_unsortedItems to m_sortedItems, emptying the former in the process. */
        void merge();
    };

    static_assert(sizeof(IndexFile::Item) == 32, "sizeof Item");

} } }

#endif
