/**
  *  \file client/downlink.hpp
  *  \brief Class client::Downlink
  */
#ifndef C2NG_CLIENT_DOWNLINK_HPP
#define C2NG_CLIENT_DOWNLINK_HPP

#include "afl/string/translator.hpp"
#include "client/widgets/busyindicator.hpp"
#include "game/proxy/waitindicator.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"

namespace client {

    /** Helper for calling "down" into the game/browser session with UI synchronisation.
        This implements a WaitIndicator using the UI framework.
        During the wait time, the sending thread (the UI thread) will be kept alive using an EventLoop,
        and the user sees a BusyIndicator.

        Use Downlink for information requests in reaction to user input.
        Do NOT use Downlink from a drawWidget() callback.

        If you're interacting with scripts, use client::si::Control. */
    class Downlink : public game::proxy::WaitIndicator {
     public:
        /** Constructor.
            \param root UI Root
            \param tx Translator */
        explicit Downlink(ui::Root& root, afl::string::Translator& tx);

        ~Downlink();

        // WaitIndicator:
        void post(bool success);
        bool wait();

     private:
        ui::Root& m_root;
        client::widgets::BusyIndicator m_indicator;
        bool m_busy;
        ui::EventLoop m_loop;

        void setBusy(bool flag);
    };
}

#endif
