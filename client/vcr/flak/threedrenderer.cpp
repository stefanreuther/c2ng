/**
  *  \file client/vcr/flak/threedrenderer.cpp
  *  \brief Class client::vcr::flak::ThreeDRenderer
  *
  *  FIXME: can we do the awesome skybox from the WebGL version?
  */

#include "client/vcr/flak/threedrenderer.hpp"
#include "client/widgets/playerlist.hpp"
#include "gfx/threed/softwarecontext.hpp"
#include "gfx/threed/vecmath.hpp"
#include "ui/colorscheme.hpp"
#include "util/math.hpp"

using game::vcr::flak::Position;
using game::vcr::flak::VisualisationState;
using gfx::ColorQuad_t;
using gfx::threed::Context;
using gfx::threed::LineRenderer;
using gfx::threed::Mat4f;
using gfx::threed::ParticleRenderer;
using gfx::threed::TriangleRenderer;
using gfx::threed::Vec3f;

namespace {
    const double RAISE_UNIT = 10000;
    const int MAX_SMOKE_AGE = 10;

    Vec3f convertPosition(const Position& pos)
    {
        return Vec3f(float(pos.x), float(pos.y), float(pos.z));
    }

    float interp(int32_t a, int32_t b, float f)
    {
        return float(a)*(1-f) + float(b)*f;
    }

    ColorQuad_t getPlayerColor(int player)
    {
        return player == 0
            ? COLORQUAD_FROM_RGB(76, 76, 76)
            : ui::STANDARD_COLORS[client::widgets::PlayerList::getPlayerColor(player)];
    }

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

    // Same as buildGenericShip, but builds a wireframe
    afl::base::Ref<LineRenderer> makeWireframeShip(Context& ctx, ColorQuad_t color)
    {
        afl::base::Ref<LineRenderer> ren = ctx.createLineRenderer();
        const float W = 0.5f, H = 1.25f;
        const Vec3f
            R1(-H, -W, -W),
            R2(-H,  W, -W),
            R3(-H,  W,  W),
            R4(-H, -W,  W),
            F(H, 0, 0),
            A(-H-0.25f, 0, 0);
        ren->add(R1, R2, color);
        ren->add(R2, R3, color);
        ren->add(R3, R4, color);
        ren->add(R4, R1, color);
        ren->add(R1, F, color);
        ren->add(R2, F, color);
        ren->add(R3, F, color);
        ren->add(R4, F, color);
        ren->add(R1, A, color);
        ren->add(R2, A, color);
        ren->add(R3, A, color);
        ren->add(R4, A, color);
        return ren;
    }

    // Make ship for a player
    afl::base::Ref<TriangleRenderer> makeShip(Context& ctx, int owner)
    {
        afl::base::Ref<TriangleRenderer> ren = ctx.createTriangleRenderer();
        ColorQuad_t color = getPlayerColor(owner);
        buildGenericShip(*ren, 1.0,
                         COLORQUAD_FROM_RGB(RED_FROM_COLORQUAD(color)   / 2 + 64,     // FIXME: correct color?
                                            GREEN_FROM_COLORQUAD(color) / 2 + 64,
                                            BLUE_FROM_COLORQUAD(color)  / 2 + 64));
        return ren;
    }

    afl::base::Ref<LineRenderer> makeWireframePlanet(Context& ctx, ColorQuad_t color)
    {
        afl::base::Ref<LineRenderer> ren = ctx.createLineRenderer();
        ren->addSphere(Vec3f(0,0,0), 1, color, 10);
        return ren;
    }

    // Make planet for a player
    afl::base::Ref<TriangleRenderer> makePlanet(Context& ctx, int owner) {
        afl::base::Ref<TriangleRenderer> ren = ctx.createTriangleRenderer();
        ColorQuad_t color = getPlayerColor(owner);
        ren->addSphere(Vec3f(0,0,0), 1,
                       COLORQUAD_FROM_RGB(RED_FROM_COLORQUAD(color)   / 2 + 64,     // FIXME: correct color?
                                          GREEN_FROM_COLORQUAD(color) / 2 + 64,
                                          BLUE_FROM_COLORQUAD(color)  / 2 + 64),
                       10);
        return ren;
    }

    // Make fighter for a player
    afl::base::Ref<TriangleRenderer> makeFighter(Context& ctx, int owner)
    {
        afl::base::Ref<TriangleRenderer> ren = ctx.createTriangleRenderer();
        ColorQuad_t color = getPlayerColor(owner);
        buildGenericShip(*ren, 0.5,
                         COLORQUAD_FROM_RGB(RED_FROM_COLORQUAD(color)   / 2 + 64,
                                            GREEN_FROM_COLORQUAD(color) / 2 + 64,
                                            BLUE_FROM_COLORQUAD(color) *3/4 + 64));
        return ren;
    }

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

    void makeGrid(LineRenderer& grid, int32_t size)
    {
        // ex makeGrid(gl, size)
        const int32_t DIST = 10000;
        const int dim = std::max(1, (size + 3000) / DIST);
        const ColorQuad_t color = COLORQUAD_FROM_RGBA(128, 128, 128, 128);

        for (int i = -dim; i <= dim; ++i) {
            grid.add(Vec3f( float(DIST*i),   -float(dim*DIST), 0), Vec3f(float(DIST*i),  float(dim*DIST), 0), color);
            grid.add(Vec3f(-float(dim*DIST),  float(DIST*i),   0), Vec3f(float(dim*DIST), float(DIST*i), 0), color);
        }
    }

    void renderModel(const Mat4f& proj, const Mat4f& mvm, TriangleRenderer& model, const Position& pos, float rotX, float rotZ, float modelScale)
    {
        // ex renderModel(fv, proj, mvm, model, x, y, z, rotX, rotZ, modelScale)
        // mvm already includes the Camera and World-to-OpenGL transformation
        Mat4f modelViewMatrix = mvm;

        // Move to world location
        modelViewMatrix.translate(convertPosition(pos));

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
            ren.add(Vec3f(convertPosition(p->pos)), a);
        }

        ren.render(proj, mvm);
    }

    void renderBeams(afl::base::Memory<const VisualisationState::Beam> beams, const Mat4f& proj, const Mat4f& mvm, LineRenderer& ren)
    {
        // ex renderBeams(fv, proj, mvm, beams)
        struct BeamDef {
            ColorQuad_t color;
            float from;
            float to;
        };
        static const BeamDef BEAMS[] = {
            { COLORQUAD_FROM_RGB(255, 0, 0),  0.0f, 0.3f },
            { COLORQUAD_FROM_RGB(255, 0, 0),  0.0f, 0.6f },
            { COLORQUAD_FROM_RGB(192, 0, 0),  0.3f, 1.0f },
            { COLORQUAD_FROM_RGB(128, 0, 0),  0.6f, 1.0f },
        };
        const int NUM_BEAMS = sizeof(BEAMS)/sizeof(BEAMS[0]);

        ren.clear();
        while (const VisualisationState::Beam* p = beams.eat()) {
            if (p->age < NUM_BEAMS) {
                const BeamDef& d = BEAMS[p->age];
                ren.add(Vec3f(interp(p->from.x, p->to.x, d.from),
                              interp(p->from.y, p->to.y, d.from),
                              interp(p->from.z, p->to.z, d.from)),
                        Vec3f(interp(p->from.x, p->to.x, d.to),
                              interp(p->from.y, p->to.y, d.to),
                              interp(p->from.z, p->to.z, d.to)),
                        d.color);
            }
        }

        // Render
        ren.render(proj, mvm);
    }

    void renderFollowedFleet(const Mat4f& proj,
                             const Mat4f& mvm,
                             const VisualisationState::Fleet* fleet,
                             const VisualisationState& st,
                             LineRenderer& planetModel,
                             LineRenderer& shipModel)
    {
        // ex renderFollowedFleet(fv, proj, mvm, fleet)
        if (fleet != 0 && fleet->isAlive) {
            for (size_t i = 0; i < fleet->numShips; ++i) {
                const VisualisationState::Ship* sh = st.ships().at(fleet->firstShip + i);
                if (sh != 0 && sh->isAlive) {
                    // mvm already includes the Camera and World-to-OpenGL transformation
                    Mat4f modelViewMatrix = mvm;

                    // Move to world location
                    modelViewMatrix.translate(convertPosition(sh->pos));

                    // Rotate (model looks down)
                    float rotX = sh->isPlanet ? 0.0f : float(util::PI/2);
                    float rotZ = sh->heading;
                    if (rotZ) {
                        modelViewMatrix.rotateZ(rotZ);
                    }
                    if (rotX) {
                        modelViewMatrix.rotateX(rotX);
                    }

                    // Scale model to world size
                    modelViewMatrix.scale(1010);

                    // Render
                    if (sh->isPlanet) {
                        planetModel.render(proj, modelViewMatrix);
                    } else {
                        shipModel.render(proj, modelViewMatrix);
                    }
                }
            }
        }
    }
}

client::vcr::flak::ThreeDRenderer::ThreeDRenderer(ui::Root& root,
                                                  game::vcr::flak::VisualisationState& state,
                                                  game::vcr::flak::VisualisationSettings& settings)
    : Renderer(),
      m_root(root),
      m_state(state),
      m_settings(settings),
      m_context(gfx::threed::SoftwareContext::create()),
      m_smokeRenderer(m_context->createParticleRenderer()),
      m_torpedoModel(m_context->createTriangleRenderer()),
      m_gridRenderer(m_context->createLineRenderer()),
      m_beamRenderer(m_context->createLineRenderer()),
      m_wireframeShip(makeWireframeShip(*m_context, COLORQUAD_FROM_RGB(192, 192, 192))),
      m_wireframePlanet(makeWireframePlanet(*m_context, COLORQUAD_FROM_RGB(192, 192, 192))),
      m_shipModels(),
      m_fighterModels(),
      m_planetModels()
{
    makeTorpedo(*m_torpedoModel);

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
client::vcr::flak::ThreeDRenderer::init()
{
    makeGrid(*m_gridRenderer, m_state.getGridSize());

    // Unowned planet
    m_planetModels.set(0, makePlanet(*m_context, 0).asPtr());

    // Models for all units
    afl::base::Memory<const VisualisationState::Fleet> fleets = m_state.fleets();
    while (const VisualisationState::Fleet* p = fleets.eat()) {
        if (m_shipModels.get(p->player).get() == 0) {
            m_shipModels   .set(p->player, makeShip   (*m_context, p->player).asPtr());
            m_fighterModels.set(p->player, makeFighter(*m_context, p->player).asPtr());
            m_planetModels .set(p->player, makePlanet (*m_context, p->player).asPtr());
        }
    }
}

void
client::vcr::flak::ThreeDRenderer::draw(gfx::Canvas& can, const gfx::Rectangle& area, bool grid)
{
    // ex fvRender(fv)
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
                if (TriangleRenderer* p = m_planetModels.get(sh->player).get()) {
                    renderModel(proj, mvm, *p, sh->pos, 0, 0, 1000);
                }
            } else {
                if (TriangleRenderer* p = m_planetModels.get(0).get()) {
                    renderModel(proj, mvm, *p, sh->pos, 0, 0, 1000);
                }
            }
        } else {
            if (sh->isAlive) {
                if (TriangleRenderer* p = m_shipModels.get(sh->player).get()) {
                    renderModel(proj, mvm, *p, sh->pos, float(util::PI/2), sh->heading, 1000);
                }
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
            if (TriangleRenderer* p = m_fighterModels.get(e->player).get()) {
                renderModel(proj, mvm, *p, e->pos, float(util::PI/2), e->heading, 500);
            }
            break;
         case VisualisationState::NoObject:
            break;
        }
    }

    // Active beams
    renderBeams(m_state.beams(), proj, mvm, *m_beamRenderer);

    // Coordinates
    if (grid) {
        m_gridRenderer->render(proj, mvm);
    }

    // Marker for followed fleet
    renderFollowedFleet(proj, mvm, m_state.fleets().at(m_settings.getFollowedFleet()), m_state, *m_wireframePlanet, *m_wireframeShip);

    // Draw
    m_context->finish();
}
