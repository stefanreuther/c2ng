/**
  *  \file interpreter/singlecontext.hpp
  */
#ifndef C2NG_INTERPRETER_SINGLECONTEXT_HPP
#define C2NG_INTERPRETER_SINGLECONTEXT_HPP

#include "interpreter/simplecontext.hpp"

namespace interpreter {

    /** Single context.
        This is a context that tells its user that it does not support iteration. */
    class SingleContext : public SimpleContext {
     public:
        virtual bool next();
    };

}


#endif
