/**
  *  \file interpreter/vmio/structures.cpp
  *  \brief interpreter::vmio Structures
  */

#include "interpreter/vmio/structures.hpp"

interpreter::vmio::structures::ProcessKind::Word_t
interpreter::vmio::structures::ProcessKind::unpack(const Bytes_t& bytes)
{
    switch (bytes[0]) {
     case 0:  return Process::pkDefault;
     case 1:  return Process::pkShipTask;
     case 2:  return Process::pkPlanetTask;
     case 3:  return Process::pkBaseTask;
     default: return Process::pkDefault;
    }
}

void
interpreter::vmio::structures::ProcessKind::pack(Bytes_t& bytes, Word_t word)
{
    uint8_t result = 0;
    switch (word) {
     case Process::pkDefault:    result = 0; break;
     case Process::pkShipTask:   result = 1; break;
     case Process::pkPlanetTask: result = 2; break;
     case Process::pkBaseTask:   result = 3; break;
    }
    bytes[0] = result;
}
