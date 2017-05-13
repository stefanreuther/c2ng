/**
  *  \file server/format/truehullpacker.hpp
  *  \brief Class server::format::TruehullPacker
  */
#ifndef C2NG_SERVER_FORMAT_TRUEHULLPACKER_HPP
#define C2NG_SERVER_FORMAT_TRUEHULLPACKER_HPP

#include "server/format/packer.hpp"

namespace server { namespace format {

    /** Packer for TRUEHULL files.
        Packs or unpacks a complete (11x20) TRUEHULL file into a vector-of-vectors. */
    class TruehullPacker : public Packer {
     public:
        // Packer:
        virtual String_t pack(afl::data::Value* data, afl::charset::Charset& cs);
        virtual afl::data::Value* unpack(const String_t& data, afl::charset::Charset& cs);
    };

} }

#endif
