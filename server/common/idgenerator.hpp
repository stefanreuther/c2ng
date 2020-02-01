/**
  *  \file server/common/idgenerator.hpp
  *  \brief Interface server::common::IdGenerator
  */
#ifndef C2NG_SERVER_COMMON_IDGENERATOR_HPP
#define C2NG_SERVER_COMMON_IDGENERATOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace server { namespace common {

    /** Algorithm for creating session Ids. */
    class IdGenerator : public afl::base::Deletable {
     public:
        /** Create a session Id.
            A session Id must be a non-empty, case-sensitive, alphanumeric string. */
        virtual String_t createId() = 0;
    };

} }

#endif
