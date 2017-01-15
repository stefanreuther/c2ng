/**
  *  \file gfx/nullengine.hpp
  *  \brief Class gfx::NullEngine
  */
#ifndef C2NG_GFX_NULLENGINE_HPP
#define C2NG_GFX_NULLENGINE_HPP

#include "gfx/engine.hpp"
#include "util/requestdispatcher.hpp"
#include "gfx/timerqueue.hpp"
#include "afl/container/ptrqueue.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "util/key.hpp"
#include "gfx/point.hpp"
#include "gfx/eventconsumer.hpp"

namespace gfx {

    /** Null graphics engine.
        This class is mainly intended for testing.
        It implements the Engine interface but provides no actual graphics output and no user input.
        It does implement proper timers and a possibility to enqueue synthetic user input. */
    class NullEngine : public Engine, private util::RequestDispatcher {
     public:
        NullEngine();
        ~NullEngine();

        // Engine:

        /** Create a window.
            NullEngine will create a RGBAPixmap.
            \param width Width of window, pixels
            \param height Height of window, pixels
            \param bpp Bits per pixel (8/16/24/32 to pick a specific width; 0 to pick the default)
            \param flags Flags
            \return newly-allocated RGBAPixmap */
        virtual afl::base::Ref<Canvas> createWindow(int width, int height, int bpp, WindowFlags_t flags);

        /** Load an image file.
            NullEngine will return an empty 1x1 RGBAPixmap for all requests.
            \param file File (ignored by NullEngine)
            \return image */
        virtual afl::base::Ref<Canvas> loadImage(afl::io::Stream& file);

        /** Wait for and handle an event.
            Returns when an event has been processed.
            \param consumer Event consumer
            \param relativeMouseMovement Ignored by NullEngine */
        virtual void handleEvent(EventConsumer& consumer, bool relativeMouseMovement);

        /** Get request dispatcher.
            Actions posted into the request dispatcher will be executed in the GUI thread,
            and cause handleEvent() to return.
            \return Event dispatcher */
        virtual util::RequestDispatcher& dispatcher();

        /** Create a user-interface timer.
            The timer callback will execute from within handleEvent() and cause it to return.
            \return newly-allocated timer; never null */
        virtual afl::base::Ref<Timer> createTimer();

        // NullEngine:
        /** Post a key event.
            Causes handleEvent() to eventually call handleKey() on its consumer.
            Events are ordered, i.e. this is a queue, not a stack.
            \param key the key
            \param prefix prefix argument */
        void postKey(util::Key_t key, int prefix);

        /** Post a mouse event.
            Causes handleEvent() to eventually call handleMouse() on its consumer.
            Events are ordered, i.e. this is a queue, not a stack.
            Note that there is no way to deal with handleEvent()'s relativeMouseMovement argument.
            \param pt position
            \param pressedButtons button state */
        void postMouse(Point pt, EventConsumer::MouseButtons_t pressedButtons);

     private:
        /** Action queue element (for RequestDispatcher personality).
            We need to pass around an EventConsumer to be able to use this for implementing postKey/postMouse. */
        class QueueElement : public afl::base::Deletable {
         public:
            virtual void handle(EventConsumer&) = 0;
        };

        /** Timer queue. Implements createTimer(). */
        TimerQueue m_timers;

        /** Action queue wakeup semaphore. */
        afl::sys::Semaphore m_wake;

        /** Action queue mutex. */
        afl::sys::Mutex m_queueMutex;

        /** Action queue (for RequestDispatcher personality). */
        afl::container::PtrQueue<QueueElement> m_queue;

        // RequestDispatcher:
        virtual void postNewRunnable(afl::base::Runnable* p);

        // NullEngine:
        /** Post new action queue element.
            \param p newly-allocated element. Must not be null. */
        void postNew(QueueElement* p);
    };

}

#endif
