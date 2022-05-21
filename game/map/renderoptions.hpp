/**
  *  \file game/map/renderoptions.hpp
  *  \brief Class game::map::RenderOptions
  */
#ifndef C2NG_GAME_MAP_RENDEROPTIONS_HPP
#define C2NG_GAME_MAP_RENDEROPTIONS_HPP

#include "afl/base/signal.hpp"
#include "afl/bits/smallset.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/map/viewport.hpp"
#include "util/key.hpp"

namespace game { namespace map {

    /** Map rendering options.
        This class bridges the gap between a game::map::Renderer/Viewport,
        configuration stored in a game::config::UserConfiguration,
        and user inputs.

        Use a RenderOptions object to load and store options in a UserConfiguration,
        modify it, and to configure a Renderer/Viewport.
        RenderOptions is a lightweight value object that can be passed between components. */
    class RenderOptions {
     public:
        /** Area for configuration access. */
        enum Area {             // ex GChartOptionIndex
            Small,              // ex SmallChartOptions
            Normal,             // ex NormalChartOptions
            Scanner             // ex ScannerChartOptions
        };
        static const size_t NUM_AREAS = 3;

        /** Definition of an option. */
        enum Option {
            ShowIonStorms,      // co_Ion       = 1
            ShowMinefields,     // co_Mine      = 2
            ShowUfos,           // co_Ufo       = 4
            ShowGrid,           // co_Sectors   = 8
            ShowBorders,        // co_Borders   = 16
            ShowDrawings,       // co_Drawings  = 32
            ShowSelection,      // co_Selection = 64
            ShowLabels,         // co_Labels    = 128
            ShowTrails,         // co_Trails    = 256
            ShowShipDots,       // co_ShipDots  = 512    // called co_NoTriangles in PCC 1.x
            ShowWarpWells,      // co_WarpWells = 1024
            ShowMessages,       // co_Messages  = 2048
            ShowMineDecay       // xref all(), xref UserConfiguration::ChartRenderOptions
        };
        typedef afl::bits::SmallSet<Option> Options_t;

        /** Value of an option. */
        enum Value {
            Disabled,           // Option is off.
            Enabled,            // Option is on.
            Filled              // Option is on + filled.
        };

        /** Default constructor.
            Sets options to defaults. */
        RenderOptions();

        /** Construct from values.
            \param show Initial value for "show".
            \param fill Initial value for "fill". */
        RenderOptions(Options_t show, Options_t fill);

        /** Toggle options.
            \param which Options to toggle */
        void toggleOptions(Options_t which);

        /** Copy option values.
            \param opts Other RenderOptions object
            \param which Options to copy */
        void copyOptions(const RenderOptions& opts, Options_t which);

        /** Set options to "enabled, not filled".
            \param which Options to set */
        void setOptions(Options_t which);

        /** Get value of an option.
            \param which Option to query
            \return value */
        Value getOption(Option which) const;

        /** Get value in Viewport::Options_t format.
            \return Viewport::Options_t */
        Viewport::Options_t getViewportOptions() const;

        /** Store to UserConfiguration.
            \param [out] config Configuration
            \param [in]  area   Area to store in */
        void storeToConfiguration(game::config::UserConfiguration& config, Area area) const;

        /** Construct from UserConfiguration.
            \param config Configuration
            \param area   Area to store in
            \return RenderOptions corresponding to selected configuration values */
        static RenderOptions fromConfiguration(const game::config::UserConfiguration& config, Area area);

        /** Get option from key.
            \param key Key; must be a printable character to be recognized
            \return Options_t value corresponding to the pressed key, for use with toggleOptions();
                    empty set if key is not recognized */
        static Options_t getOptionFromKey(util::Key_t key);

        static Options_t all();
        static Options_t tristate();
        static Options_t defaults();

     private:
        Options_t m_show;
        Options_t m_fill;
    };

} }


inline game::map::RenderOptions::Options_t
game::map::RenderOptions::all()
{
    return Options_t::allUpTo(ShowMineDecay);
}

inline game::map::RenderOptions::Options_t
game::map::RenderOptions::tristate()
{
    // ex co_Tristate
    return Options_t() + ShowIonStorms + ShowMinefields + ShowUfos + ShowGrid;
}

inline game::map::RenderOptions::Options_t
game::map::RenderOptions::defaults()
{
    // ex co_Default
    return all() - ShowTrails - ShowWarpWells - ShowMineDecay;
}

#endif
