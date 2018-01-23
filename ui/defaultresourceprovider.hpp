/**
  *  \file ui/defaultresourceprovider.hpp
  *  \brief Class ui::DefaultResourceProvider
  */
#ifndef C2NG_UI_DEFAULTRESOURCEPROVIDER_HPP
#define C2NG_UI_DEFAULTRESOURCEPROVIDER_HPP

#include <map>
#include <list>
#include "afl/io/directory.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/sys/thread.hpp"
#include "gfx/fontlist.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/res/manager.hpp"
#include "util/requestdispatcher.hpp"
#include "util/request.hpp"
#include "afl/container/ptrqueue.hpp"

namespace ui {

    /** Default resource provider implementation.
        Implements the gfx::ResourceProvider interface using a ui::res::Manager and a background thread. */
    class DefaultResourceProvider : public gfx::ResourceProvider,
                                    private afl::base::Stoppable
    {
     public:
        /** Constructor.
            Initializes the object, starts the background thread, and preloads all fonts.
            \param mgr Resource manager (responsible for actual resource loading)
            \param dir Resource directory.
            \param mainThreadDispatcher Dispatcher for the main (UI) thread to place callbacks properly.
                       Must out-live the DefaultResourceProvider. */
        DefaultResourceProvider(ui::res::Manager& mgr,
                                afl::base::Ref<afl::io::Directory> dir,
                                util::RequestDispatcher& mainThreadDispatcher,
                                afl::string::Translator& tx,
                                afl::sys::LogListener& log);

        /** Destructor. */
        ~DefaultResourceProvider();

        /** Post a request to operate on the Resource Manager.
            The request will be executed in the worker thread.
            \param req Request
            \param invalidateCache true to invalidate the image cache after this request */
        void postNewManagerRequest(util::Request<ui::res::Manager>* req, bool invalidateCache);

        // ResourceProvider:
        virtual afl::base::Ptr<gfx::Canvas> getImage(String_t name, bool* status = 0);
        virtual afl::base::Ref<gfx::Font> getFont(gfx::FontRequest req);

     private:
        /** Resource manager. */
        ui::res::Manager& m_manager;

        /** Font list. All fonts are pre-loaded. */
        gfx::FontList m_fontList;
        afl::base::Ref<gfx::Font> m_defaultFont;

        /** Main thread dispatcher to place callbacks in UI thread. */
        util::RequestDispatcher& m_mainThreadDispatcher;

        /** Logger. */
        afl::sys::LogListener& m_log;

        /** Translator. */
        afl::string::Translator& m_translator;

        /** Loader (background) thread. */
        afl::sys::Thread m_loaderThread;

        /*
         *  Data shared with background thread
         */

        /** Mutex that protects the variables below. */
        afl::sys::Mutex m_imageMutex;

        /** Loaded images. */
        std::map<String_t, afl::base::Ptr<gfx::Canvas> > m_imageCache;

        /** Queue of images to load. */
        std::list<String_t> m_imageQueue;

        afl::container::PtrQueue<util::Request<ui::res::Manager> > m_managerRequests;
        bool m_managerInvalidate;

        /** Semaphore to wake the background thread. Essentially tracks the m_imageQueue length. */
        afl::sys::Semaphore m_loaderWake;

        /** Stop request. */
        bool m_loaderStopRequest;

        void init(afl::io::Directory& dir);
        void addFont(afl::io::Directory& dir, const char* name, const gfx::FontRequest& defn);

        virtual void run();
        virtual void stop();

        util::Request<ui::res::Manager>* pullManagerRequest();
    };

}

#endif
