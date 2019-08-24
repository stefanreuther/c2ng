/**
  *  \file client/si/outputstate.hpp
  */
#ifndef C2NG_CLIENT_SI_OUTPUTSTATE_HPP
#define C2NG_CLIENT_SI_OUTPUTSTATE_HPP

#include "client/si/requestlink2.hpp"
#include "afl/string/string.hpp"

namespace client { namespace si {

    class OutputState {
     public:
        enum Target {
            NoChange,
            ExitProgram,
            ExitGame,
            PlayerScreen,
            ShipScreen,
            PlanetScreen,
            BaseScreen,
            ShipTaskScreen,
            PlanetTaskScreen,
            BaseTaskScreen,
            Starchart
        };

        OutputState();

        void set(RequestLink2 p, Target t);

        RequestLink2 getProcess() const;
        Target getTarget() const;

        static String_t toString(OutputState::Target target);

     private:
        RequestLink2 m_process;
        Target m_target;
    };

} }

#endif
