/**
  *  \file util/requestsender.hpp
  *  \brief Template class util::RequestSender
  */
#ifndef C2NG_UTIL_REQUESTSENDER_HPP
#define C2NG_UTIL_REQUESTSENDER_HPP

#include <memory>
#include "util/request.hpp"
#include "util/requestdispatcher.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/base/ptr.hpp"

namespace util {

    template<typename ObjectType> class RequestReceiver;

    /** Request sender.
        These objects are constructed from a RequestReceiver and can possibly out-live it.
        A RequestSender can be copied as needed.

        Any thread can post a request using postNewRequest().
        The requests will be processed by the origin thread's RequestDispatcher
        (or not at all if the RequestReceiver has already died). */
    template<typename ObjectType>
    class RequestSender {
     public:
        /** Convenience typedef for associated request type. */
        typedef Request<ObjectType> Request_t;

        /** Default constructor.
            Makes a null sender that discards all requests. */
        RequestSender()
            : m_pImpl()
            { }

        /** Post new request.
            Can be executed from any thread.

            The request will be processed by the origin thread's RequestDispatcher
            (or not at all if the RequestReceiver has already died).

            \param p Newly-allocated request. Ownership will be transferred to the RequestSender.

            The request will be destroyed
            - in the target thread, after executing it
            - in the target thread, without executing it, if the target object has died
            - in the origin thread, without executing it, if there is no target object */
        void postNewRequest(Request_t* p);

     private:
        friend class RequestReceiver<ObjectType>;

        /** Constructor.
            Execute from origin thread (=RequestReceiver).
            \param backlink Originating RequestReceiver
            \param dispatcher Originating thread's RequestDispatcher */
        RequestSender(RequestReceiver<ObjectType>& backlink, RequestDispatcher& dispatcher);

        /** Disconnect originating RequestReceiver.
            Execute from origin thread. */
        void disconnect();

        struct Impl {
            RequestReceiver<ObjectType>* m_pBacklink;
            RequestDispatcher& m_dispatcher;
            Impl(RequestReceiver<ObjectType>* pBacklink, RequestDispatcher& dispatcher)
                : m_pBacklink(pBacklink),
                  m_dispatcher(dispatcher)
                { }
        };
        afl::base::Ptr<Impl> m_pImpl;
    };

}

// Post new request.
template<typename ObjectType>
void
util::RequestSender<ObjectType>::postNewRequest(Request_t* p)
{
    // Request-to-Runnable adapter
    class Processor : public afl::base::Runnable {
     public:
        Processor(afl::base::Ptr<Impl> impl, std::auto_ptr<Request_t> req)
            : m_impl(impl),
              m_request(req)
            { }
        virtual void run()
            {
                if (RequestReceiver<ObjectType>* p = m_impl->m_pBacklink) {
                    m_request->handle(p->object());
                }
            }

     private:
        afl::base::Ptr<Impl> m_impl;
        std::auto_ptr<Request_t> m_request;
    };

    // Post it
    std::auto_ptr<Request_t> pp(p);
    if (m_pImpl.get() != 0) {
        m_pImpl->m_dispatcher.postNewRunnable(new Processor(m_pImpl, pp));
    }
}

// Constructor.
template<typename ObjectType>
inline
util::RequestSender<ObjectType>::RequestSender(RequestReceiver<ObjectType>& backlink, RequestDispatcher& dispatcher)
    : m_pImpl(new Impl(&backlink, dispatcher))
{ }

// Disconnect originating RequestReceiver.
template<typename ObjectType>
inline void
util::RequestSender<ObjectType>::disconnect()
{
    if (m_pImpl.get() != 0) {
        m_pImpl->m_pBacklink = 0;
    }
}

#endif
