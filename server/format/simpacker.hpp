/**
  *  \file server/format/simpacker.hpp
  */
#ifndef C2NG_SERVER_FORMAT_SIMPACKER_HPP
#define C2NG_SERVER_FORMAT_SIMPACKER_HPP

#include "server/format/packer.hpp"

namespace server { namespace format {

    class SimPacker : public Packer {
     public:
        virtual String_t pack(afl::data::Value* data, afl::charset::Charset& cs);
        virtual afl::data::Value* unpack(const String_t& data, afl::charset::Charset& cs);
    };

} }

#endif
