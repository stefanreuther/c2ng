/**
  *  \file util/rich/visitor.hpp
  *  \brief Base class util::rich::Visitor
  */
#ifndef C2NG_UTIL_RICH_VISITOR_HPP
#define C2NG_UTIL_RICH_VISITOR_HPP

#include "afl/string/string.hpp"

namespace util { namespace rich {

    class Text;
    class Attribute;

    /** Visitor for rich text. */
    class Visitor {
     public:
        Visitor()
            { }
        virtual ~Visitor()
            { }

        /** Handle run of text.
            \param text Text
            \return true to continue iteration, false to exit. */
        virtual bool handleText(String_t text) = 0;

        /** Handle beginning of an attribute.
            \param att the attribute
            \return true to continue iteration, false to exit. */
        virtual bool startAttribute(const Attribute& att) = 0;

        /** Handle end of an attribute. Note that this endAttribute()
            always refers to the last not-yet-ended startAttribute().
            \param att the attribute
            \return true to continue iteration, false to exit. */
        virtual bool endAttribute(const Attribute& att) = 0;

        /** Visit a piece of rich text.
            Calls text.visit().
            \param text Text to observe
            \return *this */
        Visitor& visit(const Text& text);
    };

} }

#endif
