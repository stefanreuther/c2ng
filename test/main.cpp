/**
  *  \file test/main.cpp
  *  \brief Entry point for tests
  */

#include <iostream>
#include "afl/test/testrunner.hpp"

int main(int, char** argv)
{
    return afl::test::TestRunner::getInstance().run(std::cout, argv);
}
