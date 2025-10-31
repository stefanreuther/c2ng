/**
  *  \file client/vcr/flak/arenawidget.hpp
  *  \brief Class client::vcr::flak::ArenaWidget
  */
#ifndef C2NG_CLIENT_VCR_FLAK_ARENAWIDGET_HPP
#define C2NG_CLIENT_VCR_FLAK_ARENAWIDGET_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/vcr/flak/visualisationsettings.hpp"
#include "game/vcr/flak/visualisationstate.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace client { namespace vcr { namespace flak {

    class Renderer;

    /** FLAK combat arena display.
        Renders a VisualisationState, using VisualisationSettings.

        Since both of those inputs are passive,
        the integrator is responsible to redraw when anything changes.

        To use:
        - construct
        - after initial state has been prepared in VisualisationState, call init()
        - repeatedly call requestRedraw() as needed */
    class ArenaWidget : public ui::SimpleWidget {
     public:
        typedef game::config::UserConfiguration::FlakRendererMode Mode_t;

        /** Constructor.
            \param root     Root
            \param state    State (can be empty at construction time; must out-live the ArenaWidget)
            \param settings Settings (must out-live the ArenaWidget) */
        ArenaWidget(ui::Root& root,
                    game::vcr::flak::VisualisationState& state,
                    game::vcr::flak::VisualisationSettings& settings);
        ~ArenaWidget();

        /** Initialize.
            Requires that the VisualisationState object has been prepared. */
        void init();

        /** Set whether grid is enabled.
            \param flag Flag */
        void setGrid(bool flag);

        /** Check whether grid is enabled.
            \return status */
        bool hasGrid() const;

        /** Set mode.
            \param m New mode */
        void setMode(Mode_t m);

        /** Get current mode.
            \return mode */
        Mode_t getMode() const;

        /** Get mode name.
            \param m Mode
            \param tx Translator */
        static String_t toString(Mode_t m, afl::string::Translator& tx);

        // Widget / SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        // Integration
        ui::Root& m_root;
        game::vcr::flak::VisualisationState& m_state;
        game::vcr::flak::VisualisationSettings& m_settings;

        afl::container::PtrVector<Renderer> m_renderers;
        Mode_t m_currentRenderer;

        bool m_grid;
    };

} } }

#endif
