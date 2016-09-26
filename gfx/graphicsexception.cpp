/**
  *  \file gfx/graphicsexception.cpp
  */

#include "gfx/graphicsexception.hpp"

gfx::GraphicsException::GraphicsException(const String_t& what)
    : std::runtime_error(what)
{ }

gfx::GraphicsException::GraphicsException(const char* what)
    : std::runtime_error(what)
{ }

gfx::GraphicsException::GraphicsException(const GraphicsException& e)
    : std::runtime_error(e)
{ }

gfx::GraphicsException::~GraphicsException() throw()
{ }
