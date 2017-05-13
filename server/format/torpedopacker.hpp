/**
  *  \file server/format/torpedopacker.hpp
  *  \brief Class server::format::TorpedoPacker
  */
#ifndef C2NG_SERVER_FORMAT_TORPEDOPACKER_HPP
#define C2NG_SERVER_FORMAT_TORPEDOPACKER_HPP

#include "server/format/packer.hpp"

namespace server { namespace format {

    /** Packer for TORPSPEC files.
        Packs or unpacks any number of TORPSPEC records. */
    class TorpedoPacker : public Packer {
     public:
        // Packer:
        virtual String_t pack(afl::data::Value* data, afl::charset::Charset& cs);
        virtual afl::data::Value* unpack(const String_t& data, afl::charset::Charset& cs);
    };

} }

#endif
