/**
  *  \file interpreter/simplecontext.hpp
  *  \brief Class interpreter::SimpleContext
  */
#ifndef C2NG_INTERPRETER_SIMPLECONTEXT_HPP
#define C2NG_INTERPRETER_SIMPLECONTEXT_HPP

#include "interpreter/context.hpp"

namespace interpreter {

    /** Simple context that ignores entered/left callbacks.
        Used to simplify implementation of contexts that do not need these, which are most. */
    class SimpleContext : public Context {
     public:
        // Context:
        virtual void onContextEntered(Process& proc);
        virtual void onContextLeft();
    };

}

#endif
