/**
  *  \file interpreter/lockaccess.hpp
  *  \brief Interface interpreter::LockAccess
  */
#ifndef C2NG_INTERPRETER_LOCKACCESS_HPP
#define C2NG_INTERPRETER_LOCKACCESS_HPP

#include "afl/string/string.hpp"

namespace interpreter {

    /** Interface for accessing a game's locks.
        This interface reduces coupling between game/map/ and interpreter/ a little.
        It is implemented by MutexList. */
    class LockAccess {
     public:
        /** Virtual destructor. */
        virtual ~LockAccess()
            { }

        /** Check presence of a lock.
            @param name Name, in upper-case
            @return true if a lock with that name exists */
        virtual bool hasLock(const String_t& name) const = 0;
    };
}

#endif
