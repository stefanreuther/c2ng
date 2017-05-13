/**
  *  \file util/syntax/segment.hpp
  */
#ifndef C2NG_UTIL_SYNTAX_SEGMENT_HPP
#define C2NG_UTIL_SYNTAX_SEGMENT_HPP

#include "util/syntax/format.hpp"
#include "afl/string/string.hpp"

namespace util { namespace syntax {

    class Segment {
     public:
        Segment();
        Segment(Format fmt, afl::string::ConstStringMemory_t text);

        void set(Format fmt, afl::string::ConstStringMemory_t text);
        void start(afl::string::ConstStringMemory_t tail);
        void finish(Format fmt, afl::string::ConstStringMemory_t tail);

        void setLink(const String_t& link);
        void setInfo(const String_t& info);
        void setFormat(Format fmt);

        Format getFormat() const;
        afl::string::ConstStringMemory_t getText() const;
        const String_t& getLink() const;
        const String_t& getInfo() const;

     private:
        Format m_format;
        afl::string::ConstStringMemory_t m_text;
        String_t m_link;
        String_t m_info;
    };

} }


#endif
