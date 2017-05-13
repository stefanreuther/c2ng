/**
  *  \file server/errors.hpp
  *  \brief Error Messages
  *
  *  This file defines error message strings.
  *  Every component from the server/ area that reports an error must use these strings.
  *  This ensures consistent behaviour.
  *
  *  Error messages are modeled after HTTP error messages.
  *  If a command produces multiple errors that need to be handled differently, they have different numeric codes.
  *  In any case, the text is informative and no guarantees are made.
  *  Applications also must expect receiving messages with no numeric code.
  *
  *  Note: defining these as "const char*const" allows the compiler to merge the strings across the whole program.
  *  Defining them as "const char[]" would not allow that, it would define a new array instance in every translation unit.
  *  Because consts are implicitly static, only the strings that are used in a binary appear in it.
  */
#ifndef C2NG_SERVER_ERRORS_HPP
#define C2NG_SERVER_ERRORS_HPP

namespace server {

    // 400 Bad Request
    const char*const UNKNOWN_COMMAND             = "400 Unknown command";
    const char*const INVALID_SORT_KEY            = "400 Invalid sort key";
    const char*const INVALID_OPTION              = "400 Invalid option";
    const char*const SYNTAX_ERROR                = "400 Syntax error";
    const char*const INVALID_NUMBER_OF_ARGUMENTS = "400 Invalid number of arguments";
    const char*const INVALID_RECEIVER            = "400 Invalid receiver";
    const char*const BAD_REQUEST                 = "400 Bad request";

    // 401 Unauthorized
    const char*const INVALID_USERNAME            = "401 Invalid user name or password";
    const char*const INVALID_PASSWORD            = "401 Invalid user name or password";

    // 403 Forbidden
    const char*const PERMISSION_DENIED           = "403 Permission denied";
    const char*const NOT_AUTHOR                  = "403 Not author";
    const char*const MUST_HAVE_USER_CONTEXT      = "403 Must have user context";
    const char*const USER_NOT_ALLOWED            = "403 USER not allowed";

    // 404 Not found. We reserve the possibility to distinguish the type in the error message, but don't do that yet.
    const char*const FORUM_NOT_FOUND             = "404 Not found";
    const char*const GROUP_NOT_FOUND             = "404 Not found";
    const char*const SYNTAX_NOT_FOUND            = "404 Not found";
    const char*const TOPIC_NOT_FOUND             = "404 Not found";
    const char*const MESSAGE_NOT_FOUND           = "404 Not found";
    const char*const FOLDER_NOT_FOUND            = "404 Not found";
    const char*const PM_NOT_FOUND                = "404 Not found";
    const char*const FILE_NOT_FOUND              = "404 Not found";

    // 405 Method not allowed
    const char*const NOT_A_DIRECTORY             = "405 Not a directory";

    // 409 Conflict
    const char*const ALREADY_EXISTS              = "409 Already exists";

    // 412 Precondition failed
    const char*const NO_RECEIVERS                = "412 No receivers";

    // 413 Request Entity Too Large
    const char*const FILE_TOO_LARGE              = "413 File too large";

    // 415 Unsupported Media Type
    const char*const INVALID_CHARSET             = "415 Invalid charset";
    const char*const INVALID_DATA_TYPE           = "415 Invalid data type";
    const char*const INVALID_FILE_TYPE           = "415 Invalid file type";

    // 422 Unprocessable Entity
    const char*const INVALID_FILE_FORMAT         = "422 Invalid file format";

}

#endif
