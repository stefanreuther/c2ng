/**
  *  \file test_apps/processrunner.cpp
  */

#include <iostream>
#include <cstring>
#include "util/processrunner.hpp"

int main(int, char** argv)
{
    util::ProcessRunner runner;
    util::ProcessRunner::Command cmd;
    for (int i = 1; argv[i] != 0; ++i) {
        if (std::strncmp(argv[i], "-cd=", 4) == 0) {
            cmd.workDirectory = argv[i]+4;
        } else {
            cmd.command.push_back(argv[i]);
        }
    }
    String_t output;
    int exit = runner.run(cmd, output);

    std::cout << "Output: <<" << output << ">>\nExit code: " << exit << "\n";
}
