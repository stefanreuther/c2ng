/**
  *  \file util/request.hpp
  *  \brief Base class util::Request
  */
#ifndef C2NG_UTIL_REQUEST_HPP
#define C2NG_UTIL_REQUEST_HPP

namespace util {

    /** Typed request.
        This is a base class used with RequestReceiver/RequestSender.
        It encapsulates an action that operates on a specified variable.

        \tparam ObjectType Variable type */
    template<typename ObjectType>
    class Request {
     public:
        virtual ~Request()
            { }

        /** Operation.
            \param v Value */
        virtual void handle(ObjectType& v) = 0;
    };

}

#endif
