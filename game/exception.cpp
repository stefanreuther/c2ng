/**
  *  \file game/exception.cpp
  *  \brief Class game::Exception
  */

#include "game/exception.hpp"

/* Standard error codes, for script side. */
const char game::Exception::eFacility[] = "Facility not available",
    game::Exception::eNotOwner[] = "Not owner",
    game::Exception::eRange[] = "Range error",
    game::Exception::ePerm[] = "Not allowed",
    game::Exception::eNoBase[] = "No base",
    game::Exception::eNoResource[] = "No resources",
    game::Exception::ePos[] = "Different position",
    game::Exception::ePartial[] = "Partial argument specification",
    game::Exception::eUser[] = "No race loaded",
    game::Exception::eFleet[] = "Fleet member",
    game::Exception::eNotFleet[] = "Not a fleet member",
    game::Exception::eGraph[] = "Not in graphics mode",
    game::Exception::eDone[] = "Action already performed",
    game::Exception::eNotPlaying[] = "Race not being played";

game::Exception::Exception(String_t error)
    : m_scriptError(error)
{ }

game::Exception::~Exception() throw()
{ }

const char*
game::Exception::what() const throw()
{
    // ex GError::what, GError::getScriptError
    return m_scriptError.c_str();
}
