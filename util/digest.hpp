/**
  *  \file util/digest.hpp
  *  \brief Class util::Digest
  */
#ifndef C2NG_UTIL_DIGEST_HPP
#define C2NG_UTIL_DIGEST_HPP

#include "afl/checksums/checksum.hpp"

namespace util {

    /** PHost digest.
        This checksum is used by PHost to compute hashes over specification files.

        Note that this class precomputes a helper table which takes some cycles.
        Therefore, you should keep around an instance for longer time or use the default instance.
        Since the object has no runtime state, using the default instance is as good as any. */
    class Digest : public afl::checksums::Checksum {
     public:
        /** Constructor. */
        Digest();

        /** Destructor. */
        virtual ~Digest();

        /** Compute checksum.
            \param data Data block to compute checksum over.
            \param prev Previous checksum.
            \return updated checksum */
        uint32_t add(Memory_t data, uint32_t prev) const;

        /** Get number of bits in checksum.
            \return Number of bits (32) */
        size_t bits() const;

        /** Access default instance.
            \return default instance */
        static Digest& getDefaultInstance();

     protected:
        virtual uint32_t addImpl(Memory_t data, uint32_t prev) const;
        virtual size_t bitsImpl() const;

     private:
        void init();

        uint32_t m_table[256];
    };

}

#endif
