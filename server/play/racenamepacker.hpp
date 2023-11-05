/**
  *  \file server/play/racenamepacker.hpp
  *  \brief Class server::play::RaceNamePacker
  */
#ifndef C2NG_SERVER_PLAY_RACENAMEPACKER_HPP
#define C2NG_SERVER_PLAY_RACENAMEPACKER_HPP

#include "afl/string/translator.hpp"
#include "game/root.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    /** Packer for race names.
        This is a subset of PlayerPacker using only a Root. */
    class RaceNamePacker : public Packer {
     public:
        /** Constructor.
            @param root       Root
            @param firstSlot  First slot (0=start with dummy slot, 1=start with Fed)
            @param tx         Translator */
        RaceNamePacker(const game::Root& root, int firstSlot, afl::string::Translator& tx);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        const game::Root& m_root;
        const int m_firstSlot;
        afl::string::Translator& m_translator;
    };

} }

#endif
