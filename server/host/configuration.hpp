/**
  *  \file server/host/configuration.hpp
  *  \brief Structure server::host::Configuration
  */
#ifndef C2NG_SERVER_HOST_CONFIGURATION_HPP
#define C2NG_SERVER_HOST_CONFIGURATION_HPP

#include "afl/net/name.hpp"
#include "afl/string/string.hpp"
#include "server/types.hpp"

namespace server { namespace host {

    /** Service configuration.
        This structure contains "passive" configuration elements. */
    struct Configuration {
        /** Default constructor.
            Sets everything to defaults. */
        Configuration();

        /** Time scale.
            Our times are stored in minutes-since-epoch.
            This variable contains the number of seconds in a minute.
            By scaling it down, the system can be accelerated for testing. */
        // ex planetscentral/host/schedule.cc:time_scale
        // ex planetscentral/host/schedule.cc:setTimeScale
        int timeScale;

        /** Work directory. */
        String_t workDirectory;

        /** Binary directory. */
        String_t binDirectory;

        /** Cron. */
        bool useCron;

        /** Backup mode. */
        bool unpackBackups;

        /** Users see temporary turns flag.
            If enabled (default since Jan 2018), users see the temporary flag for all turns.
            If disabled, only the player of a slot sees that it is temporary. */
        bool usersSeeTemporaryTurns;

        /** Number of missed turns after which users are automatically kicked.
            Zero means never. */
        int numMissedTurnsForKick;

        /** HostFile address.
            Since we're generating links to this service, it must be in the config. */
        afl::net::Name hostFileAddress;

        /** Initial suspension time. */
        Time_t initialSuspend;

        /** Number of keys to store per user. */
        int maxStoredKeys;

        /** Title (Line1) for generated keys.
            Empty to disable. */
        String_t keyTitle;

        /** Secret for generated keys. */
        String_t keySecret;

        /** Convert time.
            On the wire, times are always given in minutes-since-epoch.
            If the system internally runs on a higher rate for testing,
            we have to convert them to not confuse users too much
            (users scale by 60 to obtain Unix time).
            \param t Internal time
            \return Time to report to users */
        int32_t getUserTimeFromTime(Time_t t) const;
    };

} }

#endif
