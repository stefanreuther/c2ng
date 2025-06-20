/**
  *  \file game/browser/testapplet.hpp
  *  \brief Class game::browser::TestApplet
  */
#ifndef C2NG_GAME_BROWSER_TESTAPPLET_HPP
#define C2NG_GAME_BROWSER_TESTAPPLET_HPP

#include "afl/net/networkstack.hpp"
#include "util/applet.hpp"

namespace game { namespace browser {

    /** Interactive game browser test applet. */
    class TestApplet : public util::Applet {
     public:
        /** Constructor.
            @param net Network stack */
        TestApplet(afl::net::NetworkStack& net);

        // Applet:
        int run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl);

     private:
        afl::net::NetworkStack& m_networkStack;
        class MyUserCallback;
    };

} }

#endif
