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
        struct Definition {
            client::si::OutputState::Target target;
            const char* layoutName;
            const char* keymapName;
        };
        static const Definition ShipScreen;
        static const Definition PlanetScreen;
        static const Definition BaseScreen;

        ControlScreen(Session& session, int nr, const Definition& def);

        void run(client::si::InputState& in, client::si::OutputState& out);

     private:
        Session& m_session;
        int m_number;
        const Definition& m_definition;
    };

} }

#endif
