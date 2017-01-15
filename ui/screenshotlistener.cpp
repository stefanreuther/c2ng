/**
  *  \file ui/screenshotlistener.cpp
  *  \brief Class ui::ScreenshotListener
  */

#include "ui/screenshotlistener.hpp"
#include "afl/string/format.hpp"
#include "gfx/save.hpp"
#include "util/translation.hpp"

namespace {
    const char LOG_NAME[] = "ui";
}

ui::ScreenshotListener::ScreenshotListener(afl::io::FileSystem& fs, afl::sys::LogListener& log)
    : m_fileSystem(fs),
      m_log(log)
{ }

// Take a screenshot.
void
ui::ScreenshotListener::call(gfx::Canvas& can)
{
    // ex ui/uiglobal.h:doScreenshot
    int index = 0;
    while (1) {
        // Try creating a file
        String_t fileName = afl::string::Format("file%04d.bmp", ++index);
        afl::base::Ptr<afl::io::Stream> file = m_fileSystem.openFileNT(fileName, afl::io::FileSystem::CreateNew);
        if (file.get() != 0) {
            // OK, save screenshot
            saveCanvas(can, *file);
            file = 0;
            m_log.write(m_log.Info, LOG_NAME, afl::string::Format(_("Screenshot saved as \"%s\"").c_str(), fileName));
            break;
        }
        if (index >= 9999) {
            // NOK, directory is full
            m_log.write(m_log.Error, LOG_NAME, _("Unable to find a free file name for screenshot"));
            break;
        }
    }
}

// Clone this object.
ui::ScreenshotListener*
ui::ScreenshotListener::clone() const
{
    return new ScreenshotListener(m_fileSystem, m_log);
}
