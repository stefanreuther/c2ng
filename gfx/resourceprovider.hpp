/**
  *  \file gfx/resourceprovider.hpp
  *  \brief Interface gfx::ResourceProvider
  */
#ifndef C2NG_GFX_RESOURCEPROVIDER_HPP
#define C2NG_GFX_RESOURCEPROVIDER_HPP

#include "afl/base/deletable.hpp"
#include "gfx/canvas.hpp"
#include "afl/string/string.hpp"
#include "gfx/font.hpp"
#include "gfx/fontrequest.hpp"
#include "afl/base/signal.hpp"

namespace gfx {

    /** Resource provider.
        This interface allows UI components to access resources:
        - images
        - fonts
        Note that these methods are true "get" methods that shall not block.
        See method description for details. */
    class ResourceProvider : public afl::base::Deletable {
     public:
        /** Get an image.
            This method shall return the image identified by the given name.
            Multiple calls with the same name should return the same instance of the image (sharing).

            This method shall not block.
            If an image is requested that is not currently available, this method shall initiate loading of the image in the background;
            status is provided using the optional status parameter.
            If more images become available, sig_imageChange is raised; see there.

            Summary:
            - image available: returns a non-null pointer, *status=true
            - image known to not be available: returns a null pointer, *status=true
            - image not available, availability not known: returns a null pointer, *status=false

            \param name [in] Image identifier
            \param status [out,optional] true: return value is final; false: return value may change after next sig_imageChange call
            \return image, if any */
        virtual afl::base::Ptr<Canvas> getImage(String_t name, bool* status = 0) = 0;

        /** Get a font.
            This method shall return a font that best satisfies the given font request.
            Multiple calls with the same request should return the same instance of the font (sharing).

            This method shall not block.
            Currently, there is no provision for background font loading, meaning all fonts must be preloaded.
            This method must not return a null pointer.
            It should try hard to obtain a font, even if it does not precisely match the request.
            If absolutely no matching font is found, it can return a default font (createDefaultFont()).

            \param req Font request
            \return font */
        virtual afl::base::Ref<Font> getFont(FontRequest req) = 0;

        /** Image change.
            This signal is raised when background-loaded images become available.
            This signal is called in the UI thread.
            Components that display images can hook this signal to be notified when their loading completes, or images change.

            If multiple load requests appear, it is the responsibility of the ResourceProvider implementation,
            not the consumer, to debounce the signal. */
        afl::base::Signal<void()> sig_imageChange;
    };

}

#endif
