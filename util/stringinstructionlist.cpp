/**
  *  \file util/stringinstructionlist.cpp
  *  \brief Class util::StringInstructionList
  */

#include "util/stringinstructionlist.hpp"

util::StringInstructionList::Iterator::Iterator(const StringInstructionList& parent)
    : InstructionList::Iterator(parent),
      m_parent(parent)
{ }

bool
util::StringInstructionList::Iterator::readStringParameter(String_t& param)
{
    Parameter_t intParam;
    if (readParameter(intParam)) {
        size_t index = static_cast<size_t>(intParam);
        if (index < m_parent.m_strings.size()) {
            param = m_parent.m_strings[index];
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

util::StringInstructionList::StringInstructionList()
    : InstructionList(),
      m_strings()
{ }

util::StringInstructionList::~StringInstructionList()
{ }

util::StringInstructionList&
util::StringInstructionList::addInstruction(Instruction_t insn)
{
    InstructionList::addInstruction(insn);
    return *this;
}

util::StringInstructionList&
util::StringInstructionList::addParameter(Parameter_t param)
{
    InstructionList::addParameter(param);
    return *this;
}

util::StringInstructionList&
util::StringInstructionList::addStringParameter(const String_t& s)
{
    size_t n = m_strings.size();
    m_strings.push_back(s);
    addParameter(static_cast<Parameter_t>(n));
    return *this;
}

void
util::StringInstructionList::clear()
{
    InstructionList::clear();
    m_strings.clear();
}

util::StringInstructionList::Iterator
util::StringInstructionList::read() const
{
    return Iterator(*this);
}

void
util::StringInstructionList::swap(StringInstructionList& other)
{
    InstructionList::swap(other);
    m_strings.swap(other.m_strings);
}
