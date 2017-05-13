/**
  *  \file util/syntax/highlighter.hpp
  */
#ifndef C2NG_UTIL_SYNTAX_HIGHLIGHTER_HPP
#define C2NG_UTIL_SYNTAX_HIGHLIGHTER_HPP

#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"

namespace util { namespace syntax {

    class Segment;

    /** Syntax highlighter, base class.
        To highlight some piece of text,
        - call init()
        - call scan() until it returns false

        Each scan() produces one segment of text with highlighting information.

    //     Derived classes override init() and scan().
    //     scan() can use Segment's methods to build the segment,
    //     either set(), or start()+finish().
    //     It can also assign the members manually.
    */
    class Highlighter : public afl::base::Deletable {
     public:
        /** Initialize.
            Sets the string to parse and resets the state.
            The string is not copied. */
        virtual void init(afl::string::ConstStringMemory_t text) = 0;

        /** Extract a segment.
            \param result [out] Space for result
            \return true if a result was produced, false if the end was reached */
        virtual bool scan(Segment& result) = 0;
    };

} }

#endif
