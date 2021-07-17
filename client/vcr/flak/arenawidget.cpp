/**
  *  \file client/vcr/flak/arenawidget.cpp
  */

#include "client/vcr/flak/arenawidget.hpp"
#include "gfx/threed/softwarecontext.hpp"
#include "gfx/threed/vecmath.hpp"
#include "util/math.hpp"

using game::vcr::flak::Position;
using game::vcr::flak::VisualisationState;
using gfx::ColorQuad_t;
using gfx::threed::Mat4f;
using gfx::threed::ParticleRenderer;
using gfx::threed::TriangleRenderer;
using gfx::threed::Vec3f;

namespace {
    const double RAISE_UNIT = 10000;
    const int MAX_SMOKE_AGE = 10;

    // Generic ship: two pyramids
    void buildGenericShip(TriangleRenderer& ren, float scale, ColorQuad_t color)
    {
        // ex buildGenericShip(ren, scale, color)
        const float W = 0.5f*scale, H = 1.25f*scale;
        const Vec3f
            R1(-H, -W, -W),
            R2(-H,  W, -W),
            R3(-H,  W,  W),
            R4(-H, -W,  W),
            F(H, 0,0),
            A(-H-0.25f*scale, 0, 0);
        ren.addTriangle(R1, R2, F, color);
        ren.addTriangle(R2, R3, F, color);
        ren.addTriangle(R3, R4, F, color);
        ren.addTriangle(R4, R1, F, color);

        ren.addTriangle(R2, R1, A, color);
        ren.addTriangle(R3, R2, A, color);
        ren.addTriangle(R4, R3, A, color);
        ren.addTriangle(R1, R4, A, color);
    }

    // // Same as buildGenericShip, but builds a wireframe
    // function makeWireframeShip(gl, color) {
    //     var ren = new WebGL.LineRenderer(gl);
    //     var W = 0.5, H = 1.25;
    //     var R1 = [-H, -W, -W],
    //         R2 = [-H,  W, -W],
    //         R3 = [-H,  W,  W],
    //         R4 = [-H, -W,  W],
    //         F = [H, 0,0],
    //         A = [-H-0.25, 0,0];
    //     ren.add(R1, R2, color);
    //     ren.add(R2, R3, color);
    //     ren.add(R3, R4, color);
    //     ren.add(R4, R1, color);
    //     ren.add(R1, F, color);
    //     ren.add(R2, F, color);
    //     ren.add(R3, F, color);
    //     ren.add(R4, F, color);
    //     ren.add(R1, A, color);
    //     ren.add(R2, A, color);
    //     ren.add(R3, A, color);
    //     ren.add(R4, A, color);
    //     return ren;
    // }

    // // Make ship for a player
    // function makeShip(gl, owner) {
    //     var ren = new WebGL.TriangleRenderer(gl);
    //     var color = getPlayerColor(owner);
    //     buildGenericShip(ren, 1.0, scaleColor(mixColor(color, [0.5, 0.5, 0.5], 0.5), 0.5));
    //     return ren;
    // }

    // function makeWireframePlanet(gl, color) {
    //     var ren = new WebGL.LineRenderer(gl);
    //     ren.addSphere([0,0,0], 1, {color:scaleColor(mixColor(color, [0.5, 0.5, 0.5], 0.5), 0.5), n:10});
    //     return ren;
    // }

    void makePlanet(TriangleRenderer& ren, ColorQuad_t color)
    {
        ren.addSphere(Vec3f(0,0,0), 1, color, 10);
    }

    // // Make planet for a player
    // function makePlanet(gl, owner) {
    //     var ren = new WebGL.TriangleRenderer(gl);
    //     var color = getPlayerColor(owner);
    //     ren.addSphere([0,0,0], 1, {color:scaleColor(mixColor(color, [0.5, 0.5, 0.5], 0.5), 0.5), n:10});
    //     return ren;
    // }

    void makeFighter(TriangleRenderer& ren, ColorQuad_t color)
    {
        buildGenericShip(ren, 0.5f, color);
    }

    // // Make fighter for a player
    // function makeFighter(gl, owner) {
    //     var ren = new WebGL.TriangleRenderer(gl);
    //     var color = getPlayerColor(owner);
    //     buildGenericShip(ren, 0.5, scaleColor(mixColor(color, [0.5, 0.5, 0.75], 0.5), 0.5));
    //     return ren;
    // }

    // Make torpedo: 6-point star
    void makeTorpedo(TriangleRenderer& torpedo)
    {
        // ex makeTorpedo(gl)
        const float W = 0.15f, H = 1.5f;
        const ColorQuad_t C = COLORQUAD_FROM_RGB(255,0,0);

        // Pointing right
        torpedo.addTriangle(Vec3f(-W, -W, W), Vec3f( W, -W, W), Vec3f(0, 0, H), C);
        torpedo.addTriangle(Vec3f( W, -W, W), Vec3f( W,  W, W), Vec3f(0, 0, H), C);
        torpedo.addTriangle(Vec3f( W,  W, W), Vec3f(-W,  W, W), Vec3f(0, 0, H), C);
        torpedo.addTriangle(Vec3f(-W,  W, W), Vec3f(-W, -W, W), Vec3f(0, 0, H), C);

        // Pointing left
        torpedo.addTriangle(Vec3f( W, -W, -W), Vec3f(-W, -W, -W), Vec3f(0, 0, -H), C);
        torpedo.addTriangle(Vec3f( W,  W, -W), Vec3f( W, -W, -W), Vec3f(0, 0, -H), C);
        torpedo.addTriangle(Vec3f(-W,  W, -W), Vec3f( W,  W, -W), Vec3f(0, 0, -H), C);
        torpedo.addTriangle(Vec3f(-W, -W, -W), Vec3f(-W,  W, -W), Vec3f(0, 0, -H), C);

        // Pointing inward
        torpedo.addTriangle(Vec3f(-W, -W, -W), Vec3f( W, -W, -W), Vec3f(0, -H, 0), C);
        torpedo.addTriangle(Vec3f( W, -W, -W), Vec3f( W, -W,  W), Vec3f(0, -H, 0), C);
        torpedo.addTriangle(Vec3f( W, -W,  W), Vec3f(-W, -W,  W), Vec3f(0, -H, 0), C);
        torpedo.addTriangle(Vec3f(-W, -W,  W), Vec3f(-W, -W, -W), Vec3f(0, -H, 0), C);

        // Pointing outward
        torpedo.addTriangle(Vec3f( W, W, -W), Vec3f(-W, W, -W), Vec3f(0, H, 0), C);
        torpedo.addTriangle(Vec3f( W, W,  W), Vec3f( W, W, -W), Vec3f(0, H, 0), C);
        torpedo.addTriangle(Vec3f(-W, W,  W), Vec3f( W, W,  W), Vec3f(0, H, 0), C);
        torpedo.addTriangle(Vec3f(-W, W, -W), Vec3f(-W, W,  W), Vec3f(0, H, 0), C);

        // Pointing down
        torpedo.addTriangle(Vec3f(-W,  W, -W), Vec3f(-W, -W, -W), Vec3f(-H, 0, 0), C);
        torpedo.addTriangle(Vec3f(-W,  W,  W), Vec3f(-W,  W, -W), Vec3f(-H, 0, 0), C);
        torpedo.addTriangle(Vec3f(-W, -W,  W), Vec3f(-W,  W,  W), Vec3f(-H, 0, 0), C);
        torpedo.addTriangle(Vec3f(-W, -W, -W), Vec3f(-W, -W,  W), Vec3f(-H, 0, 0), C);

        // Pointing up
        torpedo.addTriangle(Vec3f(W, -W, -W), Vec3f(W,  W, -W), Vec3f(H, 0, 0), C);
        torpedo.addTriangle(Vec3f(W,  W, -W), Vec3f(W,  W,  W), Vec3f(H, 0, 0), C);
        torpedo.addTriangle(Vec3f(W,  W,  W), Vec3f(W, -W,  W), Vec3f(H, 0, 0), C);
        torpedo.addTriangle(Vec3f(W, -W,  W), Vec3f(W, -W, -W), Vec3f(H, 0, 0), C);
    }

    void renderModel(const Mat4f& proj, const Mat4f& mvm, TriangleRenderer& model, const Position& pos, float rotX, float rotZ, float modelScale)
    {
        // ex renderModel(fv, proj, mvm, model, x, y, z, rotX, rotZ, modelScale)
        // mvm already includes the Camera and World-to-OpenGL transformation
        Mat4f modelViewMatrix = mvm;

        // Move to world location
        modelViewMatrix.translate(Vec3f(float(pos.x), float(pos.y), float(pos.z)));

        // Rotate (model looks down)
        if (rotZ) {
            modelViewMatrix.rotateZ(rotZ);
        }
        if (rotX) {
            modelViewMatrix.rotateX(rotX);
        }

        // Scale model to world size
        modelViewMatrix.scale(modelScale);

        // Render
        model.render(proj, modelViewMatrix);
    }

    void renderSmoke(afl::base::Memory<const VisualisationState::Smoke> smoke, const Mat4f& proj, const Mat4f& mvm, ParticleRenderer& ren, float height, float azimut)
    {
        // ex renderSmoke(fv, smoke, proj, mvm, height, azimut)
        // Renderer
        Mat4f rotationMatrix = Mat4f::identity();
        rotationMatrix.rotateZ(-azimut);
        rotationMatrix.rotateX(-height);
        ren.setAxes(Vec3f(500, 0, 0).transform(rotationMatrix),
                    Vec3f(0, 500, 0).transform(rotationMatrix));

        // FIXME: Sort particles by Z index to draw farthest first - not needed for SW renderer!
        // var sortMatrix = WM.mMakeIdentity();
        // WM.mRotateX(sortMatrix, height);
        // WM.mRotateZ(sortMatrix, azimut);
        // function zIndex(p) {
        //     return p.x * sortMatrix[2] + p.y * sortMatrix[6] + p.z * sortMatrix[10];
        // }
        // smoke.sort(function(a, b) {
        //     return zIndex(a) - zIndex(b);
        // });

        // Place in ParticleRenderer
        ren.clear();
        while (const VisualisationState::Smoke* p = smoke.eat()) {
            float a = std::min(1.0f, 1.0f - float(p->age)/MAX_SMOKE_AGE)*0.5f;
            ren.add(Vec3f(float(p->pos.x), float(p->pos.y), float(p->pos.z)), a);
        }

        ren.render(proj, mvm);
    }
}


client::vcr::flak::ArenaWidget::ArenaWidget(ui::Root& root,
                                            game::vcr::flak::VisualisationState& state,
                                            game::vcr::flak::VisualisationSettings& settings)
    : SimpleWidget(),
      m_root(root),
      m_state(state),
      m_settings(settings),
      m_context(gfx::threed::SoftwareContext::create()),
      m_smokeRenderer(m_context->createParticleRenderer()),
      m_shipModel(m_context->createTriangleRenderer()),
      m_planetModel(m_context->createTriangleRenderer()),
      m_torpedoModel(m_context->createTriangleRenderer()),
      m_fighterModel(m_context->createTriangleRenderer())
{
    buildGenericShip(*m_shipModel, 1.0f, COLORQUAD_FROM_RGB(128, 128, 128));
    makePlanet(*m_planetModel, COLORQUAD_FROM_RGB(0, 128, 128));
    makeTorpedo(*m_torpedoModel);
    makeFighter(*m_fighterModel, COLORQUAD_FROM_RGB(128, 128, 128));

    static const ColorQuad_t SMOKE_COLORS[] = {
        COLORQUAD_FROM_RGBA(255, 255, 255, 255),
        COLORQUAD_FROM_RGBA(255, 192,   0, 255),
        COLORQUAD_FROM_RGBA(255, 128,   0, 255),
        COLORQUAD_FROM_RGBA(128,  64,   0, 128),
        COLORQUAD_FROM_RGBA(  0,   0, 255,   0),
    };
    m_smokeRenderer->setColors(SMOKE_COLORS);
}

void
client::vcr::flak::ArenaWidget::draw(gfx::Canvas& can)
{
    // ex fvRender(fv)
    const gfx::Rectangle& area = getExtent();
    can.drawBar(area, m_root.colorScheme().getColor(0), 0, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);

    // Render current state
    const double azimut = m_settings.getCameraAzimuth(), height = m_settings.getCameraHeight() + util::PI/2;

    Mat4f proj = Mat4f::perspective(45 * util::PI/180, double(area.getWidth()) / area.getHeight(), 0.1);
    // FIXME: var projSky = WM.mClone(proj);
    proj.translate(Vec3f(0, float(m_settings.getCameraRaise() / RAISE_UNIT), 0));

    Mat4f mvm = Mat4f::identity();

    // Camera
    mvm.translate(Vec3f(0.0, 0.0, -6.0));
    mvm.rotateX(height);
    mvm.rotateZ(azimut);

    // Scale world into OpenGL size
    float scale = float(2.0 / m_settings.getCameraDistance());
    mvm.scale(scale);

    // Start drawing
    m_context->start(area, can);

    // FIXME: Skybox
    // if (fv._sky) {
    //     fv._sky.render(projSky, mvm);
    // }

    // Smoke
    renderSmoke(m_state.smoke(), proj, mvm, *m_smokeRenderer, m_settings.getCameraHeight(), m_settings.getCameraAzimuth());

    // Ships and Planets
    afl::base::Memory<const VisualisationState::Ship> ships = m_state.ships();
    while (const VisualisationState::Ship* sh = ships.eat()) {
        if (sh->isPlanet) {
            // Render planet even if it's dead.
            if (sh->isAlive) {
                renderModel(proj, mvm, *m_planetModel /*FIXME: fv._planetModels[sh._player]*/, sh->pos, 0, 0, 1000);
            } else {
                renderModel(proj, mvm, *m_planetModel /*FIXME: fv._deadPlanetModel*/, sh->pos, 0, 0, 1000);
            }
        } else {
            if (sh->isAlive) {
                renderModel(proj, mvm, *m_shipModel /*FIXME:fv._shipModels[sh._player]*/, sh->pos, float(util::PI/2), sh->heading, 1000);
            }
        }
    }

    // Fighters and Torpedoes
    afl::base::Memory<const VisualisationState::Object> objects = m_state.objects();
    while (const VisualisationState::Object* e = objects.eat()) {
        switch (e->type) {
         case VisualisationState::TorpedoObject:
            renderModel(proj, mvm, *m_torpedoModel, e->pos, float(e->xRotation * 1.0/256), float(e->yRotation * 1.0/256), 300);
            break;
         case VisualisationState::FighterObject:
            renderModel(proj, mvm, *m_fighterModel /*FIXME: fv._fighterModels[e._player]*/, e->pos, float(util::PI/2), e->heading, 500);
            break;
         case VisualisationState::NoObject:
            break;
        }
    }

    //     // Active beams
    //     if (fv._beams.length) {
    //         renderBeams(fv, proj, mvm, fv._beams);
    //     }

    //     // Coordinates
    //     if (fv._gridEnabled) {
    //         fv._gridRenderer.render(proj, mvm);
    //     }

    //     // Marker for followed fleet
    //     renderFollowedFleet(fv, proj, mvm, fv._fleets[fv._camFollowFleet]);

    //     // renderModel(fv, proj, mvm, fv._moon, 40000, -40000, 1000, 0, 0, 500);
    //     // renderModel(fv, proj, mvm, fv._moon, -10000, -160000, 2500, 0, 0, 300);
    m_context->finish();
}

void
client::vcr::flak::ArenaWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::vcr::flak::ArenaWidget::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::vcr::flak::ArenaWidget::getLayoutInfo() const
{
    return ui::layout::Info(gfx::Point(400, 400),
                            gfx::Point(400, 400),
                            ui::layout::Info::GrowBoth);
}

bool
client::vcr::flak::ArenaWidget::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::vcr::flak::ArenaWidget::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}
