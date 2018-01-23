/**
  *  \file server/console/nullterminal.cpp
  *  \brief Class server::console::NullTerminal
  */

#include "server/console/nullterminal.hpp"

void
server::console::NullTerminal::printBanner()
{ }

void
server::console::NullTerminal::printPrimaryPrompt(const ContextStack_t& /*st*/)
{ }

void
server::console::NullTerminal::printSecondaryPrompt()
{ }

void
server::console::NullTerminal::printError(String_t /*msg*/)
{ }

void
server::console::NullTerminal::printResultPrefix()
{ }

void
server::console::NullTerminal::printResultSuffix()
{ }

void
server::console::NullTerminal::printMessage(String_t /*s*/)
{ }
