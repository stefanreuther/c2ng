/**
  *  \file server/play/shipfriendlycodepacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_SHIPFRIENDLYCODEPACKER_HPP
#define C2NG_SERVER_PLAY_SHIPFRIENDLYCODEPACKER_HPP

#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    class ShipFriendlyCodePacker : public Packer {
     public:
        ShipFriendlyCodePacker(game::Session& session, game::Id_t shipId);

        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

        static Value_t* buildFriendlyCodeList(const game::spec::FriendlyCodeList& list,
                                              const game::PlayerList& players,
                                              afl::string::Translator& tx);

     private:
        game::Session& m_session;
        game::Id_t m_shipId;
    };

} }

#endif
