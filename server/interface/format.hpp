/**
  *  \file server/interface/format.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_FORMAT_HPP
#define C2NG_SERVER_INTERFACE_FORMAT_HPP

#include "afl/base/deletable.hpp"
#include "afl/data/value.hpp"
#include "afl/charset/charset.hpp"
#include "afl/base/optional.hpp"

namespace server { namespace interface {

    class Format : public afl::base::Deletable {
     public:
        // PACK out:Format, data, [FORMAT in:Str, CHARSET cs:Str]
        // -> formatName = out
        // -> data       = data
        // -> format     = in [defaults to obj]
        // -> charset    = cs [defaults to latin1]
        // Takes data (=JSON string or object tree) and produces blob
        virtual afl::data::Value* pack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset) = 0;

        // UNPACK in:Format, data, [FORMAT out:Str, CHARSET cs:Str]
        // -> formatName = in
        // -> data       = data
        // -> format     = in [defaults to obj]
        // -> charset    = cs [defaults to latin1]
        // Takes blob and produces JSON string or object tree
        virtual afl::data::Value* unpack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset) = 0;
    };

} }

#endif
