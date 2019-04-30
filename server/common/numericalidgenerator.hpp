/**
  *  \file server/common/numericalidgenerator.hpp
  *  \brief Class server::common::NumericalIdGenerator
  */
#ifndef C2NG_SERVER_COMMON_NUMERICALIDGENERATOR_HPP
#define C2NG_SERVER_COMMON_NUMERICALIDGENERATOR_HPP

#include "server/common/idgenerator.hpp"

namespace server { namespace common {

    /** Numerical Id generator.
        Creates Ids as normal increasing integers.
        This is the classic version which is useful for debugging due to its predictability,
        but cannot be used without further protection on external interfaces. */
    class NumericalIdGenerator : public IdGenerator {
     public:
        NumericalIdGenerator();

        ~NumericalIdGenerator();

        virtual String_t createId();

     private:
        uint32_t m_counter;
    };

} }

#endif
