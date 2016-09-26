/**
  *  \file client/screens/playerscreen.hpp
  */
#ifndef C2NG_CLIENT_SCREENS_PLAYERSCREEN_HPP
#define C2NG_CLIENT_SCREENS_PLAYERSCREEN_HPP

#include "client/session.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/outputstate.hpp"

namespace client { namespace screens {

    void doPlayerScreen(Session& session, client::si::InputState& in, client::si::OutputState& out);

} }

#endif
