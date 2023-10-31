/**
  *  \file server/play/enginepacker.hpp
  *  \brief Class server::play::EnginePacker
  */
#ifndef C2NG_SERVER_PLAY_ENGINEPACKER_HPP
#define C2NG_SERVER_PLAY_ENGINEPACKER_HPP

#include "server/play/packer.hpp"
#include "game/spec/shiplist.hpp"

namespace server { namespace play {

    /** Packer for "obj/engine". */
    class EnginePacker : public Packer {
     public:
        /** Constructor.
            @param shipList  Ship list (must be dynamically allocated)
            @param firstSlot First slot to return (0=start with empty slot, 1=start with first engine)
            @see game::interface::EngineContext */
        EnginePacker(game::spec::ShipList& shipList, int firstSlot);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::spec::ShipList& m_shipList;
        int m_firstSlot;
    };

} }

#endif
