/**
  *  \file interpreter/keymapvalue.cpp
  *  \brief Class interpreter::KeymapValue
  */

#include "interpreter/keymapvalue.hpp"
#include "interpreter/error.hpp"

// Constructor.
interpreter::KeymapValue::KeymapValue(util::KeymapRef_t keymap)
    : m_keymap(keymap)
{ }

// Destructor.
interpreter::KeymapValue::~KeymapValue()
{ }

// Get keymap.
util::KeymapRef_t
interpreter::KeymapValue::getKeymap() const
{
    // ex IntKeymapValue::getKeymap
    return m_keymap;
}

String_t
interpreter::KeymapValue::toString(bool /*readable*/) const
{
    // ex IntKeymapValue::toString
    return "#<keymap:" + m_keymap->getName() + ">";
}

void
interpreter::KeymapValue::store(TagNode& /*out*/, afl::io::DataSink& /*aux*/, SaveContext& /*ctx*/) const
{
    // ex IntKeymapValue::store
    throw Error::notSerializable();
}

interpreter::KeymapValue*
interpreter::KeymapValue::clone() const
{
    // ex IntKeymapValue::clone()
    return new KeymapValue(m_keymap);
}

interpreter::KeymapValue*
interpreter::makeKeymapValue(util::KeymapRef_t km)
{
    // ex int/keymap.h:makeKeymapValue
    if (km != 0) {
        return new KeymapValue(km);
    } else {
        return 0;
    }
}
