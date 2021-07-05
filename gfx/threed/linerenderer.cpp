/**
  *  \file gfx/threed/linerenderer.cpp
  *  \brief Class gfx::threed::LineRenderer
  */

#include <vector>
#include "gfx/threed/linerenderer.hpp"
#include "util/math.hpp"

using gfx::threed::Vec3f;

namespace {
    struct SphereHelper {
        SphereHelper(const Vec3f& center, float r)
            : center(center), r(r)
            { }
        Vec3f make(const Vec3f& vec) const
            { return center + vec*r; }

        void makeRing(std::vector<Vec3f>& result, int lat, int n) const;

        const Vec3f& center;
        float r;
    };
}

void SphereHelper::makeRing(std::vector<Vec3f>& result, int lat, int n) const
{
    float a1 = float(util::PI * lat/n);
    for (int lon = 0; lon < n; ++lon) {
        float a2 = float(2*util::PI * lon/n);
        result.push_back(make(Vec3f(std::sin(a1)*std::sin(a2), std::sin(a1)*std::cos(a2), -std::cos(a1))));
    }
}


void
gfx::threed::LineRenderer::addCylinder(const Vec3f& a, const Vec3f& b, float r, ColorQuad_t color, int n)
{
    Vec3f dir = b - a;
    Vec3f x = dir.per().norm();
    Vec3f y = x.prod(dir).norm();

    // Prepare caps
    std::vector<Vec3f> aCap, bCap;
    aCap.reserve(n+1);
    bCap.reserve(n+1);
    for (int i = 0; i < n; ++i) {
        float angle = float(2*util::PI * i/n);
        Vec3f pt = x * (r*std::sin(angle)) + y * (r*std::cos(angle));
        aCap.push_back(a + pt);
        bCap.push_back(b + pt);
    }
    aCap.push_back(aCap[0]);
    bCap.push_back(bCap[0]);

    // Render
    for (int i = 0; i < n; ++i) {
        add(aCap[i], aCap[i+1], color);
        add(bCap[i], bCap[i+1], color);
        add(aCap[i], bCap[i],   color);
    }
}

void
gfx::threed::LineRenderer::addSphere(const Vec3f& center, float r, ColorQuad_t color, int n)
{
    SphereHelper h(center, r);

    // Top
    Vec3f top = h.make(Vec3f(0,0,-1));

    // First ring (lat=2)
    std::vector<Vec3f> a;
    h.makeRing(a, 1, n);
    for (int lon = 0; lon < n; ++lon) {
        add(top, a[lon], color);
        add(a[lon], a[(lon+1)%n], color);
    }

    // Further rings
    for (int lat = 2; lat < n; ++lat) {
        std::vector<Vec3f> b;
        h.makeRing(b, lat, n);
        for (int lon = 0; lon < n; ++lon) {
            add(a[lon], b[lon], color);
            add(b[lon], b[(lon+1)%n], color);
        }
        a.swap(b);
    }

    // Bottom
    Vec3f bot = h.make(Vec3f(0,0,1));
    for (int lon = 0; lon < n; ++lon) {
        add(a[lon], bot, color);
    }
}
