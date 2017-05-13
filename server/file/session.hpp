/**
  *  \file server/file/session.hpp
  *  \brief Class server::file::Session
  */
#ifndef C2NG_SERVER_FILE_SESSION_HPP
#define C2NG_SERVER_FILE_SESSION_HPP

#include "afl/string/string.hpp"
#include "server/common/session.hpp"

namespace server { namespace file {

    /** Server session state.
        Currently, the file server does not need more session state than the common session. */
    class Session : public server::common::Session {
    };

} }

#endif
