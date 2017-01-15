/**
  *  \file gfx/nullresourceprovider.cpp
  *  \brief Class gfx::NullResourceProvider
  */

#include "gfx/nullresourceprovider.hpp"
#include "gfx/defaultfont.hpp"

gfx::NullResourceProvider::NullResourceProvider()
    : ResourceProvider(),
      m_font(createDefaultFont())
{ }

gfx::NullResourceProvider::~NullResourceProvider()
{ }

afl::base::Ptr<gfx::Canvas>
gfx::NullResourceProvider::getImage(String_t /*name*/, bool* status)
{
    if (status) {
        *status = true;
    }
    return 0;
}

afl::base::Ref<gfx::Font>
gfx::NullResourceProvider::getFont(FontRequest /*req*/)
{
    return m_font;
}
