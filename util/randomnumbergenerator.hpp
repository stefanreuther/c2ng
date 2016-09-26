/**
  *  \file util/randomnumbergenerator.hpp
  */
#ifndef C2NG_UTIL_RANDOMNUMBERGENERATOR_HPP
#define C2NG_UTIL_RANDOMNUMBERGENERATOR_HPP

#include "afl/base/types.hpp"

namespace util {

    /** Random number generator.
        Instead of relying on the vague semantics of std::rand(), we use our own, simple random number generator.
        This one is expected to have a period of 4G (in other words, enough for us).

        This generator can be instantiated at will.
        Use your own instances if you need a pseudo-random stream for otherwise deterministic behaviour (e.g. TRN encryption).
        Use a possibly-shared instance for things should look random, like visual effects. */
    class RandomNumberGenerator {
     public:
        explicit RandomNumberGenerator(uint32_t seed);

        /** Get random number in range [0, 2^16). */
        uint16_t operator()();

        /** Get random number in range [0, max). */
        uint16_t operator()(uint16_t max);

        void setSeed(uint32_t seed);
        uint32_t getSeed() const;

     private:
        uint32_t m_seed;

        void advance();
    };

}

#endif
