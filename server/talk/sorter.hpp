/**
  *  \file server/talk/sorter.hpp
  */
#ifndef C2NG_SERVER_TALK_SORTER_HPP
#define C2NG_SERVER_TALK_SORTER_HPP

#include "afl/base/deletable.hpp"
#include "afl/net/redis/sortoperation.hpp"

namespace server { namespace talk {

    class Sorter : public afl::base::Deletable {
     public:
        virtual void applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const = 0;
    };

} }

#endif
