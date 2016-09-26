/**
  *  \file client/si/inputstate.hpp
  */
#ifndef C2NG_CLIENT_SI_INPUTSTATE_HPP
#define C2NG_CLIENT_SI_INPUTSTATE_HPP

#include "client/si/requestlink2.hpp"

namespace client { namespace si {

    class InputState {
     public:
        InputState();

        void setProcess(RequestLink2 p);
        RequestLink2 getProcess() const;

     private:
        RequestLink2 m_process;
    };

} }

#endif
