/**
  *  \file util/baseslaverequest.hpp
  *  \brief Template interface util::BaseSlaveRequest
  */
#ifndef C2NG_UTIL_BASESLAVEREQUEST_HPP
#define C2NG_UTIL_BASESLAVEREQUEST_HPP

#include "afl/base/deletable.hpp"

namespace util {

    template<typename ObjectType> class SlaveObject;

    /** Untyped slave request.
        This class is used with BaseSlaveRequestSender.

        \tparam ObjectType Master object type */
    template<typename ObjectType>
    class BaseSlaveRequest : public afl::base::Deletable {
     public:
        /** Perform operation.
            This is the operation executed in the master object's thread, with master and slave alive.
            \param master Master object
            \param slave Slave object */
        virtual void handle(ObjectType& master, SlaveObject<ObjectType>& slave) = 0;
    };

}

#endif
