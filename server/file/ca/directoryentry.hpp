/**
  *  \file server/file/ca/directoryentry.hpp
  *  \brief Class server::file::ca::DirectoryEntry
  */
#ifndef C2NG_SERVER_FILE_CA_DIRECTORYENTRY_HPP
#define C2NG_SERVER_FILE_CA_DIRECTORYENTRY_HPP

#include "afl/base/growablememory.hpp"
#include "afl/base/memory.hpp"
#include "server/file/ca/objectid.hpp"
#include "server/file/directoryhandler.hpp"

namespace server { namespace file { namespace ca {

    /** Entry of a tree (directory) object.
        This class parses and formats individual directory entries.

        This class is round-trip compatibly, i.e. can preserve file permissions and unknown objects encountered on the way
        even though we do not support these otherwise.
        However, building a new DirectoryEntry from the parameters taken from an existing one
        (<tt>DirectoryEntry ne(e.getName(), e.getId(), e.getType())</tt>) will re-set these values and lose the extra information. */
    class DirectoryEntry {
     public:
        /** Default constructor. */
        DirectoryEntry();

        /** Construct from data.
            \param name Name of this directory entry
            \param id Object Id of pointed-to object
            \param type Type of pointed-to object */
        DirectoryEntry(const String_t& name, const ObjectId& id, DirectoryHandler::Type type);

        /** Parse a tree object.
            Parses a single directory entry.
            \param in [in/out] Tree object. This function consumes one tree object and updates \c in to leave the remainder to be read.
            \retval true Directory entry successfully parsed; this object and \c in have been updated
            \retval false Directory entry could not be read because of a syntax error or the end of the directory has been reached;
                          this object and \c in have been left in indeterminate state */
        bool parse(afl::base::ConstBytes_t& in);

        /** Store into tree object.
            Appends this directory entry to \c out.
            \param out Buffer */
        void store(afl::base::GrowableMemory<uint8_t>& out) const;

        /** Get type of pointed-to object.
            \return type */
        DirectoryHandler::Type getType() const;

        /** Get name.
            \return name */
        const String_t& getName() const;

        /** Get object Id.
            \return object Id */
        const ObjectId& getId() const;

        /** Check whether this entry is sorted before another.
            \param other Other entry
            \return true if this entry is sorted before \c other */
        bool isBefore(const DirectoryEntry& other) const;

     private:
        uint32_t m_mode;
        String_t m_name;
        ObjectId m_id;
    };

} } }

#endif
