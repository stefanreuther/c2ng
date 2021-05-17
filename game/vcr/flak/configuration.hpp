/**
  *  \file game/vcr/flak/configuration.hpp
  *  \brief Structure game::vcr::flak::Configuration
  */
#ifndef C2NG_GAME_VCR_FLAK_CONFIGURATION_HPP
#define C2NG_GAME_VCR_FLAK_CONFIGURATION_HPP

#include "afl/io/directory.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"

namespace game { namespace vcr { namespace flak {

    /** FLAK Configuration Structure.
        This contains the FLAK-specific configuration settings.
        Its members correspond 1:1 to the configuration file entries.

        This configuration is needed in addition to the normal host configuration
        (PConfig / game::spec::HostConfiguration). */
    struct Configuration {
        // ex FlakConfig
        int   RatingBeamScale;
        int   RatingTorpScale;
        int   RatingBayScale;
        int   RatingMassScale;
        int   RatingPEBonus;
        int   RatingFullAttackBonus;
        int   RatingRandomBonus;

        int   StartingDistanceShip;        // FIXME: close to edge of 16-bit limit
        int   StartingDistancePlanet;      // FIXME: close to edge of 16-bit limit
        int   StartingDistancePerPlayer;
        int   StartingDistancePerFleet;

        int   CompensationShipScale;
        int   CompensationBeamScale;
        int   CompensationTorpScale;
        int   CompensationFighterScale;
        int   CompensationLimit;
        int   CompensationMass100KTScale;
        int   CompensationAdjust;

        int   CyborgDebrisRate;

        int   MaximumFleetSize;

        bool  SendUtilData;

        Configuration();
    };

    /** Initialize FLAK Configuration to defaults.
        \param [out] config Configuration */
    void initConfiguration(Configuration& config);

    /** Parse FLAK configuration file.
        \param [out] config     Configuration
        \param [in]  file       input file
        \param [in]  inSection  if true, assume we are inside a FLAK configuration block. If false, look for a "%flak" section delimiter first.
        \param [in]  log        LogListener (for warnings)
        \param [in]  tx         Translator (for warnings) */
    void loadConfiguration(Configuration& config, afl::io::Stream& file, bool inSection, afl::sys::LogListener& log, afl::string::Translator& tx);

    /** Load FLAK configuration from a directory.
        \param [out] config     Configuration
        \param [in]  dir        Directory
        \param [in]  log        LogListener (for warnings)
        \param [in]  tx         Translator (for warnings) */
    void loadConfiguration(Configuration& config, afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);

} } }

#endif
