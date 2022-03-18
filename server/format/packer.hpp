/**
  *  \file server/format/packer.hpp
  *  \brief Interface server::format::Packer
  */
#ifndef C2NG_SERVER_FORMAT_PACKER_HPP
#define C2NG_SERVER_FORMAT_PACKER_HPP

#include "afl/base/deletable.hpp"
#include "afl/data/value.hpp"
#include "afl/charset/charset.hpp"

namespace server { namespace format {

    /** Binary data packer/unpacker.
        Each derived class of this interface allows packing (structured data to binary blob)
        and unpacking (structured binary blob to data) of a given file format.
        The actual structured file format is defined by the class.

        Some general principles:
        - excess data in the structured data (afl::data::Value) is ignored when packing
        - files containing a number-of-X will generally accept any number of X including 0,
          even if the standard file has a fixed number of records. */
    class Packer : public afl::base::Deletable {
     public:
        /** Pack structured data into binary.
            Throws std::runtime_error on error.
            \param data Structured data (array, string, whatever)
            \param cs Character set
            \return packed data */
        virtual String_t pack(afl::data::Value* data, afl::charset::Charset& cs) = 0;

        /** Unpack binary into structured data.
            Throws std::runtime_error on error.
            \param data binary data
            \param cs Character set
            \return structured data */
        virtual afl::data::Value* unpack(const String_t& data, afl::charset::Charset& cs) = 0;
    };

} }

#endif
