/**
  *  \file client/downlink.hpp
  *  \brief Class client::Downlink
  */
#ifndef C2NG_CLIENT_DOWNLINK_HPP
#define C2NG_CLIENT_DOWNLINK_HPP

#include "client/si/userside.hpp"
#include "ui/waitindicator.hpp"

namespace client {

    /** Helper for calling "down" into the game/browser session with UI synchronisation.
        This extends ui::WaitIndicator with a constructor taking a client::si::UserSide
        that allows canceling background scripts.

        If you're interacting with scripts, use client::si::Control. */
    class Downlink : public ui::WaitIndicator {
     public:
        /** Constructor.
            \param root UI Root
            \param tx Translator */
        explicit Downlink(ui::Root& root, afl::string::Translator& tx);

        /** Constructor.
            If the Downlink is constructed using this signature, it will allow cancelling potential background scripts.
            \param us UserSide */
        explicit Downlink(client::si::UserSide& us);
    };
}

#endif
