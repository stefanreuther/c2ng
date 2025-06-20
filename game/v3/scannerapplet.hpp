/**
  *  \file game/v3/scannerapplet.hpp
  *  \brief Class game::v3::ScannerApplet
  */
#ifndef C2NG_GAME_V3_SCANNERAPPLET_HPP
#define C2NG_GAME_V3_SCANNERAPPLET_HPP

#include "util/applet.hpp"

namespace game { namespace v3 {

    /** Test applet for DirectoryScanner.
        Takes a list of directories, and invokes DirectoryScanner on each. */
    class ScannerApplet : public util::Applet {
     public:
        virtual int run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl);
    };

} }

#endif
