/**
  *  \file game/interface/userinterfacepropertystack.cpp
  *  \brief Class game::interface::UserInterfacePropertyStack
  */

#include <memory>
#include "game/interface/userinterfacepropertystack.hpp"
#include "interpreter/error.hpp"
#include "game/interface/userinterfacepropertyaccessor.hpp"

// Constructor.
game::interface::UserInterfacePropertyStack::UserInterfacePropertyStack()
    : m_stack()
{ }

// Destructor.
game::interface::UserInterfacePropertyStack::~UserInterfacePropertyStack()
{ }

// Add property accessor.
void
game::interface::UserInterfacePropertyStack::add(UserInterfacePropertyAccessor& a)
{
    m_stack.push_back(&a);
}

// Remove property accessor.
void
game::interface::UserInterfacePropertyStack::remove(UserInterfacePropertyAccessor& a)
{
    size_t i = m_stack.size();
    while (i > 0) {
        --i;
        if (m_stack[i] == &a) {
            m_stack.erase(m_stack.begin() + i);
            break;
        }
    }
}

// Get property.
afl::data::Value*
game::interface::UserInterfacePropertyStack::get(UserInterfaceProperty p) const
{
    std::auto_ptr<afl::data::Value> result;
    size_t i = m_stack.size();
    while (i > 0) {
        --i;
        if (m_stack[i]->get(p, result)) {
            break;
        }
    }
    return result.release();
}

// Set property.
void
game::interface::UserInterfacePropertyStack::set(UserInterfaceProperty p, const afl::data::Value* value)
{
    size_t i = m_stack.size();
    bool ok = false;
    while (i > 0) {
        --i;
        if (m_stack[i]->set(p, value)) {
            ok = true;
            break;
        }
    }
    if (!ok) {
        throw interpreter::Error::notAssignable();
    }
}
