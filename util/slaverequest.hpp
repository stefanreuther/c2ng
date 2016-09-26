/**
  *  \file util/slaverequest.hpp
  *  \brief Template interface SlaveRequest
  */
#ifndef C2NG_UTIL_SLAVEREQUEST_HPP
#define C2NG_UTIL_SLAVEREQUEST_HPP

#include "afl/base/deletable.hpp"

namespace util {

    /** Typed slave request.
        This class is used with SlaveRequestSender.

        \tparam ObjectType Master object type
        \param SlaveType Slave object type */
    template<typename ObjectType, typename SlaveType>
    class SlaveRequest : public afl::base::Deletable {
     public:
        /** Perform operation.
            This is the operation executed in the master object's thread, with master and slave alive.
            \param master Master object
            \param slave Slave object */
        virtual void handle(ObjectType& master, SlaveType& slave) = 0;
    };

}

#endif
