/**
  *  \file server/file/ca/commit.hpp
  *  \brief Class server::file::ca::Commit
  */
#ifndef C2NG_SERVER_FILE_CA_COMMIT_HPP
#define C2NG_SERVER_FILE_CA_COMMIT_HPP

#include "server/file/ca/objectid.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/growablememory.hpp"

namespace server { namespace file { namespace ca {

    /** Commit object representation.
        We implement commit objects to make our on-disk format compatible to git.
        This class parses and formats commit objects.

        As of 20170311, this only implements the absolute minimum.
        It does not implement full commit object parsing and is not round-trip compatible. */
    class Commit {
     public:
        /** Default constructor. */
        Commit();

        /** Construct from tree Id.
            \param treeId Id of tree this commit points to. */
        explicit Commit(const ObjectId& treeId);

        /** Parse a commit object.
            \param in Commit object body
            \retval true Successfully parsed; this object has been updated
            \retval false Object cannot be parsed; this object has been left in indeterminate state */
        bool parse(afl::base::ConstBytes_t in);

        /** Store into commit object.
            \param out Commit object body. Pass in empty buffer. On output, populated buffer. */
        void store(afl::base::GrowableMemory<uint8_t>& out) const;

        /** Get tree Id.
            \return ObjectId of the TreeObject this commit points to. */
        const ObjectId& getTreeId() const;

     private:
        ObjectId m_treeId;
    };

} } }

#endif
