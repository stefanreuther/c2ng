/**
  *  \file gfx/sdl2/nullpresenter.cpp
  */

#include "config.h"
#ifdef HAVE_SDL2
# include "gfx/sdl2/nullpresenter.hpp"

gfx::sdl2::NullPresenter::NullPresenter()
{ }

gfx::sdl2::NullPresenter::~NullPresenter()
{ }

void
gfx::sdl2::NullPresenter::present(SDL_Surface* /*sfc*/, const Rectangle& /*region*/)
{ }

#else
int g_dummyToMakeGfxSdl2NullPresenterNotEmpty;
#endif
