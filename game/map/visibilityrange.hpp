/**
  *  \file game/map/visibilityrange.hpp
  *  \brief Visibility Range Computation
  */
#ifndef C2NG_GAME_MAP_VISIBILITYRANGE_HPP
#define C2NG_GAME_MAP_VISIBILITYRANGE_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/teamsettings.hpp"

namespace game { namespace map {

    class RangeSet;
    class Universe;

    /** Mode for visibility range computation. */
    enum VisMode {
        VisModeOwn,               ///< Own units.
        VisModeShips,             ///< Own ships.
        VisModePlanets,           ///< Own planets.
        VisModeMarked             ///< Marked units no matter who owns them.
    };
    static const int VisModeMax = static_cast<int>(VisModeMarked);

    /** Canned visibility range setting.
        @see getVisibilityRangeSettings() */
    struct VisSetting {
        String_t name;            ///< Human-readable name of setting.
        VisMode mode;             ///< Mode to use.
        int range;                ///< Range to use.
        VisSetting(String_t name, VisMode mode, int range)
            : name(name), mode(mode), range(range)
            { }
    };
    typedef std::vector<VisSetting> VisSettings_t;

    /** Visibility range configuration.
        @see buildVisibilityRange(), loadVisibilityConfiguration(), saveVisibilityConfiguration() */
    struct VisConfig {
        VisMode mode;             ///< Mode.
        int range;                ///< Range.
        bool useTeam;             ///< true to include team units as own units.

        VisConfig()
            : mode(VisModeOwn), range(200), useTeam(false)
            { }
        VisConfig(VisMode mode, int range, bool useTeam)
            : mode(mode), range(range), useTeam(useTeam)
            { }
    };

    /** Get available visibility range settings.
        Determines settings from host configuration.
        @param config           Host configuration
        @param viewpointPlayer  Viewpoint player (index into config options)
        @param tx               Translator
        @return Settings */
    VisSettings_t getVisibilityRangeSettings(const game::config::HostConfiguration& config, int viewpointPlayer, afl::string::Translator& tx);

    /** Build visibility range according to configuration.
        @param [out] out        Result
        @param [in]  univ       Universe
        @param [in]  vc         Configuration
        @param [in]  team       Team settings (viewpoint player, allies) */
    void buildVisibilityRange(RangeSet& out, const Universe& univ, const VisConfig& vc, const TeamSettings& team);

    /** Save visibility range configuration in user preferences.
        @param [out] pref       User preferences
        @param [in]  vc         Configuration */
    void saveVisibilityConfiguration(game::config::UserConfiguration& pref, const VisConfig& vc);

    /** Load visibility range configuration from user preferences.
        If values are unset/invalid, returns defaults.
        @param pref             User preferences
        @return Configuration */
    VisConfig loadVisibilityConfiguration(const game::config::UserConfiguration& pref);

    /** Format VisMode as string.
        @param mode Mode
        @param tx   Translator
        @return string */
    String_t toString(VisMode mode, afl::string::Translator& tx);

} }

#endif
