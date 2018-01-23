/**
  *  \file server/console/pipeterminal.cpp
  *  \brief Class server::console::PipeTerminal
  */

#include "server/console/pipeterminal.hpp"

void
server::console::PipeTerminal::printBanner()
{ }

void
server::console::PipeTerminal::printPrimaryPrompt(const ContextStack_t& /*st*/)
{ }

void
server::console::PipeTerminal::printSecondaryPrompt()
{ }

void
server::console::PipeTerminal::printError(String_t msg)
{
    m_errorStream.writeLine("ERROR: " + msg);
}

void
server::console::PipeTerminal::printResultPrefix()
{
    m_outputStream.writeText("result=");
}

void
server::console::PipeTerminal::printResultSuffix()
{
    m_outputStream.writeLine();
}

void
server::console::PipeTerminal::printMessage(String_t s)
{
    m_outputStream.writeLine(s);
    m_outputStream.flush();
}
