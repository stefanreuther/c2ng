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
        // Using int32_t. Some values (namely: StartingDistanceShip, StartingDistancePlanet) are close to 16-bit limit.
        // No need to optimize the others as we'll probably not build this for 16 bits ever.
        int32_t RatingBeamScale;
        int32_t RatingTorpScale;
        int32_t RatingBayScale;
        int32_t RatingMassScale;
        int32_t RatingPEBonus;
        int32_t RatingFullAttackBonus;
        int32_t RatingRandomBonus;

        int32_t StartingDistanceShip;
        int32_t StartingDistancePlanet;
        int32_t StartingDistancePerPlayer;
        int32_t StartingDistancePerFleet;

        int32_t CompensationShipScale;
        int32_t CompensationBeamScale;
        int32_t CompensationTorpScale;
        int32_t CompensationFighterScale;
        int32_t CompensationLimit;
        int32_t CompensationMass100KTScale;
        int32_t CompensationAdjust;

        int32_t CyborgDebrisRate;

        int32_t MaximumFleetSize;

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
