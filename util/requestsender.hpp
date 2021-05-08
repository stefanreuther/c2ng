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

        /** Post request to nullary function.
            Calls obj.fcn() on the object addressed by this RequestSender.
            \param fcn Function to call */
        void postRequest(void (ObjectType::*fcn)());

        /** Post request to unary function.
            Calls obj.fcn(p1) on the object addressed by this RequestSender.
            \param fcn Function to call
            \param p1 Parameter
            \tparam PT1 Parameter type (must not be a reference type) */
        template<typename PT1>
        void postRequest(void (ObjectType::*fcn)(PT1), PT1 p1);

        /** Post request to binary function.
            Calls obj.fcn(p1,p2) on the object addressed by this RequestSender.
            \param fcn Function to call
            \param p1 First parameter
            \param p2 Second parameter
            \tparam PT1 First parameter type (must not be a reference type)
            \tparam PT2 Second parameter type (must not be a reference type) */
        template<typename PT1, typename PT2>
        void postRequest(void (ObjectType::*fcn)(PT1, PT2), PT1 p1, PT2 p2);

        /** Post request to ternary function.
            Calls obj.fcn(p1,p2,p3) on the object addressed by this RequestSender.
            \param fcn Function to call
            \param p1 First parameter
            \param p2 Second parameter
            \param p3 Third parameter
            \tparam PT1 First parameter type (must not be a reference type)
            \tparam PT2 Second parameter type (must not be a reference type)
            \tparam PT3 Third parameter type (must not be a reference type) */
        template<typename PT1, typename PT2, typename PT3>
        void postRequest(void (ObjectType::*fcn)(PT1, PT2, PT3), PT1 p1, PT2 p2, PT3 p3);

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

            The extraction (invocation of the closure) is performed anew every time a request is sent.

            \tparam OtherType Member object type
            \param p Newly-allocated closure; returns reference to existing sub-object of ObjectType
            \return New RequestSender */
        template<typename OtherType>
        RequestSender<OtherType> convert(afl::base::Closure<OtherType& (ObjectType&)>* p)
            {
                // Remember the closure
                typedef afl::base::Closure<OtherType& (ObjectType&)> Closure_t;
                typedef std::auto_ptr<Closure_t> ClosureRef_t;
                ClosureRef_t pp(p);

                // Request adaptor
                typedef Request<OtherType> OtherRequest_t;
                class RequestAdaptor : public Request<ObjectType> {
                 public:
                    RequestAdaptor(std::auto_ptr<OtherRequest_t> req, ClosureRef_t& c)
                        : m_request(req),
                          m_closure(c.get())
                        { }
                    void handle(ObjectType& obj)
                        { m_request->handle(m_closure->call(obj)); }
                 private:
                    std::auto_ptr<OtherRequest_t> m_request;
                    Closure_t* m_closure;
                };

                class ShutdownRequest : public Request<ObjectType> {
                 public:
                    ShutdownRequest(ClosureRef_t& c)
                        : m_closure(c)
                        { }
                    void handle(ObjectType&)
                        { m_closure.reset(); }
                 private:
                    ClosureRef_t m_closure;
                };

                // Sender implementation
                class SenderAdaptor : public RequestSender<OtherType>::Impl {
                 public:
                    SenderAdaptor(afl::base::Ref<Impl> impl, ClosureRef_t& c)
                        : m_impl(impl),
                          m_closure(c)
                        { }
                    ~SenderAdaptor()
                        { m_impl->postNewRequest(new ShutdownRequest(m_closure)); }
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

        /** Create temporary object of another type.

            Invokes a closure on the ObjectType object to create a new object of type OtherType.
            You can then send requests to that temporary object.
            The object lives as long as there are references to this RequestSender.
            If the RequestSender closes, the temporary is destroyed (by means of a ObjectType request).

            \tparam OtherType Member object type
            \param p Newly-allocated closure
            \return New RequestSender */
        template<typename OtherType>
        RequestSender<OtherType> makeTemporary(afl::base::Closure<OtherType* (ObjectType&)>* p);

     private:
        afl::base::Ref<Impl> m_pImpl;

        // Trampoline for makeTemporary.
        // - Created in creator thread to have some common workspace
        // - Initialized using InitializeRequest in RequestDispatcher thread; this creates pImpl using the user's closure
        // - Requests processed using RequestAdaptor
        // - Cleaned up normalls using ShutdownRequest in RequestDispatcher thread
        template<typename OtherType>
        struct Trampoline {
            std::auto_ptr<OtherType> pImpl;
        };
    };

}


template<typename ObjectType>
void
util::RequestSender<ObjectType>::postRequest(void (ObjectType::*fcn)())
{
    class Task : public Request_t {
     public:
        Task(void (ObjectType::*fcn)())
            : m_fcn(fcn)
            { }
        virtual void handle(ObjectType& obj)
            { (obj.*m_fcn)(); }
     private:
        void (ObjectType::*m_fcn)();
    };
    this->postNewRequest(new Task(fcn));
}

template<typename ObjectType>
template<typename PT1>
void
util::RequestSender<ObjectType>::postRequest(void (ObjectType::*fcn)(PT1), PT1 p1)
{
    class Task : public Request_t {
     public:
        Task(void (ObjectType::*fcn)(PT1), const PT1& p1)
            : m_fcn(fcn), m_p1(p1)
            { }
        virtual void handle(ObjectType& obj)
            { (obj.*m_fcn)(m_p1); }
     private:
        void (ObjectType::*m_fcn)(PT1);
        PT1 m_p1;
    };
    this->postNewRequest(new Task(fcn, p1));
}

template<typename ObjectType>
template<typename PT1, typename PT2>
void
util::RequestSender<ObjectType>::postRequest(void (ObjectType::*fcn)(PT1, PT2), PT1 p1, PT2 p2)
{
    class Task : public Request_t {
     public:
        Task(void (ObjectType::*fcn)(PT1, PT2), const PT1& p1, const PT2& p2)
            : m_fcn(fcn), m_p1(p1), m_p2(p2)
            { }
        virtual void handle(ObjectType& obj)
            { (obj.*m_fcn)(m_p1, m_p2); }
     private:
        void (ObjectType::*m_fcn)(PT1, PT2);
        PT1 m_p1;
        PT2 m_p2;
    };
    this->postNewRequest(new Task(fcn, p1, p2));
}

template<typename ObjectType>
template<typename PT1, typename PT2, typename PT3>
void
util::RequestSender<ObjectType>::postRequest(void (ObjectType::*fcn)(PT1, PT2, PT3), PT1 p1, PT2 p2, PT3 p3)
{
    class Task : public Request_t {
     public:
        Task(void (ObjectType::*fcn)(PT1, PT2, PT3), const PT1& p1, const PT2& p2, const PT3& p3)
            : m_fcn(fcn), m_p1(p1), m_p2(p2), m_p3(p3)
            { }
        virtual void handle(ObjectType& obj)
            { (obj.*m_fcn)(m_p1, m_p2, m_p3); }
     private:
        void (ObjectType::*m_fcn)(PT1, PT2, PT3);
        PT1 m_p1;
        PT2 m_p2;
        PT3 m_p3;
    };
    this->postNewRequest(new Task(fcn, p1, p2, p3));
}

template<typename ObjectType>
template<typename OtherType>
util::RequestSender<OtherType>
util::RequestSender<ObjectType>::makeTemporary(afl::base::Closure<OtherType* (ObjectType&)>* p)
{
    // Remember the closure
    typedef afl::base::Closure<OtherType* (ObjectType&)> Closure_t;
    typedef std::auto_ptr<Closure_t> ClosureRef_t;
    ClosureRef_t pp(p);

    // Initialisation request
    typedef Trampoline<OtherType> Trampoline_t;
    class InitializeRequest : public util::Request<ObjectType> {
     public:
        InitializeRequest(Trampoline_t& tpl, ClosureRef_t& c)
            : m_trampoline(tpl),
              m_closure(c)
            { }
        virtual void handle(ObjectType& obj)
            { m_trampoline.pImpl.reset(m_closure->call(obj)); }
     private:
        Trampoline_t& m_trampoline;
        ClosureRef_t m_closure;
    };

    // Shutdown request
    class ShutdownRequest : public util::Request<ObjectType> {
     public:
        ShutdownRequest(Trampoline_t* tpl)
            : m_trampoline(tpl)
            { }
        virtual void handle(ObjectType&)
            { m_trampoline->pImpl.reset(); }
     private:
        std::auto_ptr<Trampoline_t> m_trampoline;
    };

    // Adaptor to convert OtherType requests into ObjectType requests
    typedef Request<OtherType> OtherRequest_t;
    class RequestAdaptor : public Request<ObjectType> {
     public:
        RequestAdaptor(std::auto_ptr<OtherRequest_t>& req, Trampoline_t& tpl)
            : m_request(req),
              m_trampoline(tpl)
            { }
        void handle(ObjectType&)
            {
                if (m_trampoline.pImpl.get() != 0) {
                    m_request->handle(*m_trampoline.pImpl);
                }
            }
     private:
        std::auto_ptr<OtherRequest_t> m_request;
        Trampoline_t& m_trampoline;
    };

    // Sender implementation
    // Trampoline is born in this thread; first thing we do is send an InitializeRequest,
    // so requests sent after can assume it has run.
    // Last thing we do is send a ShutdownRequest which deletes the trampoline.
    // We therefore need no reference-counting here, which saves significant amounts of code.
    class SenderAdaptor : public RequestSender<OtherType>::Impl {
     public:
        SenderAdaptor(const afl::base::Ref<Impl>& impl, ClosureRef_t& c)
            : m_impl(impl),
              m_trampoline(new Trampoline_t())
            { m_impl->postNewRequest(new InitializeRequest(*m_trampoline, c)); }
        ~SenderAdaptor()
            { m_impl->postNewRequest(new ShutdownRequest(m_trampoline)); }
        virtual void postNewRequest(OtherRequest_t* p)
            {
                std::auto_ptr<OtherRequest_t> pp(p);
                m_impl->postNewRequest(new RequestAdaptor(pp, *m_trampoline));
            }
     private:
        afl::base::Ref<Impl> m_impl;
        Trampoline_t* m_trampoline;
    };
    return RequestSender<OtherType>(*new SenderAdaptor(m_pImpl, pp));
}

#endif
