/**
  *  \file game/v3/trn/parseexception.hpp
  *  \brief Class game::v3::trn::ParseException
  */
#ifndef C2NG_GAME_V3_TRN_PARSEEXCEPTION_HPP
#define C2NG_GAME_V3_TRN_PARSEEXCEPTION_HPP

#include <stdexcept>
#include "afl/string/string.hpp"

namespace game { namespace v3 { namespace trn {

    /** Exception that reports a syntax error while parsing a filter expression. */
    class ParseException : public std::runtime_error {
     public:
        /** Constructor.
            \param what Message */
        explicit ParseException(const String_t& what)
            : std::runtime_error(what)
            { }
    };

} } }

#endif
