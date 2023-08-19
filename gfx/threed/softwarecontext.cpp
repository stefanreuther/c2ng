/**
  *  \file gfx/threed/softwarecontext.cpp
  *  \brief Class gfx::threed::SoftwareContext
  *
  *  Basic idea:
  *
  *  - when ...Renderer::render() is called, primitives are instantiated.
  *    That is, they are transformed and, if visible (front-face visible, not clipped), collected for later rendering.
  *  - to avoid having one virtual object 'Primitive' for each primitive, make one 'Instance' object for each render() call, collecting that call's primitives.
  *  - to save some memory, don't store pointers; instead, store indexes into the list of Instances, or into the list of Primitives of an instance.
  *  - when finish() is called, sort all primitives by estimated Z order and draw from back to front, front overwriting back.
  *    (on the plus side, this means that particles just work.)
  */

#include <algorithm>
#include <stdexcept>
#include "gfx/threed/softwarecontext.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/complex.hpp"

namespace {
    gfx::Point convertCoordinates(const gfx::Rectangle& area, const gfx::threed::Vec3f& pos)
    {
        return gfx::Point(int((pos(0) + 1.0) * 0.5 * area.getWidth()  + area.getLeftX() + 0.5),
                          int((1.0 - pos(1)) * 0.5 * area.getHeight() + area.getTopY()  + 0.5));
    }

    int clampComponent(float f)
    {
        // The '& 255' is required to handle NaN, which will otherwise produce INT_MIN despite reporting to be in range [0,255]...
        return f > 255 ? 255 : f < 0 ? 0 : (int(f) & 255);
    }

    static const uint8_t GAMMA[] = {
          0,   20,   28,   33,   38,   42,   46,   49,   52,   55,   58,   61,   63,   65,   68,   70,
         72,   74,   76,   78,   80,   81,   83,   85,   87,   88,   90,   91,   93,   94,   96,   97,
         99,  100,  102,  103,  104,  106,  107,  108,  109,  111,  112,  113,  114,  115,  117,  118,
        119,  120,  121,  122,  123,  124,  125,  126,  128,  129,  130,  131,  132,  133,  134,  135,
        136,  136,  137,  138,  139,  140,  141,  142,  143,  144,  145,  146,  147,  147,  148,  149,
        150,  151,  152,  153,  153,  154,  155,  156,  157,  158,  158,  159,  160,  161,  162,  162,
        163,  164,  165,  165,  166,  167,  168,  168,  169,  170,  171,  171,  172,  173,  174,  174,
        175,  176,  176,  177,  178,  178,  179,  180,  181,  181,  182,  183,  183,  184,  185,  185,
        186,  187,  187,  188,  189,  189,  190,  190,  191,  192,  192,  193,  194,  194,  195,  196,
        196,  197,  197,  198,  199,  199,  200,  200,  201,  202,  202,  203,  203,  204,  205,  205,
        206,  206,  207,  208,  208,  209,  209,  210,  210,  211,  212,  212,  213,  213,  214,  214,
        215,  216,  216,  217,  217,  218,  218,  219,  219,  220,  220,  221,  222,  222,  223,  223,
        224,  224,  225,  225,  226,  226,  227,  227,  228,  228,  229,  229,  230,  230,  231,  231,
        232,  232,  233,  233,  234,  234,  235,  235,  236,  236,  237,  237,  238,  238,  239,  239,
        240,  240,  241,  241,  242,  242,  243,  243,  244,  244,  245,  245,  246,  246,  247,  247,
        248,  248,  249,  249,  249,  250,  250,  251,  251,  252,  252,  253,  253,  254,  254,  255,
    };

    /* Make a color.
       By default, this version looks rather dark.
       I'm totally not sure whether gamma is the right way to fix it, but it brings it a little closer to the brightness of the WebGL version. */
    gfx::ColorQuad_t makeColor(gfx::ColorQuad_t color,
                               gfx::threed::Vec3f lighting)
    {
        float r = RED_FROM_COLORQUAD  (color) * lighting(0);
        float g = GREEN_FROM_COLORQUAD(color) * lighting(1);
        float b = BLUE_FROM_COLORQUAD (color) * lighting(2);
        return COLORQUAD_FROM_RGB(GAMMA[clampComponent(r)],
                                  GAMMA[clampComponent(g)],
                                  GAMMA[clampComponent(b)]);
    }

    /* If a point is outside the [-1,1] cube, return bitfield of which sides it is outside of. */
    int classifyPoint(const gfx::threed::Vec3f& p)
    {
        int result = 0;
        if (p(0) < -1.0) result += 1;
        if (p(0) >  1.0) result += 2;
        if (p(1) < -1.0) result += 4;
        if (p(1) >  1.0) result += 8;
        if (p(2) < -1.0) result += 16;
        if (p(2) >  1.0) result += 32;
        return result;
    }

    /* Check whether line is (possibly) visible.
       For now, checks whether it is entirely on one side, e.g. both X coordinates > 1.0.
       This is exact enough for our needs.
       A more complex check (e.g. distance to origin = |a x (b-a)| / |a| > sqrt(3)) brings only minor improvements. */
    bool isVisibleLine(const gfx::threed::Vec3f& a, const gfx::threed::Vec3f& b)
    {
        return (classifyPoint(a) & classifyPoint(b)) == 0;
    }

    /* Check whether line is large enough to legitimate a split.
       We set a lower split threshold for the Z axis to reduce Z sorting artifacts. */
    bool checkSplitLine(const gfx::threed::Vec3f& a, const gfx::threed::Vec3f& b)
    {
        return std::abs(a(0) - b(0)) > 0.25
            || std::abs(a(1) - b(1)) > 0.25
            || std::abs(a(2) - b(2)) > (1.0 / 256);
    }
}


/*
 *  Instance base class
 *  Every instance of a model in the scene is represented by an instance of this class
 */

class gfx::threed::SoftwareContext::Instance : public afl::base::Deletable {
 public:
    virtual void renderPrimitive(const Rectangle& r, Canvas& can, Index_t index) = 0;
};


/*
 *  Instance for LineRenderer
 */

class gfx::threed::SoftwareContext::LineRendererInstance : public Instance {
 public:
    virtual void renderPrimitive(const Rectangle& /*r*/, Canvas& can, Index_t index)
        {
            Line& n = m_lines[index];
            Color_t color[1];
            can.encodeColors(n.color, color);
            BaseContext ctx(can);
            ctx.setRawColor(color[0]);
            drawLine(ctx, n.from, n.to);
        }

    Index_t add(Point from, Point to, ColorQuad_t color)
        {
            Line n = {from, to, {color}};
            Index_t result = Index_t(m_lines.size());
            m_lines.push_back(n);
            return result;
        }

 private:
    struct Line {
        Point from;
        Point to;
        ColorQuad_t color[1];
    };
    std::vector<Line> m_lines;
};


/*
 *  Public interface for LineRenderer
 */

class gfx::threed::SoftwareContext::LineRendererImpl : public LineRenderer {
 public:
    LineRendererImpl(afl::base::Ref<SoftwareContext> parent)
        : m_parent(parent),
          m_lines()
        { }

    virtual void clear()
        {
            m_lines.clear();
        }

    virtual void add(const Vec3f& from, const Vec3f& to, ColorQuad_t color)
        {
            Line e = {from, to, color};
            m_lines.push_back(e);
        }

    virtual void render(const Mat4f& proj, const Mat4f& modelView)
        {
            LineRendererInstance* instance = new LineRendererInstance();
            const Index_t instanceNr = m_parent->addNewInstance(instance);

            /*
                Subdivision and clipping

                If a line segment is too large, we split it into two, recursively.
                Lines are split using model coordinates because the transformation to view coordinates is not linear.
                Each individual segment is clipped.
            */

            /* Age limit: we will not subdivide a line more than this many times (=into 2**AGE_LIMIT segments).
               This is to avoid that floating-point effects (e.g. NaN, infinities) spoil our day. */
            const int AGE_LIMIT = 20;

            const Mat4f m = proj * modelView;
            for (size_t i = 0, n = m_lines.size(); i < n; ++i) {
                // Transform the color. We're not excitingly good with (adjacent) alpha lines, so pre-multiply alpha and draw solid lines.
                // This only works when you got few lines and a black background for now.
                ColorQuad_t color = m_lines[i].color;
                ColorQuad_t effColor = COLORQUAD_FROM_RGB((RED_FROM_COLORQUAD(color)   * ALPHA_FROM_COLORQUAD(color)) >> 8,
                                                          (GREEN_FROM_COLORQUAD(color) * ALPHA_FROM_COLORQUAD(color)) >> 8,
                                                          (BLUE_FROM_COLORQUAD(color)  * ALPHA_FROM_COLORQUAD(color)) >> 8);

                // Start with one segment
                std::vector<Segment> todo;
                todo.push_back(Segment(m_lines[i].from, m_lines[i].to, 0));
                while (!todo.empty()) {
                    const Vec3f from = todo.back().from;
                    const Vec3f to   = todo.back().to;
                    const int   age  = todo.back().age;
                    todo.pop_back();

                    // Check 'w' of transformed value
                    // Negative 'w' essentially means the point is beyond the camera plane.
                    const float fromw = m(3)*from(0) + m(7)*from(1) + m(11)*from(2) + m(15);
                    const float tow   = m(3)*to(0)   + m(7)*to(1)   + m(11)*to(2)   + m(15);
                    if (fromw <= 0 && tow <= 0) {
                        // Both 'w' are negative. Ignore this segment.
                    } else if (fromw <= 0 || tow <= 0) {
                        // One 'w' is negative. If we have age remaining, try to subdivide and find segments that are visible.
                        if (age < AGE_LIMIT) {
                            const Vec3f mid = (from + to) * 0.5;
                            todo.push_back(Segment(from, mid, age+1));
                            todo.push_back(Segment(mid, to, age+1));
                        }
                    } else {
                        // Transformation is possible: try to render
                        const Vec3f from1 = from.transform(m);
                        const Vec3f to1   = to.transform(m);
                        if (isVisibleLine(from1, to1)) {
                            if (checkSplitLine(from1, to1)) {
                                if (age < AGE_LIMIT) {
                                    const Vec3f mid = (from + to) * 0.5;
                                    todo.push_back(Segment(from, mid, age+1));
                                    todo.push_back(Segment(mid, to, age+1));
                                }
                            } else {
                                m_parent->addPrimitive((from1(2) + to1(2)) * 0.5f,      // z
                                                       instanceNr,
                                                       instance->add(convertCoordinates(m_parent->m_viewport, from1),
                                                                     convertCoordinates(m_parent->m_viewport, to1),
                                                                     effColor));
                            }
                        }
                    }
                }
            }
        }

 private:
    struct Segment {
        Vec3f from, to;
        int age;
        Segment(const Vec3f& from, const Vec3f& to, int age)
            : from(from), to(to), age(age)
            { }
    };
    afl::base::Ref<SoftwareContext> m_parent;
    struct Line {
        Vec3f from;
        Vec3f to;
        ColorQuad_t color;
    };
    std::vector<Line> m_lines;
};


/*
 *  Instance for TriangleRenderer
 */

class gfx::threed::SoftwareContext::TriangleRendererInstance : public Instance {
 public:
    virtual void renderPrimitive(const Rectangle& /*r*/, Canvas& can, Index_t index)
        {
            const Triangle& t = m_triangles[index];
            Color_t color[1];
            can.encodeColors(t.color, color);
            BaseContext ctx(can);
            ctx.setRawColor(color[0]);
            drawFilledPolygon(ctx, t.pos);
        }

    Index_t add(const Point& a, const Point& b, const Point& c, ColorQuad_t color)
        {
            Triangle t = {{a,b,c}, {color}};
            Index_t result = Index_t(m_triangles.size());
            m_triangles.push_back(t);
            return result;
        }

 private:
    struct Triangle {
        Point pos[3];
        ColorQuad_t color[1];
    };
    std::vector<Triangle> m_triangles;
};


/*
 *  Public interface for TriangleRenderer
 */

class gfx::threed::SoftwareContext::TriangleRendererImpl : public TriangleRenderer {
 public:
    TriangleRendererImpl(afl::base::Ref<SoftwareContext> parent)
        : m_parent(parent),
          m_vertices(),
          m_indexes()
        { }

    virtual void clear()
        {
            m_vertices.clear();
            m_indexes.clear();
        }
    virtual size_t addVertices(afl::base::Memory<const Vec3f> points,
                               afl::base::Memory<const Vec3f> normals,
                               afl::base::Memory<const ColorQuad_t> colors)
        {
            size_t result = m_vertices.size();
            const Vec3f *p, *n;
            const ColorQuad_t* col;
            while ((p = points.eat()) != 0 && (n = normals.eat()) != 0 && (col = colors.eat()) != 0) {
                Vertex v = {*p, *n, *col};
                m_vertices.push_back(v);
            }
            return result;
        }

    virtual void addTriangles(size_t base, afl::base::Memory<const size_t> indexes)
        {
            afl::except::checkAssertion(indexes.size() % 3 == 0, "SoftwareContext::LineRenderer::addVertices: bad number of points");
            while (const size_t* p = indexes.eat()) {
                size_t n = base + *p;
                afl::except::checkAssertion(n < m_vertices.size(), "SoftwareContext::LineRenderer::addVertices: bad index");
                m_indexes.push_back(n);
            }
        }

    virtual void render(const Mat4f& proj, const Mat4f& modelView)
        {
            TriangleRendererInstance* instance = new TriangleRendererInstance();
            Index_t instanceNr = m_parent->addNewInstance(instance);

            // Lighting parameters
            // FIXME: make the light parameters configurable (for now, same lighting as in JS, and not very good)
            const Vec3f ambientLight(0.1f, 0.1f, 0.1f);
            const Vec3f directionalLightColor(0.25f, 0.25f, 0.25f);
            const Vec3f directionalVector = Vec3f(0.85f, 0.8f, 0.75f).norm();

            for (size_t i = 0, n = m_indexes.size(); i+2 < n; i += 3) {
                // Transformations
                const Vertex& a = m_vertices[m_indexes[i]];
                const Vertex& b = m_vertices[m_indexes[i+1]];
                const Vertex& c = m_vertices[m_indexes[i+2]];
                const Vec3f amod = a.pos.transform(modelView);
                const Vec3f bmod = b.pos.transform(modelView);
                const Vec3f cmod = c.pos.transform(modelView);
                const Vec3f aproj = amod.transform(proj);
                const Vec3f bproj = bmod.transform(proj);
                const Vec3f cproj = cmod.transform(proj);

                // Clipping: a triangle is clipped if all its points are on one side of the [-1,1] cube.
                // If points are outside but on different sides, it can still be visible.
                if ((classifyPoint(aproj) & classifyPoint(bproj) & classifyPoint(cproj)) != 0) {
                    continue;
                }

                // Visibility (back-face) check
                const Vec3f vec = (bproj - aproj).prod(cproj - aproj);
                if (vec(2) < 0) {
                    continue;
                }

                // Determine color. Flat shading for now, i.e. all the same color.
                const Vec3f norm = (bmod - amod).prod(cmod - amod).norm();
                const float directional = std::max(norm.dot(directionalVector), 0.0f);
                const Vec3f lighting = ambientLight + directionalLightColor * directional;
                const ColorQuad_t color = makeColor(a.color, lighting);

                // Subdivide or add
                const size_t MAX_AGE = 3;
                std::vector<Segment> stack;
                stack.reserve(20);
                stack.push_back(Segment(aproj, bproj, cproj, MAX_AGE));
                while (!stack.empty()) {
                    Segment seg = stack.back();
                    stack.pop_back();
                    if (isnan(seg.a(0)) || isnan(seg.a(1)) || isnan(seg.a(2))
                        || isnan(seg.b(0)) || isnan(seg.b(1)) || isnan(seg.b(2))
                        || isnan(seg.c(0)) || isnan(seg.c(1)) || isnan(seg.c(2)))
                    {
                        // Ignore - FP madness
                    } else if ((classifyPoint(aproj) & classifyPoint(bproj) & classifyPoint(cproj)) != 0) {
                        // Ignore - out of view
                    } else if (seg.age > 0 && (checkSplitLine(seg.a, seg.b) || checkSplitLine(seg.a, seg.c) || checkSplitLine(seg.b, seg.c))) {
                        // Subdivide
                        const Vec3f ab = (seg.a + seg.b) * 0.5;
                        const Vec3f ac = (seg.a + seg.c) * 0.5;
                        const Vec3f bc = (seg.b + seg.c) * 0.5;
                        stack.push_back(Segment(seg.a, ab, ac, seg.age-1));
                        stack.push_back(Segment(ab, seg.b, bc, seg.age-1));
                        stack.push_back(Segment(ac, bc, seg.c, seg.age-1));
                        stack.push_back(Segment(ab, bc, ac, seg.age-1));
                    } else {
                        const float z = (seg.a(2) + seg.b(2) + seg.c(2)) * (1.0f/3.0f);
                        m_parent->addPrimitive(z, instanceNr,
                                               instance->add(convertCoordinates(m_parent->m_viewport, seg.a),
                                                             convertCoordinates(m_parent->m_viewport, seg.b),
                                                             convertCoordinates(m_parent->m_viewport, seg.c),
                                                             color));
                    }
                }
            }
        }

 private:
    struct Segment {
        Vec3f a;
        Vec3f b;
        Vec3f c;
        size_t age;

        Segment(const Vec3f& a, const Vec3f& b, const Vec3f& c, size_t age)
            : a(a), b(b), c(c), age(age)
            { }
    };

    afl::base::Ref<SoftwareContext> m_parent;
    struct Vertex {
        Vec3f pos;
        Vec3f norm;
        ColorQuad_t color;
    };
    std::vector<Vertex> m_vertices;
    std::vector<size_t> m_indexes;
};


/*
 *  Instance for ParticleRenderer
 */

static const size_t NUM_PARTICLE_COLORS = 5;

static const gfx::ColorQuad_t DEFAULT_PARTICLE_COLORS[NUM_PARTICLE_COLORS] = {
    COLORQUAD_FROM_RGBA(255, 255, 255, 255),
    COLORQUAD_FROM_RGBA(255, 255,   0, 255),
    COLORQUAD_FROM_RGBA(255, 128,   0, 255),
    COLORQUAD_FROM_RGBA(255,   0,   0, 128),
    COLORQUAD_FROM_RGBA(255,   0,   0,   0),
};

class gfx::threed::SoftwareContext::ParticleRendererInstance : public Instance {
 public:
    ParticleRendererInstance(const ColorQuad_t (&colors)[NUM_PARTICLE_COLORS])
        : m_particles()
        {
            afl::base::Memory<ColorQuad_t>(m_colors).copyFrom(colors);
        }
    virtual void renderPrimitive(const Rectangle& /*r*/, Canvas& can, Index_t index)
        {
            // This is a lo-fi implementation.
            // Instead of rendering solid balls, just some circles at each color.
            const Particle& t = m_particles[index];
            BaseContext ctx(can);
            for (int i = 0; i < int(NUM_PARTICLE_COLORS); ++i) {
                ColorQuad_t in[1] = {
                    COLORQUAD_FROM_RGB(RED_FROM_COLORQUAD(m_colors[i]),
                                       GREEN_FROM_COLORQUAD(m_colors[i]),
                                       BLUE_FROM_COLORQUAD(m_colors[i]))
                };
                Color_t out[1];
                can.encodeColors(in, out);

                ctx.setRawColor(out[0]);
                ctx.setAlpha(Alpha_t(ALPHA_FROM_COLORQUAD(m_colors[i]) * t.alpha));

                drawCircle(ctx, t.pos, (t.size-1)*i/4+1);
            }
        }

    Index_t add(const Point& a, float alpha, int size)
        {
            Particle t = {a, alpha, size};
            Index_t result = Index_t(m_particles.size());
            m_particles.push_back(t);
            return result;
        }

 private:
    ColorQuad_t m_colors[NUM_PARTICLE_COLORS];
    struct Particle {
        Point pos;
        float alpha;
        int size;
    };
    std::vector<Particle> m_particles;
};


/*
 *  Public interface for ParticleRenderer
 */

class gfx::threed::SoftwareContext::ParticleRendererImpl : public ParticleRenderer {
 public:
    ParticleRendererImpl(afl::base::Ref<SoftwareContext> parent)
        : m_parent(parent),
          m_particles(),
          m_xAxis(0.5, 0, 0),
          m_yAxis(0, 0.5, 0)
        {
            afl::base::Memory<ColorQuad_t>(m_colors).copyFrom(DEFAULT_PARTICLE_COLORS);
        }

    virtual void clear()
        {
            m_particles.clear();
        }

    virtual void add(Vec3f pos, float alpha)
        {
            Particle p = {pos, alpha};
            m_particles.push_back(p);
        }

    virtual void setAxes(Vec3f xa, Vec3f ya)
        {
            m_xAxis = xa*0.5;
            m_yAxis = ya*0.5;
        }

    virtual void setColors(afl::base::Memory<const ColorQuad_t> color)
        {
            afl::base::Memory<ColorQuad_t>(m_colors).copyFrom(color);
        }

    virtual void render(const Mat4f& proj, const Mat4f& modelView)
        {
            ParticleRendererInstance* instance = new ParticleRendererInstance(m_colors);
            Index_t instanceNr = m_parent->addNewInstance(instance);

            // Projection matrix
            const Mat4f m = proj * modelView;

            for (size_t i = 0, n = m_particles.size(); i < n; ++i) {
                // Determine center position
                const Particle& p = m_particles[i];
                Vec3f center = p.pos.transform(m);

                // Determine size as maximum axis
                Vec3f pl = (p.pos - m_xAxis).transform(m);
                Vec3f pr = (p.pos + m_xAxis).transform(m);
                Vec3f across = (pr - pl) * float(m_parent->m_viewport.getWidth());

                Vec3f pt = (p.pos - m_yAxis).transform(m);
                Vec3f pb = (p.pos + m_yAxis).transform(m);
                Vec3f down = (pt - pb) * float(m_parent->m_viewport.getHeight());

                int size = int(std::sqrt(std::min(across.dot(across), down.dot(down))) + 0.5);

                m_parent->addPrimitive(center(2), instanceNr,
                                       instance->add(convertCoordinates(m_parent->m_viewport, center), p.alpha, size));
            }
        }

 private:
    afl::base::Ref<SoftwareContext> m_parent;
    struct Particle {
        Vec3f pos;
        float alpha;
    };
    std::vector<Particle> m_particles;
    Vec3f m_xAxis;
    Vec3f m_yAxis;
    ColorQuad_t m_colors[NUM_PARTICLE_COLORS];
};


/*
 *  Comparison predicate for sort order
 */

class gfx::threed::SoftwareContext::ComparePrimitives {
 public:
    bool operator()(const Primitive& a, const Primitive& b)
        { return a.z > b.z; }
};


/*
 *  SoftwareContext - Public Class
 */

gfx::threed::SoftwareContext::SoftwareContext()
    : m_instances(),
      m_primitives(),
      m_viewport(),
      m_pCanvas()
{ }

gfx::threed::SoftwareContext::~SoftwareContext()
{ }

afl::base::Ref<gfx::threed::SoftwareContext>
gfx::threed::SoftwareContext::create()
{
    return *new SoftwareContext();
}

void
gfx::threed::SoftwareContext::start(const Rectangle& r, Canvas& can)
{
    m_instances.clear();
    m_primitives.clear();
    m_viewport = r;
    m_pCanvas = &can;
}

void
gfx::threed::SoftwareContext::finish()
{
    afl::except::checkAssertion(m_pCanvas != 0, "SoftwareContext::finish: no canvas");

    // Depth sorting
    std::sort(m_primitives.begin(), m_primitives.end(), ComparePrimitives());

    // Draw in order
    for (std::vector<Primitive>::iterator it = m_primitives.begin(), end = m_primitives.end(); it != end; ++it) {
        m_instances[it->instance]->renderPrimitive(m_viewport, *m_pCanvas, it->index);
    }
}

afl::base::Ref<gfx::threed::LineRenderer>
gfx::threed::SoftwareContext::createLineRenderer()
{
    return *new LineRendererImpl(*this);
}

afl::base::Ref<gfx::threed::TriangleRenderer>
gfx::threed::SoftwareContext::createTriangleRenderer()
{
    return *new TriangleRendererImpl(*this);
}

afl::base::Ref<gfx::threed::ParticleRenderer>
gfx::threed::SoftwareContext::createParticleRenderer()
{
    return *new ParticleRendererImpl(*this);
}

gfx::threed::SoftwareContext::Index_t
gfx::threed::SoftwareContext::addNewInstance(Instance* p)
{
    size_t nn = m_instances.size();
    Index_t n = static_cast<Index_t>(nn);
    if (n != nn) {
        throw std::runtime_error("Too many instances");
    }
    m_instances.pushBackNew(p);
    return n;
}

void
gfx::threed::SoftwareContext::addPrimitive(float z, Index_t instance, Index_t index)
{
    Primitive p;
    p.z = z;
    p.instance = instance;
    p.index = index;
    m_primitives.push_back(p);
}
