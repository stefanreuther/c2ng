/**
  *  \file gfx/graphicsexception.cpp
  *  \brief Class gfx::GraphicsException
  */

#include "gfx/graphicsexception.hpp"

// Construct from string.
gfx::GraphicsException::GraphicsException(const String_t& what)
    : std::runtime_error(what)
{ }

// Construct from literal.
gfx::GraphicsException::GraphicsException(const char* what)
    : std::runtime_error(what)
{ }

// Copy constructor.
gfx::GraphicsException::GraphicsException(const GraphicsException& e)
    : std::runtime_error(e)
{ }

// Destructor.
gfx::GraphicsException::~GraphicsException() throw()
{ }
