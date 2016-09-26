/**
  *  \file game/exception.cpp
  *  \brief Class game::Exception
  */

#include "game/exception.hpp"

/** Standard error codes, for script side. */
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

// /** Error message. This class provides a way to store error messages
//     in a format usable by both the script and user side. */

// /** Constructor. Create an error message.
//     \param script_error  Script error message. Ideally, one of the eXxxx constants.
//     \param user_error    User error message. Should be a translated string. */
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

// /** Get script-side error message. */
const String_t&
game::Exception::getScriptError() const
{
    // ex GError::getScriptError
    return m_scriptError;
}

// /** Get user error message. */
const String_t&
game::Exception::getUserError() const
{
    // ex GError::getUserError
    return m_userError;
}

// /** Get user error message. */
const char*
game::Exception::what() const throw()
{
    // ex GError::what
    return m_userError.c_str();
}


// FIXME: do we still need these?
// /** Constructor. Create a "no-error" status. */
// GError()
//     : script_error(0), user_error(0)
//     { }

// /** Convert to bool.
//     \return true if there was an error. */
// operator bool() const
//     { return script_error != 0; }

// bool isError(const char* which_one) const;

// /** Check equality.
//     \param which_one error we're testing for (one of the eFoo constants).
//     \return true iff this is the error we're asking for */
// bool
// GError::isError(const char* which_one) const
// {
//     return std::strcmp(which_one, script_error) == 0;
// }
