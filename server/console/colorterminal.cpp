/**
  *  \file server/console/colorterminal.cpp
  *  \brief Class server::console::ColorTerminal
  */

#include "server/console/colorterminal.hpp"

void
server::console::ColorTerminal::printBanner()
{
    m_outputStream.writeLine("\033[33;1m|\n|  PlanetsCentral Console\n|\033[0m");
    m_outputStream.flush();
}


void
server::console::ColorTerminal::printPrimaryPrompt(const ContextStack_t& st)
{
    m_outputStream.writeText("\033[36;1m" + packContextStack(st) + ">\033[0m ");
    m_outputStream.flush();
}

void
server::console::ColorTerminal::printSecondaryPrompt()
{
    m_outputStream.writeText("\033[36m(continue...)>\033[0m ");
    m_outputStream.flush();
}

void
server::console::ColorTerminal::printError(String_t msg)
{
    m_errorStream.writeLine("ERROR: \033[31;1m" + msg + "\033[0m");
    m_errorStream.flush();
}

void
server::console::ColorTerminal::printResultPrefix()
{
    m_outputStream.writeText("result=\033[32;1m");
}

void
server::console::ColorTerminal::printResultSuffix()
{
    m_outputStream.writeLine("\033[0m");
    m_outputStream.flush();
}

void
server::console::ColorTerminal::printMessage(String_t s)
{
    m_outputStream.writeLine(s);
    m_outputStream.flush();
}
