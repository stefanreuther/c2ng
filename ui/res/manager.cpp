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
    if (p != 0) {
        m_imageLoaders.pushBackNew(p);
    }
}

void
ui::res::Manager::addNewProvider(Provider* p, String_t key)
{
    if (p != 0) {
        std::auto_ptr<Provider> pp(p);
        m_providers.pushBackNew(new ProviderKey(pp, key));
    }
}

afl::base::Ptr<gfx::Canvas>
ui::res::Manager::loadImage(String_t name)
{
    afl::base::Ptr<gfx::Canvas> result;
    for (size_t i = m_providers.size(); i > 0; --i) {
        result = m_providers[i-1]->provider->loadImage(name, *this);
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

void
ui::res::Manager::removeProvidersByKey(String_t key)
{
    size_t out = 0;
    for (size_t i = 0, n = m_providers.size(); i < n; ++i) {
        if (m_providers[i]->key != key) {
            m_providers.swapElements(i, out++);
        }
    }
    m_providers.resize(out);
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
