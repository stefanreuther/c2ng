/**
  *  \file util/process/factory.hpp
  *  \brief Interface util::process::Factory
  */
#ifndef C2NG_UTIL_PROCESS_FACTORY_HPP
#define C2NG_UTIL_PROCESS_FACTORY_HPP

#include "afl/base/deletable.hpp"

namespace util { namespace process {

    class Subprocess;

    /** Base class for creating Subprocess descendants. */
    class Factory : public afl::base::Deletable {
     public:
        /** Create Subprocess object.
            \return Newly-allocated Subprocess object; never null */
        virtual Subprocess* createNewProcess() = 0;
    };

} }

#endif
