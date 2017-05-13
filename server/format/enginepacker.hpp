/**
  *  \file server/format/enginepacker.hpp
  *  \brief Class server::format::EnginePacker
  */
#ifndef C2NG_SERVER_FORMAT_ENGINEPACKER_HPP
#define C2NG_SERVER_FORMAT_ENGINEPACKER_HPP

#include "server/format/packer.hpp"

namespace server { namespace format {

    /** Packer for ENGSPEC files.
        Packs or unpacks any number of ENGSPEC records. */
    class EnginePacker : public Packer {
     public:
        // Packer:
        virtual String_t pack(afl::data::Value* data, afl::charset::Charset& cs);
        virtual afl::data::Value* unpack(const String_t& data, afl::charset::Charset& cs);
    };

} }

#endif
