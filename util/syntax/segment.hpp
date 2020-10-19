/**
  *  \file util/syntax/segment.hpp
  *  \brief Class util::syntax::Segment
  */
#ifndef C2NG_UTIL_SYNTAX_SEGMENT_HPP
#define C2NG_UTIL_SYNTAX_SEGMENT_HPP

#include "util/syntax/format.hpp"
#include "afl/string/string.hpp"

namespace util { namespace syntax {

    /** Segment of highlighted text.
        Stores a piece of text in form of a ConstStringMemory_t (=pointer into original text),
        format, and meta-information.

        To build a segment, do either of the following:
        - construct it
        - call set()
        - call start(), then finish()
        and the optionally use setLink(), setInfo(), setFormat() to adjust it.

        \see Highlighter::scan(). */
    class Segment {
     public:
        /** Default constructor.
            Make an empty segment.  */
        Segment();

        /** Make specific segment.
            \param fmt Format
            \param text Text */
        Segment(Format fmt, afl::string::ConstStringMemory_t text);

        /** Set content.
            \param fmt Format
            \param text Text */
        void set(Format fmt, afl::string::ConstStringMemory_t text);

        /** Start a segment.
            \param tail Buffer starting with the first character that is part of this segment */
        void start(afl::string::ConstStringMemory_t tail);

        /** Finish a segment.
            \param fmt Format
            \param tail Buffer starting with the first character that is NOT part of this segment. */
        void finish(Format fmt, afl::string::ConstStringMemory_t tail);

        /** Set associated link.
            \param link Link */
        void setLink(const String_t& link);

        /** Set associated information text.
            \param info text */
        void setInfo(const String_t& info);

        /** Set format.
            \param fmt Format */
        void setFormat(Format fmt);

        /** Get format.
            \return format */
        Format getFormat() const;

        /** Get text content.
            \return reference to text */
        afl::string::ConstStringMemory_t getText() const;

        /** Get associated link.
            \return Link */
        const String_t& getLink() const;

        /** Get associated information text.
            \return text */
        const String_t& getInfo() const;

     private:
        Format m_format;
        afl::string::ConstStringMemory_t m_text;
        String_t m_link;
        String_t m_info;
    };

} }


#endif
