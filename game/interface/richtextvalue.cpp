/**
  *  \file game/interface/richtextvalue.cpp
  */

#include "game/interface/richtextvalue.hpp"
#include "interpreter/error.hpp"

game::interface::RichTextValue::RichTextValue(Ref_t value)
    : m_value(value)
{ }

game::interface::RichTextValue::~RichTextValue()
{ }

// BaseValue:
String_t
game::interface::RichTextValue::toString(bool /*readable*/) const
{
    // ex IntRichTextValue::toString
    return m_value->getText();
}

void
game::interface::RichTextValue::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntRichTextValue::store
    throw interpreter::Error::notSerializable();
}

// Value:
game::interface::RichTextValue*
game::interface::RichTextValue::clone() const
{
    return new RichTextValue(m_value);
}

game::interface::RichTextValue::Ref_t
game::interface::RichTextValue::get()
{
    return m_value;
}
