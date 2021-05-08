/**
  *  \file util/rich/attribute.hpp
  *  \brief Base class util::rich::Attribute
  */
#ifndef C2NG_UTIL_RICH_ATTRIBUTE_HPP
#define C2NG_UTIL_RICH_ATTRIBUTE_HPP

#include "afl/base/clonable.hpp"
#include "afl/string/string.hpp"

namespace util { namespace rich {

    class Text;

    /** Base class for a rich-text attribute.
        An attribute is associated with a span of text. */
    class Attribute : public afl::base::Clonable<Attribute> {
        friend class Text;
     public:
        Attribute()
            : m_start(0), m_end(0)
            { }

     private:
        String_t::size_type m_start;
        String_t::size_type m_end;
    };

} }

#endif
