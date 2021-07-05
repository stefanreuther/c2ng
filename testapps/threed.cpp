/**
  *  \file testapps/threed.cpp
  *  \brief Minimal 3-D Test Program
  */

#include <stdexcept>
#include <iostream>
#include "config.h"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/nulltranslator.hpp"
#include "gfx/canvas.hpp"
#include "gfx/eventconsumer.hpp"
#include "gfx/threed/softwarecontext.hpp"
#include "gfx/threed/vecmath.hpp"
#include "gfx/windowparameters.hpp"
#include "util/consolelogger.hpp"
#include "util/math.hpp"
#include "util/randomnumbergenerator.hpp"

#ifdef HAVE_SDL
# include "gfx/sdl/engine.hpp"
typedef gfx::sdl::Engine Engine_t;
#elif defined(HAVE_SDL2)
# include "gfx/sdl2/engine.hpp"
typedef gfx::sdl2::Engine Engine_t;
#else
# error "foo"
#endif

// #define USE_LINES

using gfx::threed::Vec3f;
using gfx::threed::Mat4f;

namespace {
    /*
     *  Application state
     */
    class App : public gfx::EventConsumer {
     public:
        App(gfx::Canvas& can, afl::base::Ref<gfx::threed::Context> ctx)
            : m_stop(false),
              m_canvas(can),
              m_projection(Mat4f::perspective(45 * util::PI / 180, double(can.getSize().getX()) / can.getSize().getY(), 0.1)),
              m_azimut(),
              m_height(),
              m_context(ctx),
              m_particleRenderer(m_context->createParticleRenderer()),
              m_particles(),
              m_rng(0),
#ifdef USE_LINES
              m_renderer(m_context->createLineRenderer())
#else
              m_renderer(m_context->createTriangleRenderer())
#endif
            {
                // Two cylinders
                // m_renderer->addCylinder(Vec3f(0, 0, 0), Vec3f(2, 3, 4),  2, COLORQUAD_FROM_RGB(255,   0,  0), 40);
                // m_renderer->addCylinder(Vec3f(0, 0, 0), Vec3f(-2,-3,-4), 1, COLORQUAD_FROM_RGB(128, 102, 51), 20);

                // Four spheres in different colors
                m_renderer->addSphere(Vec3f( 0,  3, 0), 2, COLORQUAD_FROM_RGB(255, 0, 0), 30);
                m_renderer->addSphere(Vec3f( 3,  0, 0), 2, COLORQUAD_FROM_RGB(0, 255, 0), 30);
                m_renderer->addSphere(Vec3f( 0, -3, 0), 2, COLORQUAD_FROM_RGB(0, 0, 255), 30);
                m_renderer->addSphere(Vec3f(-3,  0, 0), 2, COLORQUAD_FROM_RGB(255, 255, 0), 30);
                draw();
            }

        void draw()
            {
                // ModelView matrix:
                Mat4f mv(Mat4f::identity());
                mv.translate(Vec3f(0, 0, -6));
                mv.rotateZ(m_azimut);
                mv.rotateX(m_height);
                mv.scale(0.5);

                renderParticles();

                // Draw
                clear();
                m_context->start(getSize(), m_canvas);
                m_renderer->render(m_projection, mv);
                m_particleRenderer->render(m_projection, mv);
                m_context->finish();
            }

        void clear()
            {
                // Clear the canvas with a predefined color
                gfx::ColorQuad_t quad[1] = {COLORQUAD_FROM_RGB(0,0,40)};
                gfx::Color_t color[1];
                m_canvas.encodeColors(quad, color);
                m_canvas.drawBar(getSize(), color[0], color[0], gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
            }

        gfx::Rectangle getSize()
            {
                // Shortcut for getting canvas size
                return gfx::Rectangle(gfx::Point(), m_canvas.getSize());
            }

        virtual bool handleKey(util::Key_t key, int /*prefix*/)
            {
                switch (key) {
                 case ' ':
                    updateParticles();
                    draw();
                    return true;

                 case 'q':
                 case util::Key_Escape:
                    m_stop = true;
                    return true;

                 case util::Key_Left:
                    m_azimut -= 0.1;
                    draw();
                    return true;

                 case util::Key_Right:
                    m_azimut += 0.1;
                    draw();
                    return true;

                 case util::Key_Down:
                    m_height += 0.1;
                    draw();
                    return true;

                 case util::Key_Up:
                    m_height -= 0.1;
                    draw();
                    return true;
                }
                return false;
            }

        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
            { return false; }

        bool isStopped() const
            { return m_stop; }

     private:
        bool m_stop;
        gfx::Canvas& m_canvas;

        Mat4f m_projection;
        double m_azimut;
        double m_height;

        afl::base::Ref<gfx::threed::Context> m_context;

        struct Particle {
            float x, y, dx, dy;
            int age;
        };
        afl::base::Ref<gfx::threed::ParticleRenderer> m_particleRenderer;
        std::vector<Particle> m_particles;
        util::RandomNumberGenerator m_rng;

#ifdef USE_LINES
        afl::base::Ref<gfx::threed::LineRenderer> m_renderer;
#else
        afl::base::Ref<gfx::threed::TriangleRenderer> m_renderer;
#endif

        void updateParticles()
            {
                // New particles
                while (m_particles.size() < 30) {
                    Particle p = {
                        m_rng() / 65536.0f,
                        m_rng() / 65536.0f - 2.0f,
                        m_rng() / 32768.0f * 2.0f - 1,
                        m_rng() / 16384.0f,
                        0
                    };
                    m_particles.push_back(p);
                }

                // Update
                size_t out = 0;
                for (size_t in = 0; in < m_particles.size(); ++in) {
                    Particle& p = m_particles[in];
                    if (p.age > 80 || p.y < -4 || p.x < -3 || p.x > 3) {
                        // skip
                    } else {
                        p.x += p.dx*0.1f;
                        p.y += p.dy*0.1f;
                        p.dy -= 0.1f;
                        ++p.age;
                        m_particles[out++] = p;
                    }
                }
                m_particles.resize(out);
            }

        void renderParticles()
            {
                // Build the particles
                Mat4f rotationMatrix = Mat4f::identity();
                rotationMatrix.rotateX(-m_height);
                rotationMatrix.rotateZ(-m_azimut);
                m_particleRenderer->setAxes(Vec3f(0.1f, 0, 0).transform(rotationMatrix),
                                            Vec3f(0, 0.1f, 0).transform(rotationMatrix));

                static const gfx::ColorQuad_t colors[] = {
                    COLORQUAD_FROM_RGBA(0, 128, 255, 255),
                    COLORQUAD_FROM_RGBA(0, 128, 255, 255),
                    COLORQUAD_FROM_RGBA(0,  64, 255, 255),
                    COLORQUAD_FROM_RGBA(0,   0, 255, 128),
                    COLORQUAD_FROM_RGBA(0,   0, 255,   0)
                };
                m_particleRenderer->setColors(colors);

                // Sort to draw farthest first -> required in JS/WebGL version.
                // // Sort matrix is part of regular projection, but we only need the Z component.
                // var sortMatrix = mMakeIdentity();
                // mRotateZ(sortMatrix, azimut);
                // mRotateX(sortMatrix, height);
                // function zIndex(p) {
                //     return p.x * sortMatrix[2] + p.y * sortMatrix[6];
                //     // return -vTransform([p.x,p.y,0], sortMatrix)[2];
                // }
                // particles.sort(function(a, b) {
                //     return zIndex(a) - zIndex(b);
                // });

                // Place in ParticleRenderer
                m_particleRenderer->clear();
                for (size_t i = 0; i < m_particles.size(); ++i) {
                    const Particle& p = m_particles[i];
                    float alpha = std::min(1.0f, float(1.0 - p.age / 80.0)) * 0.5f;
                    m_particleRenderer->add(Vec3f(p.x, 0, p.y), alpha);
                }
            }
    };
}



int main(int, char** /*argv*/)
{
    try {
        // Graphics engine
        afl::string::NullTranslator tx;
        util::ConsoleLogger log;
        Engine_t engine(log, tx);

        // Window
        afl::base::Ref<gfx::Canvas> window = engine.createWindow(gfx::WindowParameters());

        // 3D context
        afl::base::Ref<gfx::threed::Context> ctx = gfx::threed::SoftwareContext::create();

        // App main loop
        App c(*window, ctx);
        while (!c.isStopped()) {
            engine.handleEvent(c, false);
        }
    }
    catch (afl::except::FileProblemException& e) {
        std::cout << "exception: " << e.getFileName() << ": " << e.what() << "\n";
    }
    catch (std::exception& e) {
        std::cout << "exception: " << e.what() << "\n";
    }
}
