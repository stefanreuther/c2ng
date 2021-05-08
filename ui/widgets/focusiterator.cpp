/**
  *  \file ui/widgets/focusiterator.cpp
  *  \brief Class ui::widgets::FocusIterator
  */

#include "ui/widgets/focusiterator.hpp"

const int ui::widgets::FocusIterator::Horizontal;
const int ui::widgets::FocusIterator::Vertical;
const int ui::widgets::FocusIterator::Tab;
const int ui::widgets::FocusIterator::Page;
const int ui::widgets::FocusIterator::Home;
const int ui::widgets::FocusIterator::Wrap;


// Constructor.
ui::widgets::FocusIterator::FocusIterator(int flags)
    : InvisibleWidget(),
      m_flags(flags),
      m_widgets()
{
    // ex UIFocusIterator::UIFocusIterator
    // avoid that someone gives us focus
    setState(DisabledState, true);
}

// Destructor.
ui::widgets::FocusIterator::~FocusIterator()
{ }

// Add widget to FocusIterator.
void
ui::widgets::FocusIterator::add(Widget& w)
{
    m_widgets.push_back(&w);
}

// Handle keypress.
bool
ui::widgets::FocusIterator::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex UIFocusIterator::handleEvent
    // Classify the key
    enum { Forward, Backward } direction;
    int type;
    switch (key) {
     case util::Key_Tab:                         type = Tab;        direction = Forward;  break;
     case util::Key_Tab + util::KeyMod_Shift:    type = Tab;        direction = Backward; break;

     case util::Key_Up:                          type = Vertical;   direction = Backward; break;
     case util::Key_Down:                        type = Vertical;   direction = Forward;  break;

     case util::Key_Left:                        type = Horizontal; direction = Backward; break;
     case util::Key_Right:                       type = Horizontal; direction = Forward;  break;

     case util::Key_Home:                        type = Home;       direction = Forward;  break;
     case util::Key_Home + util::KeyMod_Ctrl:    type = Home;       direction = Forward;  break;
     case util::Key_End:                         type = Home;       direction = Backward; break;
     case util::Key_End + util::KeyMod_Ctrl:     type = Home;       direction = Backward; break;

     case util::Key_PgUp:                        type = Page;       direction = Forward;  break;
     case util::Key_PgDn:                        type = Page;       direction = Backward; break;

     default:                                    type = 0;          direction = Forward;  break;
    }

    // Process it
    const size_t n = m_widgets.size();
    if ((type & m_flags) != 0 && n > 0) {
        // Locate the focused widget
        bool ok = false;
        size_t index = 0;
        for (size_t i = 0; i < n; ++i) {
            if (m_widgets[i]->hasState(FocusedState)) {
                index = i;
                ok = true;
            }
        }

        // If it is none of ours, stop
        if (!ok) {
            return false;
        }

        // Regular keys will have to do one step.
        // Page/Home keys go to end, and only do steps if the end widget is not accessible.
        bool mustGo = true;
        if ((type & (Page | Home)) != 0) {
            // Direction has been set for the following walk already (Forward=go to top and then forward).
            if (direction == Forward) {
                index = 0;
            } else {
                index = n-1;
            }
            mustGo = false;
        }

        // We can wrap only if configured so, and even then only once.
        bool canWrap = (m_flags & Wrap) != 0 || (type & Tab) != 0;
        bool found = true;
        while (mustGo || m_widgets[index]->hasState(DisabledState)) {
            // Need to go another step
            if (direction == Forward) {
                ++index;
                if (index >= n) {
                    if (canWrap) {
                        canWrap = false;
                        index = 0;
                    } else {
                        found = false;
                        break;
                    }
                }
            } else {
                if (index <= 0) {
                    if (canWrap) {
                        canWrap = false;
                        index = n;
                    } else {
                        found = false;
                        break;
                    }
                }
                --index;
            }

            // Did a step; reset marker
            mustGo = false;
        }

        // Did we find a widget?
        if (found) {
            requestActive();
            m_widgets[index]->requestFocus();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}
