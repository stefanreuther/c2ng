/**
  *  \file game/vcr/classic/testapplet.hpp
  *  \brief Class game::vcr::classic::TestApplet
  */
#ifndef C2NG_GAME_VCR_CLASSIC_TESTAPPLET_HPP
#define C2NG_GAME_VCR_CLASSIC_TESTAPPLET_HPP

#include "util/applet.hpp"

namespace game { namespace vcr { namespace classic {

    /** VCR test applet.
        This applet is used for testing correctness and performance of VCR implementations.
        Its output matches that of other of my VCR implementations. */
    class TestApplet : public util::Applet {
     public:
        int run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl);

     private:
        static void help(util::Application& app);
    };

} } }

#endif
