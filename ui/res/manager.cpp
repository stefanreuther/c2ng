/**
  *  \file ui/res/manager.cpp
  */

#include "ui/res/manager.hpp"
#include "ui/res/imageloader.hpp"
#include "ui/res/provider.hpp"

ui::res::Manager::Manager()
    : m_imageLoaders(),
      m_providers(),
      m_screenSize(320, 200)
{ }

ui::res::Manager::~Manager()
{ }

void
ui::res::Manager::addNewImageLoader(ImageLoader* p)
{
    assert(p);
    m_imageLoaders.pushBackNew(p);
}

void
ui::res::Manager::addNewProvider(Provider* p)
{
    assert(p);
    m_providers.pushBackNew(p);
}

afl::base::Ptr<gfx::Canvas>
ui::res::Manager::loadImage(String_t name)
{
    afl::base::Ptr<gfx::Canvas> result;
    for (size_t i = 0, n = m_providers.size(); i < n; ++i) {
        result = m_providers[i]->loadImage(name, *this);
        if (result.get() != 0) {
            break;
        }
    }
    return result;
}

afl::base::Ptr<gfx::Canvas>
ui::res::Manager::loadImage(afl::io::Stream& s)
{
    afl::base::Ptr<gfx::Canvas> result;
    for (size_t i = 0, n = m_imageLoaders.size(); i < n; ++i) {
        result = m_imageLoaders[i]->loadImage(s);
        if (result.get() != 0) {
            break;
        }
    }
    return result;
}

gfx::Point
ui::res::Manager::getScreenSize() const
{
    return m_screenSize;
}

void
ui::res::Manager::setScreenSize(gfx::Point sz)
{
    m_screenSize = sz;
}
