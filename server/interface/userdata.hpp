/**
  *  \file server/interface/userdata.hpp
  *  \brief Interface server::interface::UserData
  */
#ifndef C2NG_SERVER_INTERFACE_USERDATA_HPP
#define C2NG_SERVER_INTERFACE_USERDATA_HPP

#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"

namespace server { namespace interface {

    /** User Data interface.
        This interface allows to store invisible auxiliary data, e.g. application states associated with a user. */
    class UserData : public afl::base::Deletable {
     public:
        /** Set value.
            \param userId User Id
            \param key    Key
            \param value  Value */
        virtual void set(String_t userId, String_t key, String_t value) = 0;

        /** Get value.
            \param userId User Id
            \param key    Key
            \return Value. Empty string if key not set. */
        virtual String_t get(String_t userId, String_t key) = 0;
    };

} }

#endif
