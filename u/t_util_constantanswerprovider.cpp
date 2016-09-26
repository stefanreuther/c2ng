/**
  *  \file u/t_util_constantanswerprovider.cpp
  *  \brief Test for util::ConstantAnswerProvider
  */

#include "util/constantanswerprovider.hpp"

#include "t_util.hpp"

/** Test on-the-fly-construction. */
void
TestUtilConstantAnswerProvider::testIt()
{
    TS_ASSERT_EQUALS(util::ConstantAnswerProvider(true).ask(0, ""), util::AnswerProvider::Yes);
    TS_ASSERT_EQUALS(util::ConstantAnswerProvider(true).ask(99, "hi"), util::AnswerProvider::Yes);
    TS_ASSERT_EQUALS(util::ConstantAnswerProvider(false).ask(0, ""), util::AnswerProvider::No);
    TS_ASSERT_EQUALS(util::ConstantAnswerProvider(false).ask(99, "hi"), util::AnswerProvider::No);
}

void
TestUtilConstantAnswerProvider::testYes()
{
    TS_ASSERT_EQUALS(util::ConstantAnswerProvider::sayNo.ask(0, ""), util::AnswerProvider::No);
    TS_ASSERT_EQUALS(util::ConstantAnswerProvider::sayNo.ask(99, "hi"), util::AnswerProvider::No);
}

void
TestUtilConstantAnswerProvider::testNo()
{
    TS_ASSERT_EQUALS(util::ConstantAnswerProvider::sayYes.ask(0, ""), util::AnswerProvider::Yes);
    TS_ASSERT_EQUALS(util::ConstantAnswerProvider::sayYes.ask(99, "hi"), util::AnswerProvider::Yes);
}
