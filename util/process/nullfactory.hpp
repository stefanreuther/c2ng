/**
  *  \file util/process/nullfactory.hpp
  */
#ifndef C2NG_UTIL_PROCESS_NULLFACTORY_HPP
#define C2NG_UTIL_PROCESS_NULLFACTORY_HPP

#include "util/process/factory.hpp"

namespace util { namespace process {

    /** Factory for creating dummy (disfunctional) Subprocess instances. */
    class NullFactory : public Factory {
     public:
        virtual Subprocess* createNewProcess();
    };

} }

#endif
