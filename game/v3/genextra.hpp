/**
  *  \file game/v3/genextra.hpp
  *  \brief Class game::v3::GenExtra
  */
#ifndef C2NG_GAME_V3_GENEXTRA_HPP
#define C2NG_GAME_V3_GENEXTRA_HPP

#include "afl/container/ptrmap.hpp"
#include "game/extra.hpp"
#include "game/turn.hpp"
#include "game/v3/genfile.hpp"

namespace game { namespace v3 {

    /** Turn Extra for storing GenFile instances.
        We primarily store GenFile instances for the passwords so we don't need an additional abstraction just for that. */
    class GenExtra : public Extra {
     public:
        /** Constructor.
            @param parent Turn */
        explicit GenExtra(Turn& parent);

        /** Destructor. */
        ~GenExtra();

        /** Create GenFile for a player.
            Call when you load a turn.
            @param player Player
            @return GenFile */
        GenFile& create(int player);

        /** Get GenFile for a player.
            Call when you update a Gen file or save a turn.
            @param player Player
            @return GenFile if present, 0 if none loaded */
        GenFile* get(int player) const;

        /** Create GenFile for a turn.
            Call when you're a TurnLoader.
            @param parent Turn
            @return GenExtra */
        static GenExtra& create(Turn& parent);

        /** Get GenFile for a turn.
            @return GenFile if present, 0 if player has none */
        static GenExtra* get(Turn& parent);
        static const GenExtra* get(const Turn& parent);

        /** Get GenFile for a player, given a turn.
            This is a shortcut for the other get() functions
            @param parent Turn
            @param player Player
            @return GenFile if present, 0 if player has none */
        static GenFile* get(Turn& parent, int player);
        static const GenFile* get(const Turn& parent, int player);

     private:
        Turn& m_parent;
        afl::container::PtrMap<int, GenFile> m_genFiles;
    };

} }

#endif
