/**
  *  \file interpreter/contextreceiver.hpp
  *  \brief Interface interpreter::ContextReceiver
  */
#ifndef C2NG_INTERPRETER_CONTEXTRECEIVER_HPP
#define C2NG_INTERPRETER_CONTEXTRECEIVER_HPP

namespace interpreter {

    class Context;

    /** Context receiver.
        This is an interface to consume contexts created by someone.
        The ContextReceiver typically wraps an interpreter::Process,
        but can also be something else for special tasks. */
    class ContextReceiver {
     public:
        virtual ~ContextReceiver()
            { }

        /** Add context.
            \param pContext Context. ContextReceiver takes ownership. */
        virtual void pushNewContext(Context* pContext) = 0;
    };

}

#endif
