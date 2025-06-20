/**
  *  \file gfx/threed/renderapplet.hpp
  *  \brief Class gfx::threed::RenderApplet
  */
#ifndef C2NG_GFX_THREED_RENDERAPPLET_HPP
#define C2NG_GFX_THREED_RENDERAPPLET_HPP

#include "gfx/applet.hpp"

namespace gfx { namespace threed {

    /** Test applet for 3-D rendering.
        Displays a fixed model using either lines or triangles. */
    class RenderApplet : public Applet {
     public:
        enum Mode {
            /** Lines (wireframe). */
            Lines,
            /** Triangles (solid). */
            Triangles
        };

        /** Constructor.
            @param mode Desired mode (Lines or Triangles) */
        RenderApplet(Mode mode);

        virtual int run(Application& app, Engine& engine, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::sys::Environment::CommandLine_t& cmdl);

     private:
        template<typename T> class App;

        Mode m_mode;
    };

} }

#endif
