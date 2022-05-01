/**
  *  \file game/proxy/visibilityrangeproxy.hpp
  *  \brief Class game::proxy::VisibilityRangeProxy
  */
#ifndef C2NG_GAME_PROXY_VISIBILITYRANGEPROXY_HPP
#define C2NG_GAME_PROXY_VISIBILITYRANGEPROXY_HPP

#include "afl/string/translator.hpp"
#include "game/map/rangeset.hpp"
#include "game/map/visibilityrange.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Bidirectional, synchronous proxy for visibility range plotting.
        Provides synchronous access to the functions from game/map/visibilityrange.hpp. */
    class VisibilityRangeProxy {
     public:
        /** Constructor.
            @param gameSender  Game sender */
        explicit VisibilityRangeProxy(util::RequestSender<Session> gameSender);

        /** Load visibility range configuration from user preferences.
            @param ind    WaitIndicator for UI synchronisation
            @return Configuration
            @see game::map::loadVisibilityConfiguration() */
        game::map::VisConfig loadVisibilityConfiguration(WaitIndicator& ind);

        /** Get available visibility range settings.
            @param ind    WaitIndicator for UI synchronisation
            @return Settings
            @see game::map::getVisibilityRangeSettings() */
        game::map::VisSettings_t getVisibilityRangeSettings(WaitIndicator& ind);

        /** Build visibility range according to configuration.
            @param ind    WaitIndicator for UI synchronisation
            @param vc     Configuration
            @return newly-allocated RangeSet containing the result. Never null.
            @see game::map::buildVisibilityRange() */
        std::auto_ptr<game::map::RangeSet> buildVisibilityRange(WaitIndicator& ind, const game::map::VisConfig& vc);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
