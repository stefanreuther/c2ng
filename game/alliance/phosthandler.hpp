/**
  *  \file game/alliance/phosthandler.hpp
  */
#ifndef C2NG_GAME_ALLIANCE_PHOSTHANDLER_HPP
#define C2NG_GAME_ALLIANCE_PHOSTHANDLER_HPP

#include "game/alliance/handler.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"

namespace game { namespace alliance {

    class PHostHandler : public Handler {
     public:
        PHostHandler(int32_t version, Turn& turn, Session& session, int player);
        ~PHostHandler();

        virtual void init(Container& allies, afl::string::Translator& tx);
        virtual void postprocess(Container& allies);
        virtual void handleChanges(const Container& allies);

     private:
        int32_t m_version;
        Turn& m_turn;

        // FIXME: I'm giving this guy a Session because that is guaranteed to out-live
        // the PHostHandler and the Turn. May we give it a Ref<Root> instead?
        Session& m_session;

        int m_player;
    };

} }

#endif
