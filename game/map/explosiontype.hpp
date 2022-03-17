/**
  *  \file game/map/explosiontype.hpp
  *  \brief Class game::map::ExplosionType
  */
#ifndef C2NG_GAME_MAP_EXPLOSIONTYPE_HPP
#define C2NG_GAME_MAP_EXPLOSIONTYPE_HPP

#include "afl/container/ptrvector.hpp"
#include "game/map/explosion.hpp"
#include "game/map/typedobjecttype.hpp"
#include "game/parser/messageinformation.hpp"

namespace game { namespace map {

    /** Explosion container.
        Contains a number of Explosion objects and methods to create/iterate them.

        Since explosions have optional Ids, the indexes used for iteration
        have no guaranteed relation to the explosion's Ids. */
    class ExplosionType : public TypedObjectType<Explosion> {
     public:
        /** Default constructor.
            Makes an empty container. */
        ExplosionType();

        /** Destructor. */
        ~ExplosionType();

        /** Add an explosion.
            If this explosion matches one we already know, merges the information.
            \param ex Explosion to add
            \see Explosion::merge */
        void add(const Explosion& ex);

        /** Add message information.
            Merges the information, creating a new explosion or updating an existing one as required.
            \param info Message information */
        void addMessageInformation(const game::parser::MessageInformation& info);

        // ObjectType:
        Explosion* getObjectByIndex(Id_t index);
        Id_t getNextIndex(Id_t index) const;
        Id_t getPreviousIndex(Id_t index) const;

     private:
        afl::container::PtrVector<Explosion> m_explosions;
    };

} }

#endif
