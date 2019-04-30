/**
  *  \file server/play/planetfriendlycodepacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_PLANETFRIENDLYCODEPACKER_HPP
#define C2NG_SERVER_PLAY_PLANETFRIENDLYCODEPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"
#include "game/spec/friendlycodelist.hpp"

namespace server { namespace play {

    class PlanetFriendlyCodePacker : public Packer {
     public:
        PlanetFriendlyCodePacker(game::Session& session, game::Id_t planetId);

        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
        game::Id_t m_planetId;
    };

} }

#endif
