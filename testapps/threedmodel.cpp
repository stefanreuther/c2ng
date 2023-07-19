/**
  *  \file testapps/threedmodel.cpp
  *  \brief Minimal 3-D Test Program, using model loader
  */

#include <stdexcept>
#include <iostream>
#include "config.h"
#include "afl/base/countof.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/environment.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/canvas.hpp"
#include "gfx/complex.hpp"
#include "gfx/defaultfont.hpp"
#include "gfx/eventconsumer.hpp"
#include "gfx/font.hpp"
#include "gfx/threed/model.hpp"
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

using gfx::threed::Vec3f;
using gfx::threed::Mat4f;

namespace {
    static const gfx::ColorQuad_t BACKGROUND_COLORS[] = {
        COLORQUAD_FROM_RGB(0,0,40),
        COLORQUAD_FROM_RGB(0,0,0),
        COLORQUAD_FROM_RGB(32,32,32),
        COLORQUAD_FROM_RGB(96,96,96),
        COLORQUAD_FROM_RGB(255,255,255),
    };

    gfx::Point convertCoordinates(const gfx::Rectangle& area, const Vec3f& pos)
    {
        return gfx::Point(int((pos(0) + 1.0) * 0.5 * area.getWidth()  + area.getLeftX() + 0.5),
                          int((1.0 - pos(1)) * 0.5 * area.getHeight() + area.getTopY()  + 0.5));
    }

    String_t getPointName(uint16_t id)
    {
        switch (id) {
         case 1: return "engine";
         case 2: return "engine start";
         case 3: return "engine end";
         case 4: return "beam";
         case 5: return "beam start";
         case 6: return "beam end";
         case 7: return "bay";
         case 8: return "bay start";
         case 9: return "bay end";
         case 10: return "launcher";
         case 11: return "launcher start";
         case 12: return "launcher end";
         case 100: return "wildcard";
         case 101: return "alchemy";
         case 102: return "terraforming";
         case 103: return "hyperdrive";
         case 104: return "gravitonic";
         case 105: return "wormhole scanner";
         case 106: return "casino";
         case 107: return "anti-cloak";
         case 108: return "cloaking device";
         case 109: return "assault transporter";
         case 110: return "bioscanner";
         case 111: return "glory device";
         case 112: return "tractor beam";
         case 113: return "ramscoop";
         case 114: return "chunnel device";
         case 115: return "shield generator";
         case 116: return "bridge";
         case 117: return "cargo room";
         case 118: return "fuel tank";
         case 119: return "crew quarters";
         case 120: return "red light";
         case 121: return "green light";
         case 122: return "yellow light";
         case 123: return "white light";
         case 124: return "blue light";
         case 125: return "headlight";
         default: return afl::string::Format("%d", id);
        }
    }

    /*
     *  Application state
     */
    class App : public gfx::EventConsumer {
     public:
        App(gfx::Canvas& can, afl::base::Ref<gfx::threed::Context> ctx, gfx::threed::Model& model)
            : m_stop(false),
              m_canvas(can),
              m_projection(Mat4f::perspective(45 * util::PI / 180, double(can.getSize().getX()) / can.getSize().getY(), 0.1)),
              m_azimut(),
              m_height(),
              m_distance(6.0),
              m_backgroundColor(0),
              m_context(ctx),
              m_showModel(!false),
              m_showOutline(!true),
              m_showWireframe(false),
              m_showLabels(false),
              m_modelRenderer(ctx->createTriangleRenderer()),
              m_outlineRenderer(ctx->createLineRenderer()),
              m_wireframeRenderer(ctx->createLineRenderer()),
              m_posList(),
              m_font(gfx::createDefaultFont())
            {
                updateModel(model);
                draw();
            }

        void updateModel(gfx::threed::Model& model)
            {
                model.renderMesh(0, *m_modelRenderer);
                model.renderGrid(0, *m_outlineRenderer, COLORQUAD_FROM_RGB(192, 192, 192));
                model.renderGrid(1, *m_wireframeRenderer, COLORQUAD_FROM_RGB(192, 255, 192));
                m_posList = model.positions();
            }

        void draw()
            {
                // ModelView matrix:
                Mat4f mv(Mat4f::identity());
                mv.translate(Vec3f(0, 0, -float(m_distance)));
                mv.rotateX(m_height);
                mv.rotateZ(m_azimut);
                mv.scale(0.5);

                // Draw
                clear();
                m_context->start(getSize(), m_canvas);
                if (m_showModel) {
                    m_modelRenderer->render(m_projection, mv);
                }
                if (m_showOutline) {
                    m_outlineRenderer->render(m_projection, mv);
                }
                if (m_showWireframe) {
                    m_wireframeRenderer->render(m_projection, mv);
                }
                m_context->finish();

                // Labels
                if (m_showLabels) {
                    drawLabels(m_projection, mv);
                }
            }

        void clear()
            {
                // Clear the canvas with a predefined color
                gfx::ColorQuad_t quad[1] = {BACKGROUND_COLORS[m_backgroundColor]};
                gfx::Color_t color[1];
                m_canvas.encodeColors(quad, color);
                m_canvas.drawBar(getSize(), color[0], color[0], gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
            }

        void drawLabels(const Mat4f& proj, const Mat4f& mv)
            {
                gfx::ColorQuad_t quad[1] = {COLORQUAD_FROM_RGB(255,255,255)};
                gfx::Color_t color[1];
                m_canvas.encodeColors(quad, color);

                gfx::BaseContext ctx(m_canvas);
                ctx.setRawColor(color[0]);
                ctx.useFont(*m_font);
                ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);

                gfx::Rectangle size = getSize();
                for (size_t i = 0; i < m_posList.getNumPositions(); ++i) {
                    const Vec3f pos = m_posList.getPositionByIndex(i).transform(mv).transform(proj);
                    gfx::Point pt = convertCoordinates(size, pos);

                    drawHLine(ctx, pt.getX()-3, pt.getY(), pt.getX() + 3);
                    drawVLine(ctx, pt.getX(), pt.getY() - 3, pt.getY() + 3);
                    outText(ctx, pt + gfx::Point(5, 0), getPointName(m_posList.getIdByIndex(i)));
                }
            }

        gfx::Rectangle getSize()
            {
                // Shortcut for getting canvas size
                return gfx::Rectangle(gfx::Point(), m_canvas.getSize());
            }

        virtual bool handleKey(util::Key_t key, int /*prefix*/)
            {
                switch (key) {
                 case 'm':
                    m_showModel = !m_showModel;
                    draw();
                    return true;

                 case 'o':
                    m_showOutline = !m_showOutline;
                    draw();
                    return true;

                 case 'w':
                    m_showWireframe = !m_showWireframe;
                    draw();
                    return true;

                 case 'l':
                    m_showLabels = !m_showLabels;
                    draw();
                    return true;

                 case 'b':
                    m_backgroundColor = (m_backgroundColor+1) % countof(BACKGROUND_COLORS);
                    draw();
                    return true;

                 case 'a':
                    m_distance -= 0.1;
                    draw();
                    return true;

                 case 'z':
                 case 'y':
                    m_distance += 0.1;
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
        double m_distance;
        size_t m_backgroundColor;

        afl::base::Ref<gfx::threed::Context> m_context;

        bool m_showModel;
        bool m_showOutline;
        bool m_showWireframe;
        bool m_showLabels;

        afl::base::Ref<gfx::threed::TriangleRenderer> m_modelRenderer;
        afl::base::Ref<gfx::threed::LineRenderer> m_outlineRenderer;
        afl::base::Ref<gfx::threed::LineRenderer> m_wireframeRenderer;
        gfx::threed::PositionList m_posList;
        afl::base::Ref<gfx::Font> m_font;
    };
}



int main(int, char** argv)
{
    try {
        // Parameters
        afl::string::NullTranslator tx;
        afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
        String_t fileName;
        if (!env.getCommandLine()->getNextElement(fileName)) {
            std::cerr << "Need model file name.\n";
            return 1;
        }

        afl::base::Ref<gfx::threed::Model> model = gfx::threed::Model::create();
        model->load(*afl::io::FileSystem::getInstance().openFile(fileName, afl::io::FileSystem::OpenRead), tx);

        // Graphics engine
        util::ConsoleLogger log;
        Engine_t engine(log, tx);

        // Window
        afl::base::Ref<gfx::Canvas> window = engine.createWindow(gfx::WindowParameters());

        // 3D context
        afl::base::Ref<gfx::threed::Context> ctx = gfx::threed::SoftwareContext::create();

        // App main loop
        App c(*window, ctx, *model);
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
