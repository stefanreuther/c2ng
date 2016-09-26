/**
  *  \file util/translation.hpp
  */
#ifndef C2NG_UTIL_TRANSLATION_HPP
#define C2NG_UTIL_TRANSLATION_HPP

#include "afl/string/translator.hpp"

#define _(x) afl::string::Translator::getSystemInstance().translateString(x)
#define N_(x) (x)

#endif
