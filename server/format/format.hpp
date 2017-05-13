/**
  *  \file server/format/format.hpp
  */
#ifndef C2NG_SERVER_FORMAT_FORMAT_HPP
#define C2NG_SERVER_FORMAT_FORMAT_HPP

#include "server/interface/format.hpp"

namespace server { namespace format {

    class Format : public server::interface::Format {
     public:
        Format();
        ~Format();
        virtual afl::data::Value* pack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset);
        virtual afl::data::Value* unpack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset);
    };

} }

#endif
