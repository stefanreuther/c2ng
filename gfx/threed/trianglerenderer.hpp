/**
  *  \file gfx/threed/trianglerenderer.hpp
  *  \brief Class gfx::threed::TriangleRenderer
  */
#ifndef C2NG_GFX_THREED_TRIANGLERENDERER_HPP
#define C2NG_GFX_THREED_TRIANGLERENDERER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/refcounted.hpp"
#include "gfx/threed/vecmath.hpp"
#include "gfx/types.hpp"

namespace gfx { namespace threed {

    /** Triangle renderer.
        Renders a bunch of triangles.

        A triangle connects three vertices.
        Each vertex can have an own color and normal, for smoothing tricks, if supported by the implementation.

        We separate the vertices and triangle definitions so we can re-use vertices.
        To add some triangles,
        - call addVertices() to define the vertices. This produces an index.
        - call addTriangles(), passing it the just-obtained index and the relative indexes to build triangles.

        Use Context::createTriangleRenderer() to create a TriangleRenderer. */
    class TriangleRenderer : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Clear.
            Discards all content. */
        virtual void clear() = 0;

        /** Add vertices.
            \param points    List of points in 3D space
            \param normals   Normals for each vertex
            \param colors    Colors for each vertex

            The three memory descriptors must have the same number of elements.
            Behaviour is undefined if they don't.

            \return Index under which first point was stored.
            Indexes are not magic; you can rely on the first call to return 0,
            the second to return (number of points in first), and so on. */
        virtual size_t addVertices(afl::base::Memory<const Vec3f> points,
                                   afl::base::Memory<const Vec3f> normals,
                                   afl::base::Memory<const ColorQuad_t> colors) = 0;

        /** Add triangles.
            \param base      Value to add for each index.
            \param indexes   Indexes. Number of elements must be divisible by 3.
                             Each value, plus base, must be less than the number of vertices (=a valid index).

            The intended use is to call addVertices() to add some vertices, obtaining an index,
            and pass that index as base, as well as a constant array for indexes. */
        virtual void addTriangles(size_t base, afl::base::Memory<const size_t> indexes) = 0;

        /** Render.
            Call after Context::start().
            Causes all triangles to be rendered onto the given canvas.

            \param proj       Projection transformation matrix
            \param modelView  Model/View transformation matrix */
        virtual void render(const Mat4f& proj, const Mat4f& modelView) = 0;


        /** Add vertex.
            A convenience method for adding a single vertex.
            \param point   Point in 3D space
            \param normal  Normal
            \param color   Color
            \return Index under which point was stored. */
        size_t addVertex(Vec3f point, Vec3f normal, ColorQuad_t color);

        /** Add cylinder.

            Adds vertices to approximate a cylinder.
            The (circular) caps of the cylinder are approximated as n-sided polygons,
            producing a mesh of 2n triangles.
            Normals are added to make it look more round.

            \param a      Top cap center
            \param b      Bottom cap center
            \param r      Radius
            \param color  Color
            \param n      Precision. Number of corners of polygons the caps are approximated with. */
        void addCylinder(const Vec3f& a, const Vec3f& b, float r, ColorQuad_t color, int n);

        /** Add sphere.

            The sphere is approximated using a stack of n-sided polygons, connected with triangles.
            Normals are added to make it look more round.

            \param center Center
            \param r      Radius
            \param color  Color
            \param n      Precision. Number of corners of the polygons and number of polygons the sphere is approximated with. */
        void addSphere(const Vec3f& center, float r, ColorQuad_t color, int n);

        /** Add single triangle

            Adds a simple triangle, internally computing its normal from its edges.

            \param a     Point
            \param b     Point
            \param c     Point
            \param color Color */
        void addTriangle(const Vec3f& a, const Vec3f& b, const Vec3f& c, ColorQuad_t color);
    };

} }

#endif
