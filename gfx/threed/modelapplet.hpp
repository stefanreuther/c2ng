/**
  *  \file gfx/threed/modelapplet.hpp
  *  \brief gfx::threed::ModelApplet
  */
#ifndef C2NG_GFX_THREED_MODELAPPLET_HPP
#define C2NG_GFX_THREED_MODELAPPLET_HPP

#include "gfx/applet.hpp"

namespace gfx { namespace threed {

    /** Test applet to render 3-D model files. */
    class ModelApplet : public Applet {
     public:
        virtual int run(Application& app, Engine& engine, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::sys::Environment::CommandLine_t& cmdl);

     private:
        class App;
    };

} }

#endif
