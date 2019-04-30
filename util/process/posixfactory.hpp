/**
  *  \file util/process/posixfactory.hpp
  *  \brief Class util::process::PosixFactory
  */
#ifndef C2NG_UTIL_PROCESS_POSIXFACTORY_HPP
#define C2NG_UTIL_PROCESS_POSIXFACTORY_HPP

#include "util/process/factory.hpp"

namespace util { namespace process {

    /** Implementation of Factory/Subprocess for POSIX.

        Limitations as of 20190124 (FIXME):
        - no file descriptor isolation (see ProcessRunner)
        - no character set translation for non UTF-8 locales */
    class PosixFactory : public Factory {
     public:
        virtual Subprocess* createNewProcess();
    };

} }

#endif
