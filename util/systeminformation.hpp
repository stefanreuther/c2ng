/**
  *  \file util/systeminformation.hpp
  *  \brief Structure util::SystemInformation
  */
#ifndef C2NG_UTIL_SYSTEMINFORMATION_HPP
#define C2NG_UTIL_SYSTEMINFORMATION_HPP

#include "afl/string/string.hpp"
#include "afl/base/types.hpp"

namespace util {

    /** System information.
        A structure of (entirely optional) information to tailor the behaviour to the current system.

        (Eventually, some of this might be moved to afl::sys::Environment.) */
    struct SystemInformation {
        /** Number of processors. */
        size_t numProcessors;

        /** Operating system name. */
        String_t operatingSystem;

        /** Constructor.
            Sets everything to defaults. */
        SystemInformation()
            : numProcessors(1),
              operatingSystem("<none>")
            { }
    };

    /** Get system information.
        Determines actual system information.
        \return populated SystemInformation structure */
    SystemInformation getSystemInformation();

}

#endif
