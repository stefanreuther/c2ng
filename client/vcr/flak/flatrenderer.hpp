/**
  *  \file client/vcr/flak/flatrenderer.hpp
  *  \brief Class client::vcr::flak::FlatRenderer
  */
#ifndef C2NG_CLIENT_VCR_FLAK_FLATRENDERER_HPP
#define C2NG_CLIENT_VCR_FLAK_FLATRENDERER_HPP

#include "client/vcr/flak/renderer.hpp"
#include "game/vcr/flak/visualisationsettings.hpp"
#include "game/vcr/flak/visualisationstate.hpp"
#include "ui/root.hpp"

namespace client { namespace vcr { namespace flak {

    /** Flat renderer.
        This is the classic visualisation available in PCC1/PCC2. */
    class FlatRenderer : public Renderer {
     public:
        FlatRenderer(ui::Root& root,
                     game::vcr::flak::VisualisationState& state,
                     game::vcr::flak::VisualisationSettings& settings);
        virtual void init();
        virtual void draw(gfx::Canvas& can, const gfx::Rectangle& area, bool grid);

     private:
        // Integration
        ui::Root& m_root;
        game::vcr::flak::VisualisationState& m_state;
        game::vcr::flak::VisualisationSettings& m_settings;
    };

} } }

#endif
