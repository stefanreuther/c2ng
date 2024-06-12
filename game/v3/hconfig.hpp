/**
  *  \file game/v3/hconfig.hpp
  *  \brief HConfig Access Functions
  */
#ifndef C2NG_GAME_V3_HCONFIG_HPP
#define C2NG_GAME_V3_HCONFIG_HPP

#include "game/v3/structures.hpp"
#include "afl/base/types.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace v3 {

    /** Unpack HCONFIG.HST image into internal structure.
        \param data    [in] Data read from file
        \param size    [in] Number of valid bytes in \c data
        \param config  [in,out] Target configuration structure.
                       Should contain up-to-date PlayerRace setting for decoding of scalar options
                       that we use as arrays.
        \param source  [in] "source" to use for values read from file */
    void unpackHConfig(const structures::HConfig& data, size_t size,
                       game::config::HostConfiguration& config,
                       game::config::ConfigurationOption::Source source);

    /** Pack HCONFIG.HST from internal structure.
        \param data    [out] Data to write from file; always populated completely
        \param config  [in] Target configuration structure */
    void packHConfig(structures::HConfig& data, const game::config::HostConfiguration& config);

} }

#endif
