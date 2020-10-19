/**
  *  \file game/vcr/classic/statustoken.hpp
  *  \brief Base class game::vcr::classic::StatusToken
  */
#ifndef C2NG_GAME_VCR_CLASSIC_STATUSTOKEN_HPP
#define C2NG_GAME_VCR_CLASSIC_STATUSTOKEN_HPP

#include "afl/base/deletable.hpp"
#include "game/vcr/classic/types.hpp"

namespace game { namespace vcr { namespace classic {

    /** Base class for VCR status token.
        \see Algorithm::createStatusToken, Algorithm::restoreStatus */
    class StatusToken : public afl::base::Deletable {
     public:
        /** Constructor.
            \param time Time (position in VCR; seconds) */
        StatusToken(Time_t time);

        /** Get time.
            \return time */
        Time_t getTime() const;

     private:
        Time_t m_time;
    };

} } }

#endif
