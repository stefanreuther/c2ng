/**
  *  \file ui/res/winplanvcrprovider.cpp
  *  \brief Class ui::res::WinplanVcrProvider
  */

#include "ui/res/winplanvcrprovider.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/io/limitedstream.hpp"
#include "ui/res/manager.hpp"
#include "ui/res/resid.hpp"

ui::res::WinplanVcrProvider::WinplanVcrProvider(afl::base::Ref<afl::io::Stream> file)
    : m_file(file)
{
    // ex ResProviderWinplanVcr::ResProviderWinplanVcr
    init();
}

afl::base::Ptr<gfx::Canvas>
ui::res::WinplanVcrProvider::loadImage(String_t name, Manager& mgr)
{
    // ex ResProviderWinplanVcr::loadPixmap
    // Match supported elements
    const Header* p = 0;
    int index = 0;
    if (matchResourceId(name, VCR_LSHIP, index)) {
        p = &m_header[0];
    } else if (matchResourceId(name, VCR_RSHIP, index)) {
        p = &m_header[1];
    } else {
        p = 0;
    }

    // Found?
    if (p == 0 || index <= 0 || index > NUM) {
        return 0;
    }

    // Do we have this one?
    uint32_t position = p->position[index-1];
    uint16_t size = p->size[index-1];
    if (position == 0 || size == 0) {
        return 0;
    }

    // OK, we have it
    afl::io::LimitedStream s(m_file, position-1, size);
    s.setPos(0);

    afl::base::Ptr<gfx::Canvas> result = mgr.loadImage(s);
    if (result.get() != 0) {
        // FIXME: port this (colorkey)
        // /* I was cropping the image here, but I think that is
        //    counter-productive. It messes up relations between
        //    images, which would be nice to preserve for FLAK. */
        // // FIXME: can we assume this color-keying? I hope, yes. It fails if someone
        // // makes a Cube that fills out its complete bitmap.
        // pix->setColorKey(pix->getPixel(GfxPoint(0, 0)));
    }
    return result;
}

void
ui::res::WinplanVcrProvider::init()
{
    // ex ResProviderWinplanVcr::init [part]
    static_assert(sizeof(Header) == 1000, "sizeof Header");
    m_file->setPos(0);
    m_file->fullRead(afl::base::fromObject(m_header));
}
