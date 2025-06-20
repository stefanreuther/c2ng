/**
  *  \file util/processrunnerapplet.cpp
  *  \brief Class util::ProcessRunnerApplet
  */

#include "util/processrunnerapplet.hpp"

#include "afl/string/format.hpp"
#include "util/processrunner.hpp"
#include "util/string.hpp"

int
util::ProcessRunnerApplet::run(Application& app, afl::sys::Environment::CommandLine_t& cmdl)
{
    ProcessRunner runner;
    ProcessRunner::Command cmd;

    String_t it;
    while (cmdl.getNextElement(it)) {
        if (const char* e = strStartsWith(it, "-cd=")) {
            cmd.workDirectory = e;
        } else {
            cmd.command.push_back(it);
        }
    }

    String_t output;
    int exit = runner.run(cmd, output);

    app.standardOutput().writeText("Output: <<");
    app.standardOutput().writeText(output);
    app.standardOutput().writeLine(afl::string::Format(">>\nExit code: %d", exit));
    return 0;
}
