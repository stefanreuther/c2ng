/**
  *  \file server/host/spec/publisher.hpp
  *  \brief Interface server::host::spec::Publisher
  */
#ifndef C2NG_SERVER_HOST_SPEC_PUBLISHER_HPP
#define C2NG_SERVER_HOST_SPEC_PUBLISHER_HPP

#include "afl/base/deletable.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/stringlist.hpp"

namespace server { namespace host { namespace spec {

    /** Specification publisher.
        Provides functionality to access a set of ship specifications.
        This is a virtual interface to simplify testing. */
    class Publisher : public afl::base::Deletable {
     public:
        /** Get specification data for a directory.
            @param pathName    Path name in host filer;
            @param flakPath    Path for FLAK tool in host filer (can be empty);
            @param keys        Specification keys to retrieve; passed directly from SPECSHIPLIST/SPECGAME command.
            @return Result */
        virtual afl::data::Hash::Ref_t getSpecificationData(String_t pathName, String_t flakPath, const afl::data::StringList_t& keys) = 0;
    };

} } }

#endif
