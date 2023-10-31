/**
  *  \file server/play/beampacker.hpp
  *  \brief Class server::play::BeamPacker
  */
#ifndef C2NG_SERVER_PLAY_BEAMPACKER_HPP
#define C2NG_SERVER_PLAY_BEAMPACKER_HPP

#include "server/play/packer.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"

namespace server { namespace play {

    /** Packer for "obj/beam". */
    class BeamPacker : public Packer {
     public:
        /** Constructor.
            @param shipList  Ship list (must be dynamically allocated)
            @param root      Root (must be dynamically allocated)
            @param firstSlot First slot to return (0=start with empty slot, 1=start with first beam)
            @see game::interface::BeamContext */
        BeamPacker(game::spec::ShipList& shipList, const game::Root& root, int firstSlot);

        // Packer:
        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::spec::ShipList& m_shipList;
        const game::Root& m_root;
        int m_firstSlot;
    };

} }

#endif
