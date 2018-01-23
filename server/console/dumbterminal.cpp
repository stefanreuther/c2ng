/**
  *  \file server/console/dumbterminal.cpp
  *  \brief Class server::console::DumbTerminal
  */

#include "server/console/dumbterminal.hpp"

void
server::console::DumbTerminal::printBanner()
{
    m_outputStream.writeLine("|\n|  PlanetsCentral Console\n|");
    m_outputStream.flush();
}

void
server::console::DumbTerminal::printPrimaryPrompt(const ContextStack_t& st)
{
    m_outputStream.writeText(packContextStack(st) + "> ");
    m_outputStream.flush();
}

void
server::console::DumbTerminal::printSecondaryPrompt()
{
    m_outputStream.writeText("(continue...)> ");
    m_outputStream.flush();
}

void
server::console::DumbTerminal::printError(String_t msg)
{
    m_errorStream.writeLine("ERROR: " + msg);
    m_errorStream.flush();
}

void
server::console::DumbTerminal::printResultPrefix()
{
    m_outputStream.writeText("result=");
}

void
server::console::DumbTerminal::printResultSuffix()
{
    m_outputStream.writeLine();
    m_outputStream.flush();
}

void
server::console::DumbTerminal::printMessage(String_t s)
{
    m_outputStream.writeLine(s);
    m_outputStream.flush();
}
