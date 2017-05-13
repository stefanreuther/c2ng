/**
  *  \file u/files.hpp
  */
#ifndef C2NG_U_FILES_HPP
#define C2NG_U_FILES_HPP

#include "afl/base/memory.hpp"

afl::base::ConstBytes_t getResultFile30();
afl::base::ConstBytes_t getResultFile35();

afl::base::ConstBytes_t getSimFileV0();
afl::base::ConstBytes_t getSimFileV1();
afl::base::ConstBytes_t getSimFileV2();
afl::base::ConstBytes_t getSimFileV3();
afl::base::ConstBytes_t getSimFileV4();
afl::base::ConstBytes_t getSimFileV5();

afl::base::ConstBytes_t getDefaultRegKey();

afl::base::ConstBytes_t getDefaultRaceNames();

#endif
