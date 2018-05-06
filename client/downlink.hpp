/**
  *  \file client/downlink.hpp
  *  \brief Class client::Downlink
  */
#ifndef C2NG_CLIENT_DOWNLINK_HPP
#define C2NG_CLIENT_DOWNLINK_HPP

#include "client/widgets/busyindicator.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "util/request.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace client {

    /** Helper for calling "down" into the game/browser session.
        If a UI component needs information from the game/browser session (background thread),
        it can use a Downlink to quasi-synchronously communicate with it.
        Downlink will send a request to a RequestSender, and waits for the request to be answered.
        During that time, the sending thread (the UI thread) will be kept alive using an EventLoop,
        and the user sees a BusyIndicator.

        Use Downlink for information requests in reaction to user input.
        Do NOT use Downlink from a drawWidget() callback.

        If you're interacting with scripts, use client::si::Control. */
    class Downlink {
     public:
        /** Constructor.
            \param root UI Root */
        explicit Downlink(ui::Root& root);

        /** Destructor. */
        ~Downlink();

        /** Send request.
            This will send the request using \c sender and wait for it being processed.
            It will return after the confirmation arrives.

            Note that this function cannot be called recursively
            (e.g. from a UI callback that is active while call() is active).

            \tparam Target object type
            \param sender RequestSender to communicate with the target object
            \param req Request to execute
            \return true if request was executed, false if request could not be executed (other end died, or recursion). */
        template<typename T>
        bool call(util::RequestSender<T> sender, util::Request<T>& req);

     private:
        /** Request wrapper.
            RequestSender requires a newly-allocated request whose lifetime it controls.
            The Request we get has lifetime controlled by the user, and requires confirmation. */
        template<typename T>
        class RequestWrapper : public util::Request<T> {
         public:
            RequestWrapper(util::Request<T>& req, Downlink& link)
                : m_request(req), m_confirm(link.m_receiver.getSender()), m_success(false)
                { }
            ~RequestWrapper()
                { confirm(m_confirm, m_success); }
            void handle(T& t)
                { m_request.handle(t); m_success = true; }
         private:
            util::Request<T>& m_request;
            util::RequestSender<Downlink> m_confirm;
            bool m_success;
        };

        ui::Root& m_root;
        client::widgets::BusyIndicator m_indicator;
        bool m_busy;
        ui::EventLoop m_loop;
        util::RequestReceiver<Downlink> m_receiver;

        void setBusy(bool flag);
        static void confirm(util::RequestSender<Downlink>& sender, bool success);
    };

}

template<typename T>
bool
client::Downlink::call(util::RequestSender<T> sender, util::Request<T>& req)
{
    if (m_busy) {
        return false;
    } else {
        setBusy(true);
        sender.postNewRequest(new RequestWrapper<T>(req, *this));
        bool success = (m_loop.run() != 0);
        setBusy(false);
        return success;
    }
}

#endif
