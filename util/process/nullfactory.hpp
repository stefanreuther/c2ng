/**
  *  \file util/process/nullfactory.hpp
  *  \brief Class util::process::NullFactory
  */
#ifndef C2NG_UTIL_PROCESS_NULLFACTORY_HPP
#define C2NG_UTIL_PROCESS_NULLFACTORY_HPP

#include "util/process/factory.hpp"

namespace util { namespace process {

    /** Factory for creating dummy (disfunctional) Subprocess instances.
        The subprocess will fail every call other than stop(). */
    class NullFactory : public Factory {
     public:
        virtual Subprocess* createNewProcess();
    };

} }

#endif
