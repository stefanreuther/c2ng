/**
  *  \file client/si/contextreceiver.hpp
  */
#ifndef C2NG_CLIENT_SI_CONTEXTRECEIVER_HPP
#define C2NG_CLIENT_SI_CONTEXTRECEIVER_HPP

#include "afl/base/deletable.hpp"
#include "interpreter/context.hpp"

namespace client { namespace si {

    /** Context receiver.
        This interface is used with ContextProvider.
        A ContextProvider pushes its contexts into a ContextReceiver.
        The ContextReceiver typically wraps an interpreter::Process,
        but can also be something else for special tasks such as completion. */
    class ContextReceiver : public afl::base::Deletable {
     public:
        /** Add context.
            \param pContext Context. ContextReceiver takes ownership. */
        virtual void addNewContext(interpreter::Context* pContext) = 0;
    };

} }

#endif
