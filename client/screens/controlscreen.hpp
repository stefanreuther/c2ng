/**
  *  \file client/screens/controlscreen.hpp
  */
#ifndef C2NG_CLIENT_SCREENS_CONTROLSCREEN_HPP
#define C2NG_CLIENT_SCREENS_CONTROLSCREEN_HPP

#include "client/session.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/outputstate.hpp"

namespace client { namespace screens {

    class ControlScreen {
     public:
        ControlScreen(Session& session, int nr, client::si::OutputState::Target me);

        void run(client::si::InputState& in, client::si::OutputState& out);

     private:
        Session& m_session;
        int m_number;
        client::si::OutputState::Target m_me;
    };

} }

#endif
