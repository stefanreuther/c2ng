/**
  *  \file client/si/contextprovider.hpp
  */
#ifndef C2NG_CLIENT_SI_CONTEXTPROVIDER_HPP
#define C2NG_CLIENT_SI_CONTEXTPROVIDER_HPP

#include "afl/base/deletable.hpp"
#include "game/session.hpp"
#include "interpreter/process.hpp"

namespace client { namespace si {

    class ContextReceiver;

    /** Context provider for a script.
        This interface allows creation of contexts for a process or other.

        ContextProvider instances are passed from the user-interface thread to the worker thread.
        They must therefore not carry any shared data.

        The usual implementation looks at the state of the game session and creates appropriate contexts. */
    class ContextProvider : public afl::base::Deletable {
     public:
        /** Create contexts.
            \param session Game session (input)
            \param recv Context receiver. Call addNewContext(). */
        virtual void createContext(game::Session& session, ContextReceiver& recv) = 0;
    };

} }

#endif
