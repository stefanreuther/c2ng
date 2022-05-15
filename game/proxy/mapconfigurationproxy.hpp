/**
  *  \file game/proxy/mapconfigurationproxy.hpp
  *  \brief Class game::proxy::MapConfigurationProxy
  */
#ifndef C2NG_GAME_PROXY_MAPCONFIGURATIONPROXY_HPP
#define C2NG_GAME_PROXY_MAPCONFIGURATIONPROXY_HPP

#include "game/config/markeroption.hpp"
#include "game/map/configuration.hpp"
#include "game/map/renderoptions.hpp"
#include "game/proxy/configurationproxy.hpp"

namespace game { namespace proxy {

    /** Proxy to access map configuration.
        Extends ConfigurationProxy with operations to access specific starchart-related configuration items.
        For now, this is separate from ConfigurationProxy to reduce dependencies; maybe merge later on. */
    class MapConfigurationProxy : public ConfigurationProxy {
     public:
        /** Constructor.
            @param gameSender Game sender */
        explicit MapConfigurationProxy(util::RequestSender<Session> gameSender);

        /** Get map configuration.
            Retrieves the current map configuration, which was built from UserConfiguration, HostConfiguration, and HostVersion.
            @param [in,out] ind     WaitIndicator
            @param [out]    config  Map configuration */
        void getMapConfiguration(WaitIndicator& ind, game::map::Configuration& config);

        /** Set map configuration.
            Updates UserConfiguration to represent the given map configuration.
            @param config  Map configuration */
        void setMapConfiguration(const game::map::Configuration& config);

        /** Get render options for an area.
            A RenderOptions object groups a few UserConfiguration options.
            @param [in,out] ind     WaitIndicator
            @param [in]     a       Area
            @return Render options for that area */
        game::map::RenderOptions getRenderOptions(WaitIndicator& ind, game::map::RenderOptions::Area a);

        /** Set render options for an area.
            @param area  Area
            @param opts  Options to set */
        void setRenderOptions(game::map::RenderOptions::Area area, const game::map::RenderOptions& opts);

        /** Get all marker templates (canned marker) configuration.
            @param [in,out] ind     WaitIndicator
            @param [out]    config  Marker templates */
        void getMarkerConfiguration(WaitIndicator& ind, std::vector<game::config::MarkerOption::Data>& config);

        /** Set marker template (canned marker) configuration.
            @param [in] index  Index (zero-based index into getMarkerConfiguration() result). Call is ignored if this is out-of-range.
            @param [in] config Configuration */
        void setMarkerConfiguration(size_t index, const game::config::MarkerOption::Data& config);
    };

} }

#endif
