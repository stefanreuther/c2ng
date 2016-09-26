/**
  *  \file util/rich/linkattribute.hpp
  */
#ifndef C2NG_UTIL_RICH_LINKATTRIBUTE_HPP
#define C2NG_UTIL_RICH_LINKATTRIBUTE_HPP

#include "util/rich/attribute.hpp"

namespace util { namespace rich {

    class LinkAttribute : public Attribute {
     public:
        LinkAttribute(String_t target);
        ~LinkAttribute();
        LinkAttribute* clone() const;

        const String_t& getTarget() const
            { return m_target; }

     private:
        String_t m_target;
    };
} }

#endif
