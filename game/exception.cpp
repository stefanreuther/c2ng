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
    game::Exception::eGraph[] = "Not in graphics mode",
    game::Exception::eDone[] = "Action already performed",
    game::Exception::eNotPlaying[] = "Race not being played";

game::Exception::Exception(String_t scriptError, String_t userError)
    : m_scriptError(scriptError),
      m_userError(userError)
{
    // ex GError::GError
}

game::Exception::Exception(String_t error)
    : m_scriptError(error),
      m_userError(error)
{ }

game::Exception::~Exception() throw()
{ }

const String_t&
game::Exception::getScriptError() const
{
    // ex GError::getScriptError
    return m_scriptError;
}

const String_t&
game::Exception::getUserError() const
{
    // ex GError::getUserError
    return m_userError;
}

const char*
game::Exception::what() const throw()
{
    // ex GError::what
    return m_userError.c_str();
}
