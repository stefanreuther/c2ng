/**
  *  \file server/common/util.hpp
  *  \brief Common Server Utilities
  */
#ifndef C2NG_SERVER_COMMON_UTIL_HPP
#define C2NG_SERVER_COMMON_UTIL_HPP

#include "afl/string/string.hpp"

namespace server { namespace common {

    /** Simplify a user name.
        @param s     Name as entered by user
        @return Simplified name: lowercased, runs of invalid characters replaced by '_' */
    String_t simplifyUserName(const String_t s);

} }

#endif
