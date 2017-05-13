/**
  *  \file server/format/stringpacker.hpp
  *  \brief Class server::format::StringPacker
  */
#ifndef C2NG_SERVER_FORMAT_STRINGPACKER_HPP
#define C2NG_SERVER_FORMAT_STRINGPACKER_HPP

#include "server/format/packer.hpp"

namespace server { namespace format {

    /** Packer for strings.
        A simple packer that just converts the incoming bytes into a text string.
        This can be used to load text files. */
    class StringPacker : public Packer {
     public:
        // Packer:
        virtual String_t pack(afl::data::Value* data, afl::charset::Charset& cs);
        virtual afl::data::Value* unpack(const String_t& data, afl::charset::Charset& cs);
    };

} }

#endif
