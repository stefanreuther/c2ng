/**
  *  \file ui/defaultresourceprovider.cpp
  *  \brief Class ui::DefaultResourceProvider
  */

#include <algorithm>
#include "ui/defaultresourceprovider.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/mutexguard.hpp"
#include "gfx/bitmapfont.hpp"
#include "ui/draw.hpp"
#include "ui/res/ccimageloader.hpp"
#include "ui/res/resid.hpp"
#include "gfx/defaultfont.hpp"

namespace {
    const char LOG_NAME[] = "ui.resload";
    const char THREAD_NAME[] = "ui.resload";
}

// Constructor.
// FIXME: get rid of the "dir" parameter
ui::DefaultResourceProvider::DefaultResourceProvider(ui::res::Manager& mgr,
                                                     afl::base::Ref<afl::io::Directory> dir,
                                                     util::RequestDispatcher& mainThreadDispatcher,
                                                     afl::string::Translator& tx,
                                                     afl::sys::LogListener& log)
    : m_manager(mgr),
      m_fontList(),
      m_defaultFont(gfx::createDefaultFont()),
      m_mainThreadDispatcher(mainThreadDispatcher),
      m_log(log),
      m_translator(tx),
      m_loaderThread(THREAD_NAME, *this),
      m_imageMutex(),
      m_imageCache(),
      m_imageQueue(),
      m_managerRequests(),
      m_managerInvalidate(false),
      m_loaderWake(0),
      m_loaderStopRequest(false)
{
    init(*dir);
}

// Destructor.
ui::DefaultResourceProvider::~DefaultResourceProvider()
{
    stop();
    m_loaderThread.join();
}

void
ui::DefaultResourceProvider::postNewManagerRequest(util::Request<ui::res::Manager>* req, bool invalidateCache)
{
    {
        afl::sys::MutexGuard g(m_imageMutex);
        if (req != 0) {
            m_managerRequests.pushBackNew(req);
        }
        m_managerInvalidate |= invalidateCache;
    }
    m_loaderWake.post();
}

// Get image.
afl::base::Ptr<gfx::Canvas>
ui::DefaultResourceProvider::getImage(String_t name, bool* status)
{
    // Check for existing image
    afl::sys::MutexGuard g(m_imageMutex);
    std::map<String_t, afl::base::Ptr<gfx::Canvas> >::iterator it = m_imageCache.find(name);
    if (it != m_imageCache.end()) {
        if (status != 0) {
            *status = true;
        }
        return it->second;
    }

    // Not found; enqueue it
    if (std::find(m_imageQueue.begin(), m_imageQueue.end(), name) == m_imageQueue.end()) {
        m_imageQueue.push_back(name);
        m_loaderWake.post();
    }
    if (status != 0) {
        *status = false;
    }
    return 0;
}

afl::base::Ref<gfx::Font>
ui::DefaultResourceProvider::getFont(gfx::FontRequest req)
{
    afl::base::Ptr<gfx::Font> result = m_fontList.findFont(req);
    if (result.get() == 0) {
        return m_defaultFont;
    } else {
        return *result;
    }
}

void
ui::DefaultResourceProvider::init(afl::io::Directory& dir)
{
    // Load fonts
    addFont(dir, "font1.fnt", gfx::FontRequest().addSize(1));                            // TITLE
    addFont(dir, "font2.fnt", gfx::FontRequest().addWeight(1));                          // NORMAL_BOLD
    addFont(dir, "font3.fnt", gfx::FontRequest().addSize(-1));                           // SMALL
    addFont(dir, "font4.fnt", gfx::FontRequest().setStyle(FixedFont));                   // FIXED
    addFont(dir, "font5.fnt", gfx::FontRequest());                                       // NORMAL
    addFont(dir, "font6.fnt", gfx::FontRequest().addSize(1).addWeight(1));               // TITLE_BOLD
    addFont(dir, "font7.fnt", gfx::FontRequest().addSize(-1).addWeight(1));              // SMALL_BOLD
    addFont(dir, "font8.fnt", gfx::FontRequest().setStyle(FixedFont).addWeight(1));      // FIXED_BOLD
    addFont(dir, "font9.fnt", gfx::FontRequest().addSize(-2));                           // TINY

    // Start background thread
    m_loaderThread.start();
}

void
ui::DefaultResourceProvider::addFont(afl::io::Directory& dir, const char* name, const gfx::FontRequest& defn)
{
    afl::base::Ref<afl::io::Stream> file = dir.openFile(name, afl::io::FileSystem::OpenRead);
    afl::base::Ptr<gfx::BitmapFont> font = new gfx::BitmapFont();
    font->load(*file, 0);
    m_fontList.addFont(defn, font);
}

void
ui::DefaultResourceProvider::run()
{
    while (1) {
        m_loaderWake.wait();

        while (util::Request<ui::res::Manager>* req = pullManagerRequest()) {
            req->handle(m_manager);
            delete req;
        }

        String_t todo;
        {
            afl::sys::MutexGuard g(m_imageMutex);
            if (m_loaderStopRequest) {
                break;
            }

            while (!m_imageQueue.empty()) {
                String_t n = m_imageQueue.front();
                m_imageQueue.pop_front();
                if (m_imageCache.find(n) == m_imageCache.end()) {
                    todo = n;
                    break;
                }
            }
        }

        if (!todo.empty()) {
            // Load it
            afl::base::Ptr<gfx::Canvas> can;
            try {
                String_t id = todo;
                while (1) {
                    can = m_manager.loadImage(id);
                    if (can.get() != 0) {
                        break;
                    }
                    if (!ui::res::generalizeResourceId(id)) {
                        break;
                    }
                }
                if (can.get() == 0) {
                    m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Image \"%s\" not found").c_str(), todo));
                } else {
                    m_log.write(m_log.Trace, LOG_NAME, afl::string::Format(m_translator.translateString("Loaded \"%s\"").c_str(), todo));
                }
            }
            catch (std::exception& e) {
                m_log.write(m_log.Warn, LOG_NAME, todo, e);
            }
            catch (...) {
                m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Unhandled exception while loading \"%s\"").c_str(), todo));
            }

            // Save it
            // for testing: afl::sys::Thread::sleep(1000);
            afl::sys::MutexGuard g(m_imageMutex);
            m_imageCache[todo] = can;

            // Tell caller
            class Signaler : public afl::base::Runnable {
             public:
                Signaler(afl::base::Signal<void()>& sig)
                    : m_sig(sig)
                    { }
                void run()
                    { m_sig.raise(); }
             private:
                afl::base::Signal<void()>& m_sig;
            };
            m_mainThreadDispatcher.postNewRunnable(new Signaler(sig_imageChange));
        }
    }
}

void
ui::DefaultResourceProvider::stop()
{
    {
        afl::sys::MutexGuard g(m_imageMutex);
        m_loaderStopRequest = true;
    }
    m_loaderWake.post();
}

util::Request<ui::res::Manager>*
ui::DefaultResourceProvider::pullManagerRequest()
{
    afl::sys::MutexGuard g(m_imageMutex);
    if (!m_managerRequests.empty()) {
        return m_managerRequests.extractFront();
    }
    if (m_managerInvalidate) {
        m_managerInvalidate = false;
        m_imageCache.clear();
    }
    return 0;
}
