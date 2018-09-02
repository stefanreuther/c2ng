/**
  *  \file game/alliance/hosthandler.hpp
  */
#ifndef C2NG_GAME_ALLIANCE_HOSTHANDLER_HPP
#define C2NG_GAME_ALLIANCE_HOSTHANDLER_HPP

#include "game/alliance/handler.hpp"
#include "afl/base/types.hpp"
#include "game/turn.hpp"

namespace game { namespace alliance {

    class HostHandler : public Handler {
     public:
        HostHandler(int32_t version, Turn& turn, int player);
        ~HostHandler();

        virtual void init(Container& allies);
        virtual void postprocess(Container& allies);
        virtual void handleChanges(const Container& allies);

     private:
        int32_t m_version;
        Turn& m_turn;
        int m_player;
    };

} }

#endif
