/**
  *  \file util/requestdispatcher.hpp
  *  \brief Base class util::RequestDispatcher
  */
#ifndef C2NG_UTIL_REQUESTDISPATCHER_HPP
#define C2NG_UTIL_REQUESTDISPATCHER_HPP

#include "afl/base/runnable.hpp"

namespace util {

    /** Request dispatcher.
        This is an interface for something that consumes (and executes) Runnables.
        It is used with RequestReceiver/RequestSender. */
    class RequestDispatcher {
     public:
        virtual ~RequestDispatcher()
            { }

        /** Post new Runnable.
            The runnable will eventually be executed in the target thread (p->run()).
            Runnable invocations will not be nested.
            Runnables will be invoked in order: if a, b, and c are posted, they are executed in order a, b, c.

            \param p newly-allocated Runnable. RequestDispatcher takes ownership. Must not be null. */
        virtual void postNewRunnable(afl::base::Runnable* p) = 0;
    };

}

#endif
