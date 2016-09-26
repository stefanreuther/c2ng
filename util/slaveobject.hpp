/**
  *  \file util/slaveobject.hpp
  *  \brief Template interface util::SlaveObject
  */
#ifndef C2NG_UTIL_SLAVEOBJECT_HPP
#define C2NG_UTIL_SLAVEOBJECT_HPP

#include "afl/base/deletable.hpp"

namespace util {

    /** Slave object.
        These are used with SlaveRequestSender / BaseSlaveRequestSender, see there.

        A slave object's lifetime is:
        - construct it in a user thread. This constructor does not have access to the master object.
        - init() being called in the master object's thread, so it could be treated as a post-constructor.
        - SlaveRequest's being called in the master object's thread.
        - done() being called in the master object's thread, so it could be treated as a pre-destructor.
        - destructor being called in the master object's thread.

        If the master object dies prematurely, done() will not be called.
        If there is no master object, neither of this object's methods will be called, and the destructor will be called in the original thread.

        \tparam ObjectType type of the controlling object (same as ObjectType of the RequestSender used to build the SlaveRequestSender). */
    template<typename ObjectType>
    class SlaveObject : public afl::base::Deletable {
     public:
        /** Initialize.
            This is the first method called on the SlaveObject.
            \param master Master object */
        virtual void init(ObjectType& master) = 0;

        /** Shutdown.
            This is the last method called on the SlaveObject.
            \param master Master object */
        virtual void done(ObjectType& master) = 0;
    };

}

#endif
