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
game::interface::RichTextValue::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    // ex IntRichTextValue::store
    rejectStore(out, aux, ctx);
}

// Value:
game::interface::RichTextValue*
game::interface::RichTextValue::clone() const
{
    return new RichTextValue(m_value);
}

game::interface::RichTextValue::Ref_t
game::interface::RichTextValue::get() const
{
    return m_value;
}
