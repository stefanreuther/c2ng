/**
  *  \file u/t_util_answerprovider.cpp
  *  \brief Test for util::AnswerProvider
  */

#include "util/answerprovider.hpp"

#include "t_util.hpp"

/** Interface test. */
void
TestUtilAnswerProvider::testIt()
{
    class Tester : public util::AnswerProvider {
     public:
        virtual Result ask(int /*questionId*/, String_t /*question*/)
            { return Yes; }
    };
    Tester t;
}

