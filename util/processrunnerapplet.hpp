/**
  *  \file util/processrunnerapplet.hpp
  *  \brief Class util::ProcessRunnerApplet
  */
#ifndef C2NG_UTIL_PROCESSRUNNERAPPLET_HPP
#define C2NG_UTIL_PROCESSRUNNERAPPLET_HPP

#include "util/applet.hpp"

namespace util {

    /** Test applet for ProcessRunner. */
    class ProcessRunnerApplet : public Applet {
     public:
        virtual int run(Application& app, afl::sys::Environment::CommandLine_t& cmdl);
    };

}

#endif
