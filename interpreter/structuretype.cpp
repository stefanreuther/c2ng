/**
  *  \file interpreter/structuretype.cpp
  */

#include "interpreter/structuretype.hpp"
#include "interpreter/error.hpp"

interpreter::StructureTypeData::StructureTypeData()
{
    // ex IntStructureTypeData::IntStructureTypeData
}

interpreter::StructureTypeData::~StructureTypeData()
{
    // ex IntStructureTypeData::~IntStructureTypeData
}




interpreter::StructureType::StructureType(afl::base::Ptr<StructureTypeData> type)
    : m_type(type)
{
    // ex IntStructureType::IntStructureType
}

interpreter::StructureType::~StructureType()
{
    // ex IntStructureType::~IntStructureType
}

// IntValue:
String_t
interpreter::StructureType::toString(bool /*readable*/) const
{
    // ex IntStructureType::toString
    return "#<struct-type>";
}

void
interpreter::StructureType::store(TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, SaveContext* /*ctx*/) const
{
    // FIXME: port this (IntStructureType::store)
    // ex IntStructureType::store
//     IntVMSaveContext* vsc = IntVMSaveContext::getCurrentInstance();
//     if (vsc != 0) {
//         sv.tag   = IntTagNode::Tag_StructType;
//         sv.value = vsc->addStructureType(*type);
//     } else {
    throw Error::notSerializable();
//     }
}

interpreter::StructureType*
interpreter::StructureType::clone() const
{
    // ex IntStructureType::clone
    return new StructureType(m_type);
}
