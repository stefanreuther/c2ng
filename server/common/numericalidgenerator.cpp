/**
  *  \file server/common/numericalidgenerator.cpp
  */

#include "server/common/numericalidgenerator.hpp"
#include "afl/string/format.hpp"

server::common::NumericalIdGenerator::NumericalIdGenerator()
    : m_counter()
{ }

server::common::NumericalIdGenerator::~NumericalIdGenerator()
{ }

String_t
server::common::NumericalIdGenerator::createId()
{
    return afl::string::Format("%d", ++m_counter);
}
