/**
  *  \file client/imageloader.hpp
  *  \brief Class client::ImageLoader
  */
#ifndef C2NG_CLIENT_IMAGELOADER_HPP
#define C2NG_CLIENT_IMAGELOADER_HPP

#include <vector>
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"

namespace client {

    /** Image loader.
        Normal image loading is asynchronous.
        This class allows you to synchronously load images from the UI thread.
        This needed for example if image dimensions are required for sizing a widget.

        - construct an ImageLoader
        - call loadImage() for all images you need
        - call wait()

        After wait() returns, root.provider().getImage() will return a conclusive result for all images.
        If wait() has to block, it will show a BusyIndicator.

        This means, after wait() you can proceed with getImage() and need not expect these images to change. */
    class ImageLoader {
     public:
        /** Constructor.
            \param root Root
            \param tx Translator */
        explicit ImageLoader(ui::Root& root, afl::string::Translator& tx);

        /** Load an image.
            \param name Resource identifier */
        void loadImage(const String_t& name);

        /** Wait for pending images.
            Returns after all images have been loaded, or if user requested quit.
            \return true on success, false on quit (a Key_Quit is on the input queue)  */
        bool wait();

     private:
        void onImageChange();
        void onQuit();

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::EventLoop m_loop;

        // This vector contains all unloaded images.
        // If an image is already loaded in loadImage(), it is not added.
        // If an image becomes ready in onImageChange(), it is removed.
        // Thus, we need to wait if this is nonempty.
        std::vector<String_t> m_unloadedImages;
    };

}

#endif
