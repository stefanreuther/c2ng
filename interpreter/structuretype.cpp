/**
  *  \file interpreter/structuretype.cpp
  *  \brief Class interpreter::StructureType
  */

#include "interpreter/structuretype.hpp"
#include "interpreter/savecontext.hpp"

// Constructor.
interpreter::StructureType::StructureType(StructureTypeData::Ref_t type)
    : m_type(type)
{
    // ex IntStructureType::IntStructureType
}

// Destructor.
interpreter::StructureType::~StructureType()
{
    // ex IntStructureType::~IntStructureType
}

String_t
interpreter::StructureType::toString(bool /*readable*/) const
{
    // ex IntStructureType::toString
    return "#<struct-type>";
}

void
interpreter::StructureType::store(TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, SaveContext& ctx) const
{
    // ex IntStructureType::store
    out.tag   = TagNode::Tag_StructType;
    out.value = ctx.addStructureType(*m_type);
}

interpreter::StructureType*
interpreter::StructureType::clone() const
{
    // ex IntStructureType::clone
    return new StructureType(m_type);
}
