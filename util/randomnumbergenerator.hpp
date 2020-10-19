/**
  *  \file util/randomnumbergenerator.hpp
  *  \brief Class util::RandomNumberGenerator
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
        Use a possibly-shared instance for things should look random, like visual effects.

        This is the classic Turbo/Delphi RNG which provides a period of 2^32. */
    class RandomNumberGenerator {
     public:
        /** Constructor.
            \param seed Initial seed */
        explicit RandomNumberGenerator(uint32_t seed);

        /** Get random number in range [0, 2^16). */
        uint16_t operator()();

        /** Get random number in range [0, max). */
        uint16_t operator()(uint16_t max);

        /** Set seed.
            \param seed New seed */
        void setSeed(uint32_t seed);

        /** Get seed
            \return seed */
        uint32_t getSeed() const;

     private:
        uint32_t m_seed;

        void advance();
    };

}

#endif
