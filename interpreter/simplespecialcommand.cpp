/**
  *  \file interpreter/simplespecialcommand.cpp
  */

#include "interpreter/simplespecialcommand.hpp"

interpreter::SimpleSpecialCommand::SimpleSpecialCommand(Compile_t function)
    : m_function(function)
{
    // IntSimpleSpecialCommand::IntSimpleSpecialCommand
}

interpreter::SimpleSpecialCommand::~SimpleSpecialCommand()
{
    // ex IntSimpleSpecialCommand::~IntSimpleSpecialCommand
}

void
interpreter::SimpleSpecialCommand::compileCommand(Tokenizer& line, BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntSimpleSpecialCommand::compileCommand
    m_function(line, bco, scc);
}
