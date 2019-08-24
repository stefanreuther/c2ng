/**
  *  \file util/requestreceiver.hpp
  *  \brief Template class util::RequestReceiver
  */
#ifndef C2NG_UTIL_REQUESTRECEIVER_HPP
#define C2NG_UTIL_REQUESTRECEIVER_HPP

#include <memory>
#include "afl/base/runnable.hpp"
#include "afl/base/uncopyable.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"

namespace util {

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
        typedef Request<ObjectType> Request_t;

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
        class SenderImpl;

        RequestDispatcher& m_dispatcher;
        ObjectType& m_obj;
        afl::base::Ref<SenderImpl> m_sender;
    };

}


/*
 *  Implemenation of Sender
 */

template<typename ObjectType>
class util::RequestReceiver<ObjectType>::SenderImpl : public RequestSender<ObjectType>::Impl {
 public:
    SenderImpl(RequestReceiver& parent, RequestDispatcher& dispatcher)
        : m_pBacklink(&parent),
          m_dispatcher(dispatcher)
        { }
    virtual void postNewRequest(Request_t* req)
        {
            // Request-to-Runnable adapter
            class Processor : public afl::base::Runnable {
             public:
                Processor(afl::base::Ref<SenderImpl> impl, std::auto_ptr<Request_t> req)
                    : m_impl(impl),
                      m_request(req)
                    { }
                virtual void run()
                    {
                        if (RequestReceiver* p = m_impl->m_pBacklink) {
                            m_request->handle(p->object());
                        }
                    }

             private:
                afl::base::Ref<SenderImpl> m_impl;
                std::auto_ptr<Request_t> m_request;
            };

            // Post it
            std::auto_ptr<Request_t> pp(req);
            m_dispatcher.postNewRunnable(new Processor(*this, pp));
        }
    void disconnect()
        { m_pBacklink = 0; }
 private:
    RequestReceiver* m_pBacklink;
    RequestDispatcher& m_dispatcher;
};



template<typename ObjectType>
util::RequestReceiver<ObjectType>::RequestReceiver(RequestDispatcher& dispatcher, ObjectType& obj)
    : m_dispatcher(dispatcher),
      m_obj(obj),
      m_sender(*new SenderImpl(*this, m_dispatcher))
{ }

template<typename ObjectType>
inline
util::RequestReceiver<ObjectType>::~RequestReceiver()
{
    m_sender->disconnect();
}

template<typename ObjectType>
inline util::RequestSender<ObjectType>
util::RequestReceiver<ObjectType>::getSender()
{
    return RequestSender<ObjectType>(m_sender);
}

template<typename ObjectType>
inline ObjectType&
util::RequestReceiver<ObjectType>::object()
{
    return m_obj;
}

#endif
