/**
  *  \file game/config/userconfiguration.hpp
  */
#ifndef C2NG_GAME_CONFIG_USERCONFIGURATION_HPP
#define C2NG_GAME_CONFIG_USERCONFIGURATION_HPP

#include "game/config/configuration.hpp"
#include "util/profiledirectory.hpp"
#include "afl/io/directory.hpp"

namespace game { namespace config {

    class UserConfiguration : public Configuration {
     public:
        UserConfiguration();
        ~UserConfiguration();

        void setDefaultValues();

        void loadUserConfiguration(util::ProfileDirectory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);
        void loadGameConfiguration(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);

//     // Game-related options:
//     ConfigIntOption CollapseOldMessages;
//     ConfigIntOption RewrapMessages;
//     ConfigIntOption InstantBattleResult;

//     // Sound options
//     ConfigIntOption Sound16Bits;
//     ConfigIntOption SoundEnabled;
//     ConfigIntOption SoundFrequency;
//     ConfigIntOption SoundHeadphone;
//     ConfigIntOption SoundReverse;
//     ConfigIntOption SoundStereo;

//     // Starchart options
//     ConfigIntOption ChartAnimThreshold;
//     ConfigIntOption ChartMouseStickiness;
//     ConfigIntOption ChartScannerWarpWells;

//     // Alliance
//     ConfigIntOption TeamAutoSync;

//     // Preference
//     ConfigIntOption DisplayThousandSep;
//     ConfigIntOption DisplayClans;

//     // Export
//     ConfigStringOption ExportShipFields;
//     ConfigStringOption ExportPlanetFields;
    };

} }

#endif
