/**
  *  \file ui/res/factory.hpp
  */
#ifndef C2NG_UI_RES_FACTORY_HPP
#define C2NG_UI_RES_FACTORY_HPP

#include "afl/io/filesystem.hpp"

namespace ui { namespace res {

    class Provider;

    Provider* createProvider(String_t name, String_t baseDirectory, afl::io::FileSystem& fs);

} }

#endif
