/**
  *  \file util/rich/attribute.hpp
  */
#ifndef C2NG_UTIL_RICH_ATTRIBUTE_HPP
#define C2NG_UTIL_RICH_ATTRIBUTE_HPP

#include "afl/string/string.hpp"
#include "afl/base/clonable.hpp"

namespace util { namespace rich {

    class Text;

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
