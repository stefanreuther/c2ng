/**
  *  \file game/sim/session.hpp
  */
#ifndef C2NG_GAME_SIM_SESSION_HPP
#define C2NG_GAME_SIM_SESSION_HPP

#include "game/sim/setup.hpp"
#include "game/sim/configuration.hpp"
#include "afl/base/refcounted.hpp"
#include "game/sim/gameinterface.hpp"

namespace game { namespace sim {

    class Session : public afl::base::RefCounted {
     public:
        Session();
        ~Session();

        Setup& setup();
        const Setup& setup() const;

        Configuration& configuration();
        const Configuration& configuration() const;

        void setNewGameInterface(GameInterface* gi);
        GameInterface* getGameInterface() const;

     private:
        Setup m_setup;
        Configuration m_config;
        std::auto_ptr<GameInterface> m_gameInterface;
    };

} }

#endif
