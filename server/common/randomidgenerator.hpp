/**
  *  \file server/common/randomidgenerator.hpp
  *  \brief Class server::common::RandomIdGenerator
  */
#ifndef C2NG_SERVER_COMMON_RANDOMIDGENERATOR_HPP
#define C2NG_SERVER_COMMON_RANDOMIDGENERATOR_HPP

#include "afl/base/growablememory.hpp"
#include "afl/io/filesystem.hpp"
#include "server/common/idgenerator.hpp"

namespace server { namespace common {

    /** Generate random session Ids.
        Generates Ids consisting of a random hex string.
        Those Ids should be safe to use in an external interface without further validation;
        their length will make Id predictions essentially impossible.

        <b>Theory of operation:</b>

        We obtain some initial entropy from /dev/urandom if available,
        plus the startup time of our server for additional/fallback randomness.

        For each Id, the result buffer is incremented as if it were a big number,
        and its SHA-1 computed to produce the Id. */
    class RandomIdGenerator : public IdGenerator {
     public:
        /** Constructor.
            \param fs Filesystem (required to access /dev/urandom) */
        RandomIdGenerator(afl::io::FileSystem& fs);

        ~RandomIdGenerator();

        virtual String_t createId();

     private:
        afl::base::GrowableMemory<uint8_t> m_state;

        void init(afl::io::FileSystem& fs);
    };

} }

#endif
