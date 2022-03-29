/**
  *  \file game/interface/contextprovider.hpp
  *  \brief Interface game::interface::ContextProvider
  */
#ifndef C2NG_GAME_INTERFACE_CONTEXTPROVIDER_HPP
#define C2NG_GAME_INTERFACE_CONTEXTPROVIDER_HPP

#include "afl/base/deletable.hpp"
#include "game/session.hpp"
#include "interpreter/contextreceiver.hpp"

namespace game { namespace interface {

    /** Context provider for a script.
        This interface allows creation of contexts for a process or other.

        ContextProvider instances are passed from the user-interface thread to the worker thread.
        They must therefore not carry any shared data.

        The usual implementation looks at the state of the game session and creates appropriate contexts. */
    class ContextProvider : public afl::base::Deletable {
     public:
        /** Create contexts.
            \param session Game session (input)
            \param recv Context receiver. Call pushNewContext(). */
        virtual void createContext(Session& session, interpreter::ContextReceiver& recv) = 0;
    };

} }

#endif
