/**
  *  \file server/interface/base.hpp
  *  \brief Interface server::interface::Base
  */
#ifndef C2NG_SERVER_INTERFACE_BASE_HPP
#define C2NG_SERVER_INTERFACE_BASE_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    /** Base operations.
        These operations are available on every RESP microservice. */
    class Base : public afl::base::Deletable {
     public:
        /** Ping it.
            Contacts the server and returns the response.
            Under normal circumstances, the answer is "PONG".
            \return Answer */
        virtual String_t ping() = 0;

        /** Set user context.
            Configures the user context for subsequent calls.
            \param user User Id; empty string for admin context. */
        virtual void setUserContext(String_t user) = 0;
    };

} }

#endif
