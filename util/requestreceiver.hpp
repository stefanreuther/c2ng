/**
  *  \file util/requestreceiver.hpp
  *  \brief Template class util::RequestReceiver
  */
#ifndef C2NG_UTIL_REQUESTRECEIVER_HPP
#define C2NG_UTIL_REQUESTRECEIVER_HPP

#include "afl/base/ptr.hpp"
#include "util/requestsender.hpp"
#include "afl/base/uncopyable.hpp"

namespace util {

    class RequestDispatcher;

    /** Request receiver.
        An object of type RequestReceiver<T> allows a thread to receive Request<T>.

        Operation:
        - implement a long-lived RequestDispatcher that processes Runnables in this thread
        - make a RequestReceiver
        - pass the RequestReceiver's RequestSender (getSender()) around

        Other threads can use the RequestSender to submit Request<T>'s.
        These will be processed in this thread.
        If the RequestReceiver has died, future requests will be silently ignored.

        \todo can we drop the long-lived RequestDispatcher requirment?
        This would need a Mutex in RequestSender; what else?

        \tparam ObjectType object type */
    template<typename ObjectType>
    class RequestReceiver : private afl::base::Uncopyable {
     public:
        /** Constructor.
            \param dispatcher Request dispatcher. Must live longer than the latest request.
            \param obj Object */
        RequestReceiver(RequestDispatcher& dispatcher, ObjectType& obj);

        /** Destructor. */
        ~RequestReceiver();

        /** Get sender.
            \return sender to use in other threads */
        RequestSender<ObjectType> getSender();

        /** Get object.
            \return object */
        ObjectType& object();

     private:
        RequestDispatcher& m_dispatcher;
        ObjectType& m_obj;
        RequestSender<ObjectType> m_sender;
    };

}



template<typename ObjectType>
util::RequestReceiver<ObjectType>::RequestReceiver(RequestDispatcher& dispatcher, ObjectType& obj)
    : m_dispatcher(dispatcher),
      m_obj(obj),
      m_sender(*this, m_dispatcher)
{ }

template<typename ObjectType>
inline
util::RequestReceiver<ObjectType>::~RequestReceiver()
{
    m_sender.disconnect();
}

template<typename ObjectType>
inline util::RequestSender<ObjectType>
util::RequestReceiver<ObjectType>::getSender()
{
    return m_sender;
}

template<typename ObjectType>
inline ObjectType&
util::RequestReceiver<ObjectType>::object()
{
    return m_obj;
}

#endif
