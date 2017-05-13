/**
  *  \file server/format/beampacker.hpp
  *  \brief Class server::format::BeamPacker
  */
#ifndef C2NG_SERVER_FORMAT_BEAMPACKER_HPP
#define C2NG_SERVER_FORMAT_BEAMPACKER_HPP

#include "server/format/packer.hpp"

namespace server { namespace format {

    /** Packer for BEAMSPEC files.
        Packs or unpacks any number of BEAMSPEC records. */
    class BeamPacker : public Packer {
     public:
        // Packer:
        virtual String_t pack(afl::data::Value* data, afl::charset::Charset& cs);
        virtual afl::data::Value* unpack(const String_t& data, afl::charset::Charset& cs);
    };

} }

#endif
