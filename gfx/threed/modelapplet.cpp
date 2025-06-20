/**
  *  \file gfx/threed/modelapplet.cpp
  *  \brief gfx::threed::ModelApplet
  */

#include "gfx/threed/modelapplet.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/format.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/complex.hpp"
#include "gfx/defaultfont.hpp"
#include "gfx/eventconsumer.hpp"
#include "gfx/font.hpp"
#include "gfx/point.hpp"
#include "gfx/threed/colortransformation.hpp"
#include "gfx/threed/context.hpp"
#include "gfx/threed/model.hpp"
#include "gfx/threed/positionlist.hpp"
#include "gfx/threed/softwarecontext.hpp"
#include "gfx/threed/vecmath.hpp"
#include "gfx/types.hpp"
#include "gfx/windowparameters.hpp"
#include "util/math.hpp"

using afl::base::Ref;
using afl::io::FileSystem;
using afl::string::Format;
using gfx::threed::Vec3f;

namespace {
    static const gfx::ColorQuad_t BACKGROUND_COLORS[] = {
        COLORQUAD_FROM_RGB(0,0,40),
        COLORQUAD_FROM_RGB(0,0,0),
        COLORQUAD_FROM_RGB(32,32,32),
        COLORQUAD_FROM_RGB(96,96,96),
        COLORQUAD_FROM_RGB(255,255,255),
    };

    static const gfx::ColorQuad_t PLAYER_COLORS[] = {
        0,
        COLORQUAD_FROM_RGB(128,128,150),
        COLORQUAD_FROM_RGB(255,255,255),
        COLORQUAD_FROM_RGB(255,255,0),
        COLORQUAD_FROM_RGB(97,242,97),
        COLORQUAD_FROM_RGB(97,97,194),
        COLORQUAD_FROM_RGB(255,0,0),
        COLORQUAD_FROM_RGB(255,85,255),
        COLORQUAD_FROM_RGB(194,97,0),
        COLORQUAD_FROM_RGB(255,194,0),
        COLORQUAD_FROM_RGB(85,255,255),
        COLORQUAD_FROM_RGB(0,170,0),
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
         default: return Format("%d", id);
        }
    }
}

/*
 *  Application state
 */
class gfx::threed::ModelApplet::App : public EventConsumer {
 public:
    App(Canvas& can, Ref<Context> ctx, Model& model)
        : m_stop(false),
          m_canvas(can),
          m_model(model),
          m_projection(Mat4f::perspective(45 * util::PI / 180, double(can.getSize().getX()) / can.getSize().getY(), 0.1)),
          m_azimut(),
          m_height(),
          m_distance(6.0),
          m_backgroundColor(0),
          m_playerColor(0),
          m_context(ctx),
          m_showModel(!false),
          m_showOutline(!true),
          m_showWireframe(false),
          m_showLabels(false),
          m_modelRenderer(ctx->createTriangleRenderer()),
          m_outlineRenderer(ctx->createLineRenderer()),
          m_wireframeRenderer(ctx->createLineRenderer()),
          m_posList(),
          m_font(createDefaultFont())
        {
            updateModel();
            draw();
        }

    void updateModel()
        {
            updateMesh();
            m_model.renderGrid(0, *m_outlineRenderer, COLORQUAD_FROM_RGB(192, 192, 192));
            m_model.renderGrid(1, *m_wireframeRenderer, COLORQUAD_FROM_RGB(192, 255, 192));
            m_posList = m_model.positions();
        }

    void updateMesh()
        {
            m_modelRenderer->clear();
            if (m_playerColor == 0) {
                m_model.renderMesh(0, *m_modelRenderer);
            } else {
                ColorTransformation dim = ColorTransformation::identity().scale(0.3f);
                ColorTransformation gray = ColorTransformation::toGrayscale(PLAYER_COLORS[m_playerColor]).scale(0.7f);
                m_model.renderMesh(0, *m_modelRenderer, dim + gray);
            }
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
            ColorQuad_t quad[1] = {BACKGROUND_COLORS[m_backgroundColor]};
            Color_t color[1];
            m_canvas.encodeColors(quad, color);
            m_canvas.drawBar(getSize(), color[0], color[0], FillPattern::SOLID, OPAQUE_ALPHA);
        }

    void drawLabels(const Mat4f& proj, const Mat4f& mv)
        {
            ColorQuad_t quad[1] = {COLORQUAD_FROM_RGB(255,255,255)};
            Color_t color[1];
            m_canvas.encodeColors(quad, color);

            BaseContext ctx(m_canvas);
            ctx.setRawColor(color[0]);
            ctx.useFont(*m_font);
            ctx.setTextAlign(LeftAlign, MiddleAlign);

            Rectangle size = getSize();
            for (size_t i = 0; i < m_posList.getNumPositions(); ++i) {
                const Vec3f pos = m_posList.getPositionByIndex(i).transform(mv).transform(proj);
                Point pt = convertCoordinates(size, pos);

                drawHLine(ctx, pt.getX()-3, pt.getY(), pt.getX() + 3);
                drawVLine(ctx, pt.getX(), pt.getY() - 3, pt.getY() + 3);
                outText(ctx, pt + Point(5, 0), getPointName(m_posList.getIdByIndex(i)));
            }
        }

    Rectangle getSize()
        {
            // Shortcut for getting canvas size
            return Rectangle(Point(), m_canvas.getSize());
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

             case 'p':
                m_playerColor = (m_playerColor+1) % countof(PLAYER_COLORS);
                updateMesh();
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

    virtual bool handleMouse(Point /*pt*/, MouseButtons_t /*pressedButtons*/)
        { return false; }

    bool isStopped() const
        { return m_stop; }

 private:
    bool m_stop;
    Canvas& m_canvas;
    Model& m_model;

    Mat4f m_projection;
    double m_azimut;
    double m_height;
    double m_distance;
    size_t m_backgroundColor;
    size_t m_playerColor;

    Ref<Context> m_context;

    bool m_showModel;
    bool m_showOutline;
    bool m_showWireframe;
    bool m_showLabels;

    Ref<TriangleRenderer> m_modelRenderer;
    Ref<LineRenderer> m_outlineRenderer;
    Ref<LineRenderer> m_wireframeRenderer;
    PositionList m_posList;
    Ref<Font> m_font;
};

int
gfx::threed::ModelApplet::run(Application& app, Engine& engine, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::sys::Environment::CommandLine_t& cmdl)
{
    // Parameters
    afl::string::Translator& tx = app.translator();
    String_t fileName;
    if (!cmdl.getNextElement(fileName)) {
        app.dialog().showError("Need model file name.", env.getInvocationName());
        return 1;
    }

    Ref<Model> model = Model::create();
    model->load(*fs.openFile(fileName, FileSystem::OpenRead), tx);

    // Window
    Ref<Canvas> window = engine.createWindow(WindowParameters());

    // 3D context
    Ref<Context> ctx = SoftwareContext::create();

    // App main loop
    App c(*window, ctx, *model);
    while (!c.isStopped()) {
        engine.handleEvent(c, false);
    }
    return 0;
}
