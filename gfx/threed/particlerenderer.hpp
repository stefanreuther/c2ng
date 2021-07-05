/**
  *  \file gfx/threed/particlerenderer.hpp
  *  \brief Class gfx::threed::ParticleRenderer
  */
#ifndef C2NG_GFX_THREED_PARTICLERENDERER_HPP
#define C2NG_GFX_THREED_PARTICLERENDERER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/refcounted.hpp"
#include "gfx/threed/vecmath.hpp"
#include "gfx/types.hpp"

namespace gfx { namespace threed {

    /** Particle renderer.

        Renders a set of circular particles.
        This is intended to render approximations of fire, smoke, etc.

        Each particle consists of a circle/ellipse centered at a point
        (represented as a two triangles forming a rectangle).
        All particles are colored with the same color gradient using 5 points (0, 1/4, 1/2, 3/4, 1),
        see setColor().

        By default, particles are rendered on the X/Y plane.
        If the scene is viewed at a different angle,
        they need to be rotated in the inverse direction; see setAxes().

        Use Context::createParticleRenderer() to create a ParticleRenderer.

        TODO: the JavaScript/WebGL version has a setDepthTest() to control the interaction
        of ParticleRenderer with other primitives (with GL, transparent primitives need to
        be drawn last, back-to-front). Make up some rules. */
    class ParticleRenderer : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Clear.
            Discards all content. */
        virtual void clear() = 0;

        /** Add particle

            \param pos   Position
            \param alpha Alpha (0.0=totally transparent) */
        virtual void add(Vec3f pos, float alpha) = 0;

        /** Set axes

            Every particle is rendered as a 1x1 square, (-0.5,-0.5) - (+0.5,+0.5) centered across its anchor point.
            This function determines where the axes lie in the scene transformation.

            If your model-view matrix includes a rotation, say, .rotateX(ax).rotateZ(az)),
            the inverse rotation will be, .rotateZ(-az).rotateX(-ax).

            Increasing the length of the axis vectors enlarges the particles.

            @param xa  X axis, i.e. inverseRotation.transform([1,0,0])
            @param ya  Y axis, i.e. inverseRotation.transform([0,1,0]) */
        virtual void setAxes(Vec3f xa, Vec3f ya) = 0;

        /** Set colors.

            Pass an array of 5 ColorQuad_ts.
            The first color will be used for the inside of the particle;
            the last color will be used for the outside.

            @param {Array} cs Colors */
        virtual void setColors(afl::base::Memory<const ColorQuad_t> color) = 0;

        /** Render.
            Call after Context::start().
            Causes all particles to be rendered onto the given canvas.

            \param proj       Projection transformation matrix
            \param modelView  Model/View transformation matrix */
        virtual void render(const Mat4f& proj, const Mat4f& modelView) = 0;
    };

} }


#endif
