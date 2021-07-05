/**
  *  \file gfx/threed/trianglerenderer.cpp
  *  \brief Class gfx::threed::TriangleRenderer
  */

#include <cmath>
#include "gfx/threed/trianglerenderer.hpp"
#include "util/math.hpp"

namespace {
    // Build a triangle fan building a (convex) polygon
    void addPolygon(gfx::threed::TriangleRenderer& me,
                    const gfx::threed::Vec3f& center,
                    const std::vector<gfx::threed::Vec3f>& points,
                    gfx::ColorQuad_t color,
                    const gfx::threed::Vec3f& norm,
                    bool order)
    {
        size_t index = me.addVertex(center, norm, color);
        size_t o1 = order ? 1 : 2;
        size_t o2 = order ? 2 : 1;

        // Points
        for (size_t i = 0; i < points.size(); ++i) {
            me.addVertex(points[i], norm, color);
        }

        // Connect them
        for (size_t i = 0; i+1 < points.size(); ++i) {
            size_t data[] = {0, i+o1, i+o2};
            me.addTriangles(index, data);
        }
    }
}

size_t
gfx::threed::TriangleRenderer::addVertex(Vec3f point, Vec3f normal, ColorQuad_t color)
{
    Vec3f points[] = {point};
    Vec3f normals[] = {normal};
    ColorQuad_t colors[] = {color};
    return addVertices(points, normals, colors);
}

void
gfx::threed::TriangleRenderer::addCylinder(const Vec3f& a, const Vec3f& b, float r, ColorQuad_t color, int n)
{
    const Vec3f dir = b - a;
    const Vec3f x = dir.per().norm();
    const Vec3f y = x.prod(dir).norm();

    if (n <= 1) {
        return;
    }

    // 'a' cap
    std::vector<Vec3f> aCap;
    for (int i = 0; i < n; ++i) {
        double angle = 2*util::PI * i/n;
        aCap.push_back(a + x * float(r*std::sin(angle)) + y * float(r*std::cos(angle)));
    }
    aCap.push_back(aCap[0]);

    // 'b' cap
    std::vector<Vec3f> bCap;
    for (int i = 0; i < n; ++i) {
        double angle = 2*util::PI * (i+0.5)/n;
        bCap.push_back(b + x * float(r*std::sin(angle)) + y * float(r*std::cos(angle)));
    }
    bCap.push_back(bCap[0]);

    // Render sides
    for (int i = 0; i < n; ++i) {
        Vec3f p1 = aCap[i], p2 = aCap[i+1], p3 = bCap[i], p4 = bCap[i+1];
        ColorQuad_t colors[] = {color, color, color};

        {
            static const size_t index1[] = {0,1,2};
            Vec3f pos1[]  = {p1, p2, p3};
            Vec3f norm1[] = {(p1-a).norm(), (p2-a).norm(), (p3-b).norm()};
            addTriangles(addVertices(pos1, norm1, colors), index1);
        }

        {
            static const size_t index2[] = {1,0,2};
            Vec3f pos2[] = {p2, p3, p4};
            Vec3f norm2[] = {(p2-a).norm(), (p3-b).norm(), (p4-b).norm()};
            addTriangles(addVertices(pos2, norm2, colors), index2);
        }

        // Render caps
        addPolygon(*this, a, aCap, color, (b - a).norm(), false);
        addPolygon(*this, b, bCap, color, (a - b).norm(), true);
    }
}

void
gfx::threed::TriangleRenderer::addSphere(const Vec3f& center, float r, ColorQuad_t color, int n)
{
    class SphereHelper {
     public:
        SphereHelper(TriangleRenderer& me, const Vec3f& center, float r, ColorQuad_t color)
            : m_self(me), m_center(center), m_radius(r), m_color(color)
            { }
        size_t add(Vec3f vec)
            {
                // This is actually a semi-believable planet for n=15..25:
                // var f = vec[2]*vec[2]*vec[2]*vec[2];
                // me._colorData.pushN([
                //     f + (1-f)*Math.random()*0.2,
                //     f + (1-f)*Math.random()*0.8,
                //     f + (1-f)*Math.random()*0.8,
                //     1]);
                return m_self.addVertex(m_center + vec*m_radius, vec, m_color);
            }
     private:
        TriangleRenderer& m_self;
        const Vec3f& m_center;
        float m_radius;
        ColorQuad_t m_color;
    };
    SphereHelper h(*this, center, r, color);

    // +0 = top
    size_t index = h.add(Vec3f(0, 0, -1));

    // +1 .. +n = first ring
    // +1+n*(n-1) .. +n*n = n'th ring
    for (int lat = 1; lat < n; ++lat) {
        double a1 = util::PI * lat/n;
        double shift = (lat&1) * 0.5;
        for (int lon = 0; lon < n; ++lon) {
            double a2 = 2*util::PI * (lon + shift)/n;
            h.add(Vec3f(float(std::sin(a1)*std::sin(a2)),
                        float(std::sin(a1)*std::cos(a2)),
                        float(-std::cos(a1))));
        }
    }
    // +n*n+1 = bottom
    h.add(Vec3f(0, 0, 1));

    // Connect
    for (int lon = 0; lon < n; ++lon) {
        size_t is[] = {0, size_t(1+lon), size_t(1+(lon+1)%n)};
        addTriangles(index, is);
    }
    for (int lat = 1; lat+1 < n; ++lat) {
        for (int lon = 0; lon < n; ++lon) {
            if (lat & 1) {
                size_t is[] = { size_t(1+lat*n-n+(lon)%n),
                                size_t(1+lat*n+(lon)%n),
                                size_t(1+lat*n+(lon+1)%n),
                                size_t(1+lat*n-n+lon),
                                size_t(1+lat*n+(lon+1)%n),
                                size_t(1+lat*n-n+(lon+1)%n) };
                addTriangles(index, is);
            } else {
                size_t is[] = { size_t(1+lat*n-n+(lon+1)%n),
                                size_t(1+lat*n+(lon)%n),
                                size_t(1+lat*n+(lon+1)%n),
                                size_t(1+lat*n-n+(lon+1)%n),
                                size_t(1+lat*n-n+(lon)%n),
                                size_t(1+lat*n+(lon)%n) };
                addTriangles(index, is);
            }
        }
    }
    for (int lon = 0; lon < n; ++lon) {
        size_t is[] = { size_t(1+n*(n-1)), size_t(1+n*(n-2)+(lon+1)%n), size_t(1+n*(n-2)+lon) };
        addTriangles(index, is);
    }
}

void
gfx::threed::TriangleRenderer::addTriangle(const Vec3f& a, const Vec3f& b, const Vec3f& c, ColorQuad_t color)
{
    // Compute normal
    const Vec3f norm = (b - a).prod(c - a).norm();

    // Vertices
    const Vec3f posData[]         = {a, b, c};
    const Vec3f normalData[]      = {norm, norm, norm};
    const ColorQuad_t colorData[] = {color, color, color};
    size_t base = addVertices(posData, normalData, colorData);

    // Indexes
    static const size_t indexData[] = {0, 1, 2};
    addTriangles(base, indexData);
}
