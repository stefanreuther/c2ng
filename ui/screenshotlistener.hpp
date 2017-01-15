/**
  *  \file ui/screenshotlistener.hpp
  *  \brief Class ui::ScreenshotListener
  */
#ifndef C2NG_UI_SCREENSHOTLISTENER_HPP
#define C2NG_UI_SCREENSHOTLISTENER_HPP

#include "afl/base/closure.hpp"
#include "gfx/canvas.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/sys/loglistener.hpp"

namespace ui {

    /** Screenshot Listener.
        This class can be used as a Root::sig_screenshot listener.
        It will save numbered screenshots as *.bmp files using gfx::saveCanvas(). */
    class ScreenshotListener : public afl::base::Closure<void(gfx::Canvas&)> {
     public:
        /** Constructor.
            \param fs File system
            \param log Log listener */
        ScreenshotListener(afl::io::FileSystem& fs, afl::sys::LogListener& log);

        // Closure:

        /** Take a screenshot.
            The screenshot will be saved in current directory of the file system.
            \param can Canvas to save */
        virtual void call(gfx::Canvas& can);

        /** Clone this object.
            \return Copy of this ScreenshotListener. */
        virtual ScreenshotListener* clone() const;

     private:
        afl::io::FileSystem& m_fileSystem;
        afl::sys::LogListener& m_log;
    };

}

#endif
