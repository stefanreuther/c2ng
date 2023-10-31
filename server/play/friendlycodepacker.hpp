/**
  *  \file server/play/friendlycodepacker.hpp
  *  \brief Class server::play::FriendlyCodePacker
  */
#ifndef C2NG_SERVER_PLAY_FRIENDLYCODEPACKER_HPP
#define C2NG_SERVER_PLAY_FRIENDLYCODEPACKER_HPP

#include "server/play/packer.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/string/translator.hpp"

namespace server { namespace play {

    /** Packer for "obj/fcode" (friendly code list).
        Produces array of NAME, DESCRIPTION, FLAGS, RACES with friendly-code properties.
        @since PCC2 2.40.9 */
    class FriendlyCodePacker : public Packer {
     public:
        /** Constructor.
            @param shipList  Ship list
            @param root      Root
            @param tx        Translator */
        explicit FriendlyCodePacker(const game::spec::ShipList& shipList, const game::Root& root, afl::string::Translator& tx);

        // Packer:
        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        const game::spec::ShipList& m_shipList;
        const game::Root& m_root;
        afl::string::Translator& m_translator;
    };

} }

#endif
