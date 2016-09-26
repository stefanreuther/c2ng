/**
  *  \file game/vcr/classic/statustoken.hpp
  */
#ifndef C2NG_GAME_VCR_CLASSIC_STATUSTOKEN_HPP
#define C2NG_GAME_VCR_CLASSIC_STATUSTOKEN_HPP

#include "afl/base/deletable.hpp"
#include "game/vcr/classic/types.hpp"

namespace game { namespace vcr { namespace classic {


// /// Abstract VCR Status token.
    class StatusToken : public afl::base::Deletable {
     public:
        StatusToken(Time_t time);

        Time_t getTime() const;

     private:
        Time_t m_time;
    };

} } }

#endif
