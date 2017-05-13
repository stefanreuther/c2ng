/**
  *  \file server/file/ca/referenceupdater.hpp
  */
#ifndef C2NG_SERVER_FILE_CA_REFERENCEUPDATER_HPP
#define C2NG_SERVER_FILE_CA_REFERENCEUPDATER_HPP

#include "afl/base/refcounted.hpp"
#include "afl/string/string.hpp"
#include "server/file/ca/objectid.hpp"

namespace server { namespace file { namespace ca {

    class ReferenceUpdater : public afl::base::RefCounted, public afl::base::Deletable {
     public:
        virtual void updateDirectoryReference(const String_t& name, const ObjectId& newId) = 0;
    };

} } }

#endif
