/**
  *  \file ui/res/winplanbitmapprovider.cpp
  */

#include "ui/res/winplanbitmapprovider.hpp"
#include "afl/string/format.hpp"
#include "ui/res/manager.hpp"
#include "ui/res/resid.hpp"

ui::res::WinplanBitmapProvider::WinplanBitmapProvider(afl::base::Ref<afl::io::Directory> dir)
    : m_directory(dir)
{ }

afl::base::Ptr<gfx::Canvas>
ui::res::WinplanBitmapProvider::loadImage(String_t name, Manager& mgr)
{
    // ex ResProviderWinplanBitmaps::loadPixmap

    /* We do not produce a resource for ship type #200. In genuine
       Winplan picture packs, this is a planet image. It is therefore
       not used in regular hullspec.dat files. However, the simulator
       uses 200 for custom ships. */
    int imageNumber;
    if (matchResourceId(name, SHIP, imageNumber) && imageNumber != 200) {
        afl::base::Ptr<afl::io::Stream> s = openResourceFile(*m_directory, afl::string::Format("vpl%d.", imageNumber), graphicsSuffixes());
        if (s.get() != 0) {
            return mgr.loadImage(*s);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
