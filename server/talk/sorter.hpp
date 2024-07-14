/**
  *  \file server/talk/sorter.hpp
  *  \brief Interface server::talk::Sorter
  */
#ifndef C2NG_SERVER_TALK_SORTER_HPP
#define C2NG_SERVER_TALK_SORTER_HPP

#include "afl/base/deletable.hpp"
#include "afl/net/redis/sortoperation.hpp"

namespace server { namespace talk {

    /** Interface for generating sorted output. */
    class Sorter : public afl::base::Deletable {
     public:
        /** Apply sort key to a SortOperation.
            @param op       [in,out]  Prepared sort operation
            @param keyName  [in]      User-provided sort-key in upper case */
        virtual void applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const = 0;
    };

} }

#endif
