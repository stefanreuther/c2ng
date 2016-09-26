/**
  *  \file util/baseslaverequestsender.hpp
  *  \brief Template class util::BaseSlaveRequestSender
  */
#ifndef C2NG_UTIL_BASESLAVEREQUESTSENDER_HPP
#define C2NG_UTIL_BASESLAVEREQUESTSENDER_HPP

#include <memory>
#include "afl/base/uncopyable.hpp"
#include "util/slaveobject.hpp"
#include "util/baseslaverequest.hpp"
#include "util/requestsender.hpp"
#include "util/request.hpp"
#include "afl/base/ptr.hpp"

namespace util {

    /** Basic slave request sender.
        This is the base class for SlaveRequestSender; see there. */
    template<typename ObjectType>
    class BaseSlaveRequestSender : public afl::base::Uncopyable {
     public:
        /** Convenience typedef for slave object. */
        typedef SlaveObject<ObjectType> SlaveObject_t;

        /** Convenience typedef for slave request. */
        typedef BaseSlaveRequest<ObjectType> SlaveRequest_t;

        /** Constructor.
            Makes a BaseSlaveRequestSender that executes BaseSlaveRequest<ObjectType>.
            Constructing the BaseSlaveRequestSender will eventually cause SlaveObject::init() to be executed
            before the first request is handled.

            \param sender Master object sender
            \param p Slave object. Must be newly-allocated; should not be null. BaseSlaveRequestSender takes ownership.

            If the master object sender is not connected, all requests will be ignored
            and the slave object will eventually be destroyed in the thread owning the BaseSlaveRequestSender
            (as opposed to when it is connected, in which case it will be destroyed in the master object's thread).

            If the slave object is null, all requests will be ignored. */
        BaseSlaveRequestSender(RequestSender<ObjectType> sender, SlaveObject_t* p);

        /** Destructor.
            Destructing the BaseSlaveRequestSender will eventually cause SlaveObject::done() to be executed
            after the last request is handled.

            To emphasize, the destructor only schedules the SlaveObject for deletion, it does not immediately delete it. */
        ~BaseSlaveRequestSender();

        /** Post new request.
            Can be executed from any thread.

            The request will be processed by the master object's thread's RequestDispatcher
            (or not at all if the RequestReceiver has already died).

            \param p Newly-allocated request. Ownership will be transferred to the BaseSlaveRequestSender.

            The request will be destroyed
            - in the target thread, after executing it
            - in the target thread, without executing it, if the target master object has died
            - in the origin thread, without executing it, if there is no master object */
        void postNewRequest(SlaveRequest_t* p);

     private:
        class InitTask;
        class DoneTask;
        class ProxyTask;

        RequestSender<ObjectType> m_sender;
        afl::base::Ptr<SlaveObject_t> m_p;
    };

}

/*
 *  InitTask
 *
 *  Initializes the object.
 *  If the BaseSlaveRequestSender already died again, this will keep the slave object alive using a Ptr<>.
 */
template<typename ObjectType>
class util::BaseSlaveRequestSender<ObjectType>::InitTask : public Request<ObjectType> {
 public:
    InitTask(afl::base::Ptr<SlaveObject_t> p)
        : m_p(p)
        { }
    void handle(ObjectType& t)
        {
            if (m_p.get() != 0) {
                m_p->init(t);
            }
        }
 private:
    afl::base::Ptr<SlaveObject_t> m_p;
};

/*
 *  DoneTask
 *
 *  Shuts down the object.
 *  We need the done() method as we cannot give the destructor a master object reference.
 *  Destruction of the object is ultimately triggered by the DoneTask being the last one holding a reference to the slave object.
 */
template<typename ObjectType>
class util::BaseSlaveRequestSender<ObjectType>::DoneTask : public Request<ObjectType> {
 public:
    DoneTask(afl::base::Ptr<SlaveObject_t> p)
        : m_p(p)
        { }
    void handle(ObjectType& t)
        {
            if (m_p.get() != 0) {
                m_p->done(t);
            }
        }
 private:
    afl::base::Ptr<SlaveObject_t> m_p;
};

/*
 *  ProxyTask
 *
 *  Call an arbitrary BaseSlaveRequest on the master/slave objects.
 */
template<typename ObjectType>
class util::BaseSlaveRequestSender<ObjectType>::ProxyTask : public Request<ObjectType> {
 public:
    ProxyTask(afl::base::Ptr<SlaveObject_t> p, std::auto_ptr<SlaveRequest_t> req)
        : m_p(p),
          m_req(req)
        { }
    void handle(ObjectType& t)
        {
            if (m_req.get() != 0 && m_p.get() != 0) {
                m_req->handle(t, *m_p);
            }
        }
 private:
    afl::base::Ptr<SlaveObject_t> m_p;
    std::auto_ptr<SlaveRequest_t> m_req;
};


// Constructor.
template<typename ObjectType>
util::BaseSlaveRequestSender<ObjectType>::BaseSlaveRequestSender(RequestSender<ObjectType> sender, SlaveObject_t* p)
    : m_sender(sender),
      m_p(p)
{
    m_sender.postNewRequest(new InitTask(m_p));
}

// Destructor.
template<typename ObjectType>
util::BaseSlaveRequestSender<ObjectType>::~BaseSlaveRequestSender()
{
    // Make sure the DoneTask is the only one holding a reference to the slave object.
    std::auto_ptr<DoneTask> t(new DoneTask(m_p));
    m_p = 0;

    // Post the DoneTask.
    m_sender.postNewRequest(t.release());
}

// Post new request.
template<typename ObjectType>
void
util::BaseSlaveRequestSender<ObjectType>::postNewRequest(SlaveRequest_t* p)
{
    std::auto_ptr<SlaveRequest_t> ap(p);
    m_sender.postNewRequest(new ProxyTask(m_p, ap));
}

#endif
