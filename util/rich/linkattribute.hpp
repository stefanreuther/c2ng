/**
  *  \file util/rich/linkattribute.hpp
  *  \brief Class util::rich::LinkAttribute
  */
#ifndef C2NG_UTIL_RICH_LINKATTRIBUTE_HPP
#define C2NG_UTIL_RICH_LINKATTRIBUTE_HPP

#include "util/rich/attribute.hpp"

namespace util { namespace rich {

    /** Link attribute.
        Attaches a string describing a link target to a piece of text.
        The interpretation of the link target string is up to the user. */
    class LinkAttribute : public Attribute {
     public:
        /** Constructor.
            \param target Link target */
        explicit LinkAttribute(String_t target);

        /** Destructor. */
        ~LinkAttribute();

        /** Clone.
            \return newly-allocated identical copy of this attribute */
        LinkAttribute* clone() const;

        /** Get link target.
            \return link target */
        const String_t& getTarget() const;

     private:
        const String_t m_target;
    };
} }

inline const String_t&
util::rich::LinkAttribute::getTarget() const
{
    return m_target;
}

#endif
