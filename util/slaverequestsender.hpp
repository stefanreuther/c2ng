/**
  *  \file util/slaverequestsender.hpp
  *  \brief Template class SlaveRequestSender
  */
#ifndef C2NG_UTIL_SLAVEREQUESTSENDER_HPP
#define C2NG_UTIL_SLAVEREQUESTSENDER_HPP

#include "util/slaveobject.hpp"
#include "util/baseslaverequest.hpp"
#include "util/baseslaverequestsender.hpp"
#include "util/slaverequest.hpp"

namespace util {

    /** Slave request sender.
        This is a type-safe wrapper for SlaveRequestSender.

        SlaveRequestSender allows creation of temporary objects (slave objects) that operate on a master object receiving requests.
        Given an object that implements RequestReceiver<T>, you obtain a RequestSender<T> to execute operations on T:
        - Request<T>::handle(T&).
        If you need additional information for stateful operations, a SlaveRequestSender<T,U> allows you to execute operations on an object pair:
        - SlaveRequest<T,U>::handle(T&,U&).
        
        The slave object must be a type derived from SlaveObject<T>.
        It is created (constructed) in the same thread that constructs the SlaveRequestSender
        and paired with the master object using SlaveObject<T>::init(T&);
        see there for the lifecycle description.

        \tparam ObjectType Master object type
        \tparam SlaveType Slave object type */
    template<typename ObjectType, typename SlaveType>
    class SlaveRequestSender : public BaseSlaveRequestSender<ObjectType> {
     public:
        typedef SlaveRequest<ObjectType,SlaveType> SlaveRequest_t;

        /** Constructor.
            Makes a SlaveRequestSender that executes SlaveRequest<ObjectType,SlaveTyoe>.
            Constructing the SlaveRequestSender will eventually cause SlaveObject::init() to be executed
            before the first request is handled.

            \param sender Master object sender
            \param p Slave object. Must be newly-allocated; should not be null. SlaveRequestSender takes ownership.

            If the master object sender is not connected, all requests will be ignored
            and the slave object will eventually be destroyed in the thread owning the SlaveRequestSender
            (as opposed to when it is connected, in which case it will be destroyed in the master object's thread).

            If the slave object is null, all requests will be ignored. */
        SlaveRequestSender(RequestSender<ObjectType> sender, SlaveType* p);

        /** Destructor.
            Destructing the SlaveRequestSender will eventually cause SlaveObject::done() to be executed
            after the last request is handled.

            To emphasize, the destructor only schedules the SlaveObject for deletion, it does not immediately delete it. */
        ~SlaveRequestSender();

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
        class ProxyTask;
    };

}

template<typename ObjectType, typename SlaveType>
class util::SlaveRequestSender<ObjectType,SlaveType>::ProxyTask : public BaseSlaveRequest<ObjectType> {
 public:
    ProxyTask(std::auto_ptr<SlaveRequest_t> req)
        : m_req(req)
        { }
    void handle(ObjectType& t, SlaveObject<ObjectType>& obj)
        {
            m_req->handle(t, static_cast<SlaveType&>(obj));
        }
 private:
    std::auto_ptr<SlaveRequest_t> m_req;
};

template<typename ObjectType, typename SlaveType>
inline
util::SlaveRequestSender<ObjectType,SlaveType>::SlaveRequestSender(RequestSender<ObjectType> sender, SlaveType* p)
    : BaseSlaveRequestSender<ObjectType>(sender, p)
{ }

template<typename ObjectType, typename SlaveType>
inline
util::SlaveRequestSender<ObjectType,SlaveType>::~SlaveRequestSender()
{ }

template<typename ObjectType, typename SlaveType>
void
util::SlaveRequestSender<ObjectType,SlaveType>::postNewRequest(SlaveRequest_t* p)
{
    std::auto_ptr<SlaveRequest_t> ap(p);
    BaseSlaveRequestSender<ObjectType>::postNewRequest(new ProxyTask(ap));
}

#endif
