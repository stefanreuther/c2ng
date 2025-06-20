/**
  *  \file util/directorybrowserapplet.hpp
  *  \brief Class util::DirectoryBrowserApplet
  */
#ifndef C2NG_UTIL_DIRECTORYBROWSERAPPLET_HPP
#define C2NG_UTIL_DIRECTORYBROWSERAPPLET_HPP

#include "util/applet.hpp"
#include "util/application.hpp"

namespace util {

    /** Interactive test applet for util::DirectoryBrowser. */
    class DirectoryBrowserApplet : public Applet {
     public:
        int run(Application& app, afl::sys::Environment::CommandLine_t& cmdl);
    };

}

#endif
