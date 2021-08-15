/**
  *  \file util/systeminformation.cpp
  *  \brief Structure util::SystemInformation
  */

#include "util/systeminformation.hpp"

#ifdef TARGET_OS_POSIX
/*
 *  POSIX implementation
 */

# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# include <sys/utsname.h>
# include <sched.h>
# include "config.h"

namespace {
    size_t getNumberOfProcessors()
    {
#if HAVE_SCHED_GETAFFINITY
        // Linux version for most of us
        // An alternative to sched_getaffinity would be to parse /proc/cpuinfo.
        cpu_set_t set;
        if (sched_getaffinity(0, sizeof(set), &set) == 0) {
            size_t n = CPU_COUNT(&set);
            if (n != 0) {
                return n;
            }
        }
#endif
        // Fallback
        return 1;
    }

    String_t getSystemName()
    {
        // uname/utsname is POSIX.1-2001, so it should be widely available
        struct utsname u;
        if (uname(&u) == 0) {
            String_t result = u.sysname;
            result += " ";
            result += u.release;
            return result;
        }
        return "?";
    }
}

#elif defined(TARGET_OS_WIN32)
/*
 *  Win32 implementation
 */

# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
# include "afl/bits/bits.hpp"

namespace {
    size_t getNumberOfProcessors()
    {
        // Normal
        DWORD_PTR processMask, systemMask;
        if (GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask)) {
            size_t n = afl::bits::bitPop(processMask);
            if (n != 0) {
                return n;
            }
        }

        // Fallback
        return 1;
    }

    String_t getSystemName()
    {
        // Getting the Windows version seems to be rocket science with functions being
        // deprecated, returning wrong values, and no well-defined way to obtain the
        // actual name ("Windows 10") other than poking the registry. Keep it simple.
        return "Windows";
    }
}

#else
/*
 *  Unknown target
 */

# error Implement me

#endif

/*
 *  Entry Point
 */

util::SystemInformation
util::getSystemInformation()
{
    SystemInformation result;
    result.numProcessors = getNumberOfProcessors();
    result.operatingSystem = getSystemName();
    return result;
}
