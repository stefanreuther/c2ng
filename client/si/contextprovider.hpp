/**
  *  \file client/si/contextprovider.hpp
  */
#ifndef C2NG_CLIENT_SI_CONTEXTPROVIDER_HPP
#define C2NG_CLIENT_SI_CONTEXTPROVIDER_HPP

#include "afl/base/deletable.hpp"
#include "game/session.hpp"
#include "interpreter/process.hpp"

namespace client { namespace si {

    /** Context provider for a script.
        This interface allows creation of contexts for a process.

        ContextProvider instances are passed from the user-interface thread to the worker thread.
        They must therefore not carry any shared data.

        The usual implementation looks at the state of the game session and creates appropriate contexts. */
    class ContextProvider : public afl::base::Deletable {
     public:
        /** Create contexts.
            \param session Game session (input)
            \param proc Process. Contexts should be created here. */
        virtual void createContext(game::Session& session, interpreter::Process& proc) = 0;
    };

} }

#endif
