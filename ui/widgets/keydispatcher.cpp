/**
  *  \file ui/widgets/keydispatcher.cpp
  */

#include "ui/widgets/keydispatcher.hpp"

// /** Constructor. Makes a blank UIKeyTranslator which doesn't yet translate any keys. */
ui::widgets::KeyDispatcher::KeyDispatcher()
    : m_keys()
{
    // ex UIKeyTranslator::UIKeyTranslator
}

ui::widgets::KeyDispatcher::~KeyDispatcher()
{ }

void
ui::widgets::KeyDispatcher::addNewClosure(Key_t key, Closure_t* closure)
{
    // ex UIKeyTranslator::add (sort-of)
    m_keys.insertNew(key, closure);
}

// Widget:
bool
ui::widgets::KeyDispatcher::handleKey(util::Key_t key, int prefix)
{
    // ex UIKeyTranslator::handleEvent
    // Difference to PCC2: UIKeyTranslator only handled keys in second pass.
    // Because we have better dispatch semantic (focused sees first), we can always handle the keys.
    if (Closure_t* p = m_keys[key]) {
        requestActive();
        p->call(prefix);
        dropActive();
        return true;
    } else {
        return false;
    }
}

bool
ui::widgets::KeyDispatcher::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}
