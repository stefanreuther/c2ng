/**
  *  \file util/constantanswerprovider.cpp
  */

#include "util/constantanswerprovider.hpp"

util::ConstantAnswerProvider util::ConstantAnswerProvider::sayYes(true);
util::ConstantAnswerProvider util::ConstantAnswerProvider::sayNo(false);

util::AnswerProvider::Result
util::ConstantAnswerProvider::ask(int, String_t)
{
    return m_answer ? Yes : No;
}
