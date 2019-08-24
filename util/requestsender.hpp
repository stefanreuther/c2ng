/**
  *  \file util/requestsender.hpp
  *  \brief Template class util::RequestSender
  */
#ifndef C2NG_UTIL_REQUESTSENDER_HPP
#define C2NG_UTIL_REQUESTSENDER_HPP

#include <memory>
#include "util/request.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/base/deletable.hpp"
#include "afl/base/closure.hpp"
#include "afl/base/clonableref.hpp"

namespace util {

    /** Request sender.
        These objects are normally constructed from a RequestReceiver and can possibly out-live it.
        A RequestSender can be copied as needed.

        Any thread can post a request using postNewRequest().
        The requests will be processed by the origin thread's RequestDispatcher
        (or not at all if the RequestReceiver has already died).

        Alternatively, a RequestSender can be constructed from a custom implementation. */
    template<typename ObjectType>
    class RequestSender {
     public:
        /** Convenience typedef for associated request type. */
        typedef Request<ObjectType> Request_t;

        /** Base class for implementations. */
        class Impl : public afl::base::RefCounted, public afl::base::Deletable {
         public:
            virtual void postNewRequest(Request_t* req) = 0;
        };

        /** Null implementation. */
        class NullImpl : public Impl {
         public:
            virtual void postNewRequest(Request_t* req)
                { delete req; }
        };


        /** Default constructor.
            Makes a null sender that discards all requests. */
        RequestSender()
            : m_pImpl(*new NullImpl())
            { }

        /** Construct from implementation.
            \param pImpl Implementation */
        RequestSender(afl::base::Ref<Impl> pImpl)
            : m_pImpl(pImpl)
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
        void postNewRequest(Request_t* p)
            { m_pImpl->postNewRequest(p); }

        /** Assignment operator.
            \param other Other RequestSender */
        RequestSender& operator=(const RequestSender& other)
            {
                m_pImpl.reset(*other.m_pImpl);
                return *this;
            }

        /** Convert to another type.
            If ObjectType is an aggregation of multiple objects,
            it makes sense to send a request to a member object.
            Given a closure that extracts a reference to the member object,
            this function converts a RequestSender that sends to the aggregate
            into one that sends to the member.

            \tparam OtherType Member object type
            \param p Newly-allocated closure
            \return New RequestSender */
        template<typename OtherType>
        RequestSender<OtherType> convert(afl::base::Closure<OtherType& (ObjectType&)>* p)
            {
                // Remember the closure
                typedef afl::base::Closure<OtherType& (ObjectType&)> Closure_t;
                typedef afl::base::ClonableRef<Closure_t> ClosureRef_t;
                ClosureRef_t pp(p);

                // Request adaptor
                typedef Request<OtherType> OtherRequest_t;
                class RequestAdaptor : public Request<ObjectType> {
                 public:
                    RequestAdaptor(std::auto_ptr<OtherRequest_t> req, ClosureRef_t& c)
                        : m_request(req),
                          m_closure(c)
                        { }
                    void handle(ObjectType& obj)
                        { m_request->handle(m_closure->call(obj)); }
                 private:
                    std::auto_ptr<OtherRequest_t> m_request;
                    ClosureRef_t m_closure;
                };

                // Sender implementation
                class SenderAdaptor : public RequestSender<OtherType>::Impl {
                 public:
                    SenderAdaptor(afl::base::Ref<Impl> impl, ClosureRef_t& c)
                        : m_impl(impl),
                          m_closure(c)
                        { }
                    virtual void postNewRequest(OtherRequest_t* p)
                        {
                            std::auto_ptr<OtherRequest_t> pp(p);
                            m_impl->postNewRequest(new RequestAdaptor(pp, m_closure));
                        }
                 private:
                    afl::base::Ref<Impl> m_impl;
                    ClosureRef_t m_closure;
                };
                return RequestSender<OtherType>(*new SenderAdaptor(m_pImpl, pp));
            }

     private:
        afl::base::Ref<Impl> m_pImpl;
    };

}

#endif
