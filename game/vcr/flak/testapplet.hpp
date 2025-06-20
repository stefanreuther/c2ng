/**
  *  \file game/vcr/flak/testapplet.hpp
  *  \brief Class game::vcr::flak::TestApplet
  */
#ifndef C2NG_GAME_VCR_FLAK_TESTAPPLET_HPP
#define C2NG_GAME_VCR_FLAK_TESTAPPLET_HPP

#include "util/applet.hpp"

namespace game { namespace vcr { namespace flak {

    /** FLAK test applet.
        This applet is used for testing correctness and performance of the FLAK port.
        Its output matches that of other implementations. */
    class TestApplet : public util::Applet {
     public:
        int run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl);
    };

} } }

#endif
