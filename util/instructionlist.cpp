/**
  *  \file util/instructionlist.cpp
  *  \brief Class util::InstructionList
  */

#include "util/instructionlist.hpp"

namespace {
    inline uint16_t unpackInstruction(int32_t p)
    {
        return static_cast<uint16_t>(static_cast<uint32_t>(p) >> 16);
    }

    inline size_t unpackParameterCount(int32_t p)
    {
        return static_cast<uint32_t>(p) & 0xFFFFU;
    }

    inline int32_t packInstruction(uint16_t insn, size_t argc)
    {
        return static_cast<int32_t>((static_cast<uint32_t>(insn) << 16) + argc);
    }
}


util::InstructionList::Iterator::Iterator(const InstructionList& parent)
    : m_parent(parent),
      m_nextInstruction(0),
      m_nextParameter(0)
{ }

bool
util::InstructionList::Iterator::readInstruction(Instruction_t& insn)
{
    if (m_nextInstruction < m_parent.m_data.size()) {
        const Parameter_t pair = m_parent.m_data[m_nextInstruction++];
        const size_t argc = unpackParameterCount(pair);
        insn = unpackInstruction(pair);
        m_nextParameter = m_nextInstruction;
        m_nextInstruction += argc;
        return true;
    } else {
        return false;
    }
}

bool
util::InstructionList::Iterator::readParameter(Parameter_t& param)
{
    if (m_nextParameter < m_nextInstruction && m_nextParameter < m_parent.m_data.size()) {
        param = m_parent.m_data[m_nextParameter++];
        return true;
    } else {
        return false;
    }
}


util::InstructionList::InstructionList()
    : m_data(),
      m_lastInstruction()
{ }

util::InstructionList::~InstructionList()
{ }

util::InstructionList&
util::InstructionList::addInstruction(Instruction_t insn)
{
    m_lastInstruction = m_data.size();
    m_data.push_back(packInstruction(insn, 0));
    return *this;
}

util::InstructionList&
util::InstructionList::addParameter(Parameter_t param)
{
    if (m_lastInstruction < m_data.size()) {
        const Parameter_t oldPair = m_data[m_lastInstruction];
        const Instruction_t oldInsn = unpackInstruction(oldPair);
        const Parameter_t newPair = packInstruction(oldInsn, unpackParameterCount(oldPair) + 1);
        if (unpackInstruction(newPair) == oldInsn) {
            m_data[m_lastInstruction] = newPair;
            m_data.push_back(param);
        } else {
            // Too many parameters. Ignore.
        }
    } else {
        // No instruction written so far. Ignore.
    }
    return *this;
}

void
util::InstructionList::clear()
{
    m_data.clear();
    m_lastInstruction = 0;
}

void
util::InstructionList::append(const InstructionList& other)
{
    m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
}

size_t
util::InstructionList::size() const
{
    return m_data.size();
}

util::InstructionList::Iterator
util::InstructionList::read() const
{
    return Iterator(*this);
}

void
util::InstructionList::swap(InstructionList& other)
{
    m_data.swap(other.m_data);
    std::swap(m_lastInstruction, other.m_lastInstruction);
}

