/**
  *  \file interpreter/simplespecialcommand.cpp
  *  \brief Class interpreter::SimpleSpecialCommand
  */

#include "interpreter/simplespecialcommand.hpp"

// Constructor.
interpreter::SimpleSpecialCommand::SimpleSpecialCommand(Compile_t function)
    : m_function(function)
{
    // IntSimpleSpecialCommand::IntSimpleSpecialCommand
}

// Destructor.
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
