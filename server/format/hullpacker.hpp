/**
  *  \file server/format/hullpacker.hpp
  *  \brief Class server::format::HullPacker
  */
#ifndef C2NG_SERVER_FORMAT_HULLPACKER_HPP
#define C2NG_SERVER_FORMAT_HULLPACKER_HPP

#include "server/format/packer.hpp"

namespace server { namespace format {

    /** Packer for HULLSPEC files.
        Packs or unpacks any number of HULLSPEC records. */
    class HullPacker : public Packer {
     public:
        // Packer:
        virtual String_t pack(afl::data::Value* data, afl::charset::Charset& cs);
        virtual afl::data::Value* unpack(const String_t& data, afl::charset::Charset& cs);
    };

} }

#endif
