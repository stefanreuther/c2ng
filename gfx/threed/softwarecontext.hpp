/**
  *  \file gfx/threed/softwarecontext.hpp
  *  \brief Class gfx::threed::SoftwareContext
  */
#ifndef C2NG_GFX_THREED_SOFTWARECONTEXT_HPP
#define C2NG_GFX_THREED_SOFTWARECONTEXT_HPP

#include "afl/base/ref.hpp"
#include "afl/container/ptrvector.hpp"
#include "gfx/threed/context.hpp"

namespace gfx { namespace threed {

    /** Simple software renderer implementation.
        This implements a simple "minimum viable product" implementation of a 3D Context.
        It doesn't aim to be 100.0% feature-complete and pixel-perfect,
        but it should be good enough to implement a FLAK player.

        As of July 2021, this class has the following restrictions:

        - no Z buffer. Intersecting primitives will look wrong.
        - no interpolation of normals or colors for triangles, i.e. flat shading.
        - not optimized for speed. Still does a few 10000 primitives per second. */
    class SoftwareContext : public Context {
     public:
        /** Create SoftwareContext.
            \return new SoftwareContext */
        static afl::base::Ref<SoftwareContext> create();
        ~SoftwareContext();

        // Context implementation:
        virtual void start(const Rectangle& r, Canvas& can);
        virtual void finish();
        virtual afl::base::Ref<LineRenderer> createLineRenderer();
        virtual afl::base::Ref<TriangleRenderer> createTriangleRenderer();
        virtual afl::base::Ref<ParticleRenderer> createParticleRenderer();

     private:
        SoftwareContext();

        class Instance;
        class LineRendererInstance;
        class LineRendererImpl;
        class TriangleRendererInstance;
        class TriangleRendererImpl;
        class ParticleRendererInstance;
        class ParticleRendererImpl;
        class ComparePrimitives;

        typedef uint16_t Index_t;

        struct Primitive {
            float z;
            Index_t instance;
            Index_t index;
        };

        // Instances (for life-time management)
        afl::container::PtrVector<Instance> m_instances;

        // Primitives
        std::vector<Primitive> m_primitives;

        // Status
        Rectangle m_viewport;
        Canvas* m_pCanvas;

        Index_t addNewInstance(Instance* p);
        void addPrimitive(float z, Index_t instance, Index_t index);
    };

} }

#endif
