/**
  *  \file client/vcr/flak/threedrenderer.hpp
  *  \brief Class client::vcr::flak::ThreeDRenderer
  */
#ifndef C2NG_CLIENT_VCR_FLAK_THREEDRENDERER_HPP
#define C2NG_CLIENT_VCR_FLAK_THREEDRENDERER_HPP

#include "afl/base/ref.hpp"
#include "client/vcr/flak/renderer.hpp"
#include "game/playerarray.hpp"
#include "game/vcr/flak/visualisationsettings.hpp"
#include "game/vcr/flak/visualisationstate.hpp"
#include "gfx/threed/context.hpp"
#include "ui/root.hpp"

namespace client { namespace vcr { namespace flak {

    /** 3D renderer.
        This is modeled after the WebGL version in PCC2 Web. */
    class ThreeDRenderer : public Renderer {
     public:
        ThreeDRenderer(ui::Root& root,
                       game::vcr::flak::VisualisationState& state,
                       game::vcr::flak::VisualisationSettings& settings);
        virtual void init();
        virtual void draw(gfx::Canvas& can, const gfx::Rectangle& area, bool grid);

     private:
        // Integration
        ui::Root& m_root;
        game::vcr::flak::VisualisationState& m_state;
        game::vcr::flak::VisualisationSettings& m_settings;

        // 3D Models
        afl::base::Ref<gfx::threed::Context> m_context;
        afl::base::Ref<gfx::threed::ParticleRenderer> m_smokeRenderer;
        afl::base::Ref<gfx::threed::TriangleRenderer> m_torpedoModel;
        afl::base::Ref<gfx::threed::LineRenderer> m_gridRenderer;
        afl::base::Ref<gfx::threed::LineRenderer> m_beamRenderer;
        afl::base::Ref<gfx::threed::LineRenderer> m_wireframeShip;
        afl::base::Ref<gfx::threed::LineRenderer> m_wireframePlanet;

        game::PlayerArray<afl::base::Ptr<gfx::threed::TriangleRenderer> > m_shipModels;
        game::PlayerArray<afl::base::Ptr<gfx::threed::TriangleRenderer> > m_fighterModels;
        game::PlayerArray<afl::base::Ptr<gfx::threed::TriangleRenderer> > m_planetModels;
    };

} } }

#endif
