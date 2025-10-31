/**
  *  \file client/vcr/configuration.hpp
  *  \brief Class client::vcr::Configuration
  */
#ifndef C2NG_CLIENT_VCR_CONFIGURATION_HPP
#define C2NG_CLIENT_VCR_CONFIGURATION_HPP

#include "afl/string/translator.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/proxy/configurationproxy.hpp"

namespace client { namespace vcr {

    /** VCR Player configuration.
        Provides a wrapper to access configuration through a ConfigurationProxy,
        and conversion to c2ng-specific values/ranges. */
    class Configuration {
     public:
        /** Fastest speed value. */
        static const int FASTEST_SPEED = 0;

        /** Slowest speed value. */
        static const int SLOWEST_SPEED = 10;

        /** Constructor. */
        Configuration();

        /** Load from ConfigurationProxy.
            @param link  WaitIndicator
            @param proxy Proxy */
        void load(game::proxy::WaitIndicator& link, game::proxy::ConfigurationProxy& proxy);

        /** Save to ConfigurationProxy.
            @param proxy Proxy */
        void save(game::proxy::ConfigurationProxy& proxy);

        /** Change speed.
            @param delta Difference. Positive: slower, negative: faster */
        void changeSpeed(int delta);

        /** Set speed to fixed value (UserConfiguration::Vcr_Speed).

            PCC2 defines speed as interval-in-50-Hz-ticks with the default speed of 2 (=25 Hz).
            We define speeds such that the first values have roughly the same meaning as in PCC2,
            which means 0 (FASTEST_SPEED) is fastest, higher values are slower.

            @param value New speed */
        void setSpeed(int value);

        /** Set renderer mode (UserConfiguration::Vcr_Renderer).
            @param m Mode */
        void setRendererMode(game::config::UserConfiguration::RendererMode m);

        /** Set next available renderer mode. */
        void cycleRendererMode();

        /** Set effects mode (UserConfiguration::Vcr_Effects).
            @param m Mode */
        void setEffectsMode(game::config::UserConfiguration::EffectsMode m);

        /** Set next available effects mode. */
        void cycleEffectsMode();

        /** Set FLAK renderer mode (UserConfiguration::Flak_Renderer).
            @param m Mode */
        void setFlakRendererMode(game::config::UserConfiguration::FlakRendererMode m);

        /** Set next available FLAK renderer mode. */
        void cycleFlakRendererMode();

        /** Toggle FLAK renderer mode.
            If mode is a, set mode b; otherwise, set a.
            @param a First mode
            @param b Second mode */
        void toggleFlakRendererMode(game::config::UserConfiguration::FlakRendererMode a,
                                    game::config::UserConfiguration::FlakRendererMode b);

        /** Set FLAK grid display status (UserConfiguration::Flak_Grid).
            @param flag Flag */
        void setFlakGrid(bool flag);

        /** Toggle FLAK grid display. */
        void toggleFlakGrid();

        /** Get timer tick interval derived from speed.
            Animations shall tick with this interval.
            @return interval, milliseconds */
        int getTickInterval() const;

        /** Get timer ticks per battle cycle.
            For every this number of timer ticks, one call to Algorithm::playCycle() shall be done.
            @return count */
        int getNumTicksPerBattleCycle() const;

        /** Get speed value.
            @return value
            @see setSpeed */
        int getSpeed() const;

        /** Get renderer mode.
            @return mode
            @see setRendererMode */
        game::config::UserConfiguration::RendererMode getRendererMode() const;

        /** Get effects mode.
            @return mode
            @see setEffectsMode */
        game::config::UserConfiguration::EffectsMode getEffectsMode() const;

        /** Get FLAK renderer mode.
            @return mode
            @see setFlakRendererMode */
        game::config::UserConfiguration::FlakRendererMode getFlakRendererMode() const;

        /** Check whether grid is shown.
            @return flag */
        bool hasFlakGrid() const;

        /** Get name for a speed value.
            @param speed Speed
            @param tx    Translator
            @return name
            @see setSpeed */
        static String_t getSpeedName(int speed, afl::string::Translator& tx);

     private:
        int m_speed;
        int m_rendererMode;
        int m_effectsMode;
        int m_flakRendererMode;
        bool m_flakGrid;
    };

} }

#endif
