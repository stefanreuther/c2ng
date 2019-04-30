/**
  *  \file server/interface/gameaccess.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_GAMEACCESS_HPP
#define C2NG_SERVER_INTERFACE_GAMEACCESS_HPP

#include "afl/base/deletable.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class GameAccess : public afl::base::Deletable {
     public:
        // SAVE
        virtual void save() = 0;

        // STAT
        virtual String_t getStatus() = 0;

        // GET objName
        virtual Value_t* get(String_t objName) = 0;

        // POST
        virtual Value_t* post(String_t objName, const Value_t* value) = 0;

        // Not wrapped: HELP, QUIT
    };

} }

#endif
