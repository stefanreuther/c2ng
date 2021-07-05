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
        return f > 255 ? 255 : f < 0 ? 0 : int(f);
    }

    gfx::ColorQuad_t makeColor(gfx::ColorQuad_t color,
                               gfx::threed::Vec3f lighting)
    {
        float r = RED_FROM_COLORQUAD(color) * lighting(0);
        float g = GREEN_FROM_COLORQUAD(color) * lighting(1);
        float b = BLUE_FROM_COLORQUAD(color) * lighting(2);
        return COLORQUAD_FROM_RGB(clampComponent(r),
                                  clampComponent(g),
                                  clampComponent(b));
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
            Index_t instanceNr = m_parent->addNewInstance(instance);

            Mat4f m = proj * modelView;
            for (size_t i = 0, n = m_lines.size(); i < n; ++i) {
                Vec3f from = m_lines[i].from.transform(m);
                Vec3f to = m_lines[i].to.transform(m);
                // FIXME: if distance is too large, split.
                // FIXME: remove line segments that are entirely outside the frustum.
                m_parent->addPrimitive((from(2) + to(2)) * 0.5f,      // z
                                       instanceNr,
                                       instance->add(convertCoordinates(m_parent->m_viewport, from),
                                                     convertCoordinates(m_parent->m_viewport, to),
                                                     m_lines[i].color));
            }
        }

 private:
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
                Vec3f amod = a.pos.transform(modelView);
                Vec3f bmod = b.pos.transform(modelView);
                Vec3f cmod = c.pos.transform(modelView);
                Vec3f aproj = amod.transform(proj);
                Vec3f bproj = bmod.transform(proj);
                Vec3f cproj = cmod.transform(proj);

                // Visibility (back-face) check
                const Vec3f vec = (bproj - aproj).prod(cproj - aproj);
                if (vec(2) < 0) {
                    continue;
                }

                // Determine Z position for sorting
                float z = (aproj(2) + bproj(2) + cproj(2)) * (1.0f/3.0f);

                // Determine color. Flat shading for now, i.e. all the same color.
                const Vec3f norm = (bmod - amod).prod(cmod - amod).norm();
                float directional = std::max(norm.dot(directionalVector), 0.0f);
                Vec3f lighting = ambientLight + directionalLightColor * directional;

                m_parent->addPrimitive(z, instanceNr,
                                       instance->add(convertCoordinates(m_parent->m_viewport, aproj),
                                                     convertCoordinates(m_parent->m_viewport, bproj),
                                                     convertCoordinates(m_parent->m_viewport, cproj),
                                                     makeColor(a.color, lighting)));
            }
        }

 private:
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
