/**
  *  \file gfx/threed/linerenderer.hpp
  *  \brief Class gfx::threed::LineRenderer
  */
#ifndef C2NG_GFX_THREED_LINERENDERER_HPP
#define C2NG_GFX_THREED_LINERENDERER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/refcounted.hpp"
#include "gfx/threed/vecmath.hpp"
#include "gfx/types.hpp"

namespace gfx { namespace threed {

    /** Line renderer.
        Renders a bunch of line segments.
        Each line segment connects two points in 3D space, with a given color.
        Can be used to display wireframes, grids, etc.

        Use Context::createLineRenderer() to create a LineRenderer. */
    class LineRenderer : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Clear.
            Discards all content. */
        virtual void clear() = 0;

        /** Add new line segment.
            \param from   Starting point
            \param to     Ending point
            \param color  Color */
        virtual void add(const Vec3f& from, const Vec3f& to, ColorQuad_t color) = 0;

        /** Render.
            Call after Context::start().
            Causes all line segments to be rendered onto the given canvas.

            \param proj       Projection transformation matrix
            \param modelView  Model/View transformation matrix */
        virtual void render(const Mat4f& proj, const Mat4f& modelView) = 0;


        /** Add cylinder wireframe.
            The cylinder is approximated as an N-sided prism.
            \param a      Top cap center
            \param b      Bottom cap center
            \param r      Radius
            \param color  Color
            \param n      Precision. Number of corners of the prism the cylinder is approximated with. */
        void addCylinder(const Vec3f& a, const Vec3f& b, float r, ColorQuad_t color, int n);

        /** Add sphere wireframe.
            The sphere is approximated as a stack of N-sided (truncated) pyramids.
            \param center Center
            \param r      Radius
            \param color  Color
            \param n      Precision. Number of corners of the pyramids and number of pyramids the sphere is approximated with. */
        void addSphere(const Vec3f& center, float r, ColorQuad_t color, int n);
    };

} }

#endif
