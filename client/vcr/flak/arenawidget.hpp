/**
  *  \file client/vcr/flak/arenawidget.hpp
  */
#ifndef C2NG_CLIENT_VCR_FLAK_ARENAWIDGET_HPP
#define C2NG_CLIENT_VCR_FLAK_ARENAWIDGET_HPP

#include "ui/simplewidget.hpp"
#include "game/vcr/flak/visualisationstate.hpp"
#include "game/vcr/flak/visualisationsettings.hpp"
#include "gfx/threed/context.hpp"
#include "ui/root.hpp"

namespace client { namespace vcr { namespace flak {

    class ArenaWidget : public ui::SimpleWidget {
     public:
        ArenaWidget(ui::Root& root,
                    game::vcr::flak::VisualisationState& state,
                    game::vcr::flak::VisualisationSettings& settings);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ui::Root& m_root;
        game::vcr::flak::VisualisationState& m_state;
        game::vcr::flak::VisualisationSettings& m_settings;

        // FIXME: colors for models!
        afl::base::Ref<gfx::threed::Context> m_context;
        afl::base::Ref<gfx::threed::ParticleRenderer> m_smokeRenderer;
        afl::base::Ref<gfx::threed::TriangleRenderer> m_shipModel;
        afl::base::Ref<gfx::threed::TriangleRenderer> m_planetModel;
        afl::base::Ref<gfx::threed::TriangleRenderer> m_torpedoModel;
        afl::base::Ref<gfx::threed::TriangleRenderer> m_fighterModel;
        // FIXME: grid
    };

} } }

#endif
