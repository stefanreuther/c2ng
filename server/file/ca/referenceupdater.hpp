/**
  *  \file server/file/ca/referenceupdater.hpp
  *  \brief Interface server::file::ca::ReferenceUpdater
  */
#ifndef C2NG_SERVER_FILE_CA_REFERENCEUPDATER_HPP
#define C2NG_SERVER_FILE_CA_REFERENCEUPDATER_HPP

#include "afl/base/refcounted.hpp"
#include "afl/string/string.hpp"
#include "server/file/ca/objectid.hpp"

namespace server { namespace file { namespace ca {

    /** Directory reference updater.
        If a directory's content changes, its Id changes and must be updated in its parent
        (causing further updates in the process).
        An instance of this interface implements the change by updating the parent directory,
        or the initial reference (refs/heads/) pointing to it. */
    class ReferenceUpdater : public afl::base::RefCounted, public afl::base::Deletable {
     public:
        /** Update reference to a directory.
            @param name   Name (basename)
            @param newId  New ObjectId of the tree object representing the directory content */
        virtual void updateDirectoryReference(const String_t& name, const ObjectId& newId) = 0;
    };

} } }

#endif
