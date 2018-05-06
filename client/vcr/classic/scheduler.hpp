/**
  *  \file client/vcr/classic/scheduler.hpp
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_SCHEDULER_HPP
#define C2NG_CLIENT_VCR_CLASSIC_SCHEDULER_HPP

#include "game/vcr/classic/eventlistener.hpp"

namespace client { namespace vcr { namespace classic {

    class Scheduler : public game::vcr::classic::EventListener {
     public:
        typedef game::vcr::classic::Side Side_t;
        typedef game::vcr::classic::Time_t Time_t;
        typedef game::vcr::classic::FighterStatus FighterStatus_t;

        virtual void removeAnimations() = 0;
    };

} } }

#endif
