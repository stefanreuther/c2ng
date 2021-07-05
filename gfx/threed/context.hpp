/**
  *  \file gfx/threed/context.hpp
  *  \brief Interface gfx::threed::Context
  */
#ifndef C2NG_GFX_THREED_CONTEXT_HPP
#define C2NG_GFX_THREED_CONTEXT_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"
#include "gfx/canvas.hpp"
#include "gfx/rectangle.hpp"
#include "gfx/threed/linerenderer.hpp"
#include "gfx/threed/particlerenderer.hpp"
#include "gfx/threed/trianglerenderer.hpp"

namespace gfx { namespace threed {

    /** 3D context.

        This interface allows simple 3D rendering.
        It allows creation of "...Renderer" classes which can display models of some kind.
        Each renderer draws into the Context it was created from.

        This class roughly corresponds to a WebGL context.
        The "...Renderer" classes roughly correspond to a shader program with associated data.

        To use,
        - create createLineRenderer() etc. to obtain renderers
        - configure the renderers (=define models)
        - call Context::start() to start a rendering
        - call each renderer's render() method to submit an instance, repeat as needed.
          Renderers can be invoked as often as needed.
        - call Context::finish() to finish.

        The coordinate handling is modeled after OpenGL.
        Coordinates are given as Vec3f.
        They are transformed by multiplying with transformation matrices (Mat4f).
        The result must be in the cube (-1,-1,-1) to (+1,+1,+1).

        This interface is NOT intended to be a full mapping of 3D APIs,
        and is probably NOT suited to make a first-person shooter or window compositor.

        Context instances are intended to be managed by Ptr/Ref. */
    class Context : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Start rendering.

            After this, call the renderers' render() methods, then finish().

            \param r Output viewport.
            \param can Target canvas. Must live at least until the call to finish().
                       It is unspecified at what time the canvas is modified;
                       modifications can start immediately or be deferred until finish(). */
        virtual void start(const Rectangle& r, Canvas& can) = 0;

        /** Finish rendering.
            This causes the image to appear on the canvas. */
        virtual void finish() = 0;

        /** Create LineRenderer.
            \return new LineRenderer instance */
        virtual afl::base::Ref<LineRenderer> createLineRenderer() = 0;

        /** Create TriangleRenderer.
            \return new TriangleRenderer instance */
        virtual afl::base::Ref<TriangleRenderer> createTriangleRenderer() = 0;

        /** Create ParticleRenderer.
            \return new ParticleRenderer instance */
        virtual afl::base::Ref<ParticleRenderer> createParticleRenderer() = 0;
    };

} }

#endif
