/**
  *  \file gfx/nullengine.cpp
  *  \brief Class gfx::NullEngine
  */

#include <memory>
#include "gfx/nullengine.hpp"
#include "gfx/rgbapixmap.hpp"
#include "afl/sys/mutexguard.hpp"
#include "afl/sys/time.hpp"
#include "gfx/windowparameters.hpp"

gfx::NullEngine::NullEngine()
    : m_timers(),
      m_wake(false),
      m_queueMutex(),
      m_queue()
{ }

gfx::NullEngine::~NullEngine()
{ }

afl::base::Ref<gfx::Canvas>
gfx::NullEngine::createWindow(const WindowParameters& param)
{
    return RGBAPixmap::create(param.size.getX(), param.size.getY())->makeCanvas();
}


afl::base::Ref<gfx::Canvas>
gfx::NullEngine::loadImage(afl::io::Stream& /*file*/)
{
    return RGBAPixmap::create(1, 1)->makeCanvas();
}

// Wait for and handle an event.
void
gfx::NullEngine::handleEvent(EventConsumer& consumer, bool /*relativeMouseMovement*/)
{
    uint32_t t = afl::sys::Time::getTickCounter();
    while (1) {
        bool result = m_wake.wait(m_timers.getNextTimeout());
        uint32_t now = afl::sys::Time::getTickCounter();

        // Process timers
        bool did = m_timers.handleElapsedTime(now - t);
        t = now;

        // Got the semaphore? This means we have an element in the queue.
        if (result) {
            std::auto_ptr<QueueElement> r;
            {
                afl::sys::MutexGuard g(m_queueMutex);
                r.reset(m_queue.extractFront());
            }
            if (r.get()) {
                r->handle(consumer);
            }
            did = true;
        }

        // Did something?
        if (did) {
            break;
        }
    }
}

// Get keyboard modifiers.
util::Key_t
gfx::NullEngine::getKeyboardModifierState()
{
    return 0;
}

// Get request dispatcher.
util::RequestDispatcher&
gfx::NullEngine::dispatcher()
{
    return *this;
}

// Create a user-interface timer.
afl::base::Ref<gfx::Timer>
gfx::NullEngine::createTimer()
{
    return m_timers.createTimer();
}

// Post a key event.
void
gfx::NullEngine::postKey(util::Key_t key, int prefix)
{
    class KeyHandler : public QueueElement {
     public:
        KeyHandler(util::Key_t key, int prefix)
            : m_key(key),
              m_prefix(prefix)
            { }
        void handle(EventConsumer& consumer)
            { consumer.handleKey(m_key, m_prefix); }
     private:
        util::Key_t m_key;
        int m_prefix;
    };
    postNew(new KeyHandler(key, prefix));
}

// Post a mouse event.
void
gfx::NullEngine::postMouse(Point pt, EventConsumer::MouseButtons_t pressedButtons)
{
    class MouseHandler : public QueueElement {
     public:
        MouseHandler(Point pt, EventConsumer::MouseButtons_t pressedButtons)
            : m_point(pt),
              m_pressedButtons(pressedButtons)
            { }
        void handle(EventConsumer& consumer)
            { consumer.handleMouse(m_point, m_pressedButtons); }
     private:
        Point m_point;
        EventConsumer::MouseButtons_t m_pressedButtons;
    };
    postNew(new MouseHandler(pt, pressedButtons));
}

// Post new Runnable.
void
gfx::NullEngine::postNewRunnable(afl::base::Runnable* p)
{
    if (p) {
        class Runner : public QueueElement {
         public:
            Runner(std::auto_ptr<afl::base::Runnable> pp)
                : m_runnable(pp)
                { }
            virtual void handle(EventConsumer&)
                { m_runnable->run(); }
         private:
            std::auto_ptr<afl::base::Runnable> m_runnable;
        };

        std::auto_ptr<afl::base::Runnable> pp(p);
        postNew(new Runner(pp));
    }
}

// Post new action queue element.
void
gfx::NullEngine::postNew(QueueElement* p)
{
    afl::sys::MutexGuard g(m_queueMutex);
    m_queue.pushBackNew(p);
    m_wake.post();
}
