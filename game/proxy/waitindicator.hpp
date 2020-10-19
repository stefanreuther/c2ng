/**
  *  \file game/proxy/waitindicator.hpp
  *  \brief Class game::proxy::WaitIndicator
  */
#ifndef C2NG_GAME_PROXY_WAITINDICATOR_HPP
#define C2NG_GAME_PROXY_WAITINDICATOR_HPP

#include "util/request.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequest.hpp"
#include "util/slaverequestsender.hpp"

namespace game { namespace proxy {

    /** Helper for calling "down" into a proxy from a UI thread.
        If a UI component needs information from the game/browser session (background thread),
        it can use a WaitIndicator to quasi-synchronously communicate with it.

        The WaitIndicator interface allows proxy implementations to signal the wait situation,
        thus keeping a UI message pump alive and possibly showing a busy indication.
        Derived classes implement the actual UI policy:
        - wait() to start a wait, and drive a nested message pump
        - post() to finish a wait

        The post() call is made from the same thread as wait() using a RequestDispatcher
        to inject it into wait()'s message pump.
        This is done to benefit from RequestSender/RequestReceiver's lifetime management.
        Just mapping post()/wait() to the equivalent calls of, say, a semaphore,
        would require post() to manage lifetime of that semaphore if the UI thread exits prematurely.

        Implementors need not deal with nested wait()/post() calls. */
    class WaitIndicator {
     public:
        /** Constructor.
            \param disp RequestDispatcher to dispatch into this thread */
        explicit WaitIndicator(util::RequestDispatcher& disp);

        /** Destructor. */
        virtual ~WaitIndicator();

        /** Send request (RequestSender).
            This will send the request using \c sender and wait for it being processed.
            It will return after the confirmation arrives.

            Note that this function cannot be called recursively
            (e.g. from a UI callback that is active while call() is active).

            \tparam T object type
            \param sender RequestSender to communicate with the target object
            \param req Request to execute
            \return true if request was executed, false if request could not be executed (other end died, or recursion). */
        template<typename T>
        bool call(util::RequestSender<T> sender, util::Request<T>& req);

        /** Send request (SlaveRequestSender).
            This will send the request using \c sender and wait for it being processed.
            It will return after the confirmation arrives.

            Note that this function cannot be called recursively
            (e.g. from a UI callback that is active while call() is active).

            \tparam T object type
            \tparam S object type
            \param sender SlaveRequestSender to communicate with the target object
            \param req Request to execute
            \return true if request was executed, false if request could not be executed (other end died, or recursion). */
        template<typename T, typename S>
        bool call(util::SlaveRequestSender<T,S>& sender, util::SlaveRequest<T,S>& req);

        /** Release this thread.
            Upon completion of the task given to the target object,
            a call to post() will be posted into this thread's RequestDispatcher,
            which will probably already sit in wait().
            This call must release the wait() call.
            \param success Value to return from wait() */
        virtual void post(bool success) = 0;

        /** Suspend this thread until post() is called.
            Upon completion of the task given to the target object,
            a call to post() will be posted into this thread's RequestDispatcher
            as given to the WaitIndicator's constructor.
            \return Value passed to post(). */
        virtual bool wait() = 0;

     private:
        /** Request wrapper.
            RequestSender requires a newly-allocated request whose lifetime it controls.
            The Request we get has lifetime controlled by the user, and requires confirmation. */
        template<typename T>
        class RequestWrapper : public util::Request<T> {
         public:
            RequestWrapper(util::Request<T>& req, WaitIndicator& link)
                : m_request(req), m_confirm(link.m_receiver.getSender()), m_success(false)
                { }
            ~RequestWrapper()
                { confirm(m_confirm, m_success); }
            void handle(T& t)
                { m_request.handle(t); m_success = true; }
         private:
            util::Request<T>& m_request;
            util::RequestSender<WaitIndicator> m_confirm;
            bool m_success;
        };

        /** Request wrapper.
            Same thing for SlaveRequestSender. */
        template<typename T, typename S>
        class SlaveRequestWrapper : public util::SlaveRequest<T,S> {
         public:
            SlaveRequestWrapper(util::SlaveRequest<T,S>& req, WaitIndicator& link)
                : m_request(req), m_confirm(link.m_receiver.getSender()), m_success(false)
                { }
            ~SlaveRequestWrapper()
                { confirm(m_confirm, m_success); }
            void handle(T& t, S& s)
                { m_request.handle(t, s); m_success = true; }
         private:
            util::SlaveRequest<T, S>& m_request;
            util::RequestSender<WaitIndicator> m_confirm;
            bool m_success;
        };

        util::RequestReceiver<WaitIndicator> m_receiver;

        static void confirm(util::RequestSender<WaitIndicator>& sender, bool success);
    };

} }

template<typename T>
bool
game::proxy::WaitIndicator::call(util::RequestSender<T> sender, util::Request<T>& req)
{
    sender.postNewRequest(new RequestWrapper<T>(req, *this));
    return this->wait();
}

template<typename T, typename S>
bool
game::proxy::WaitIndicator::call(util::SlaveRequestSender<T,S>& sender, util::SlaveRequest<T,S>& req)
{
    sender.postNewRequest(new SlaveRequestWrapper<T,S>(req, *this));
    return this->wait();
}

#endif
