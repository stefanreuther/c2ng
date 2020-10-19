/**
  *  \file game/map/minefieldtype.hpp
  *  \brief Class game::map::MinefieldType
  */
#ifndef C2NG_GAME_MAP_MINEFIELDTYPE_HPP
#define C2NG_GAME_MAP_MINEFIELDTYPE_HPP

#include "game/map/objectvector.hpp"
#include "game/map/minefield.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/playerset.hpp"
#include "game/parser/messageinformation.hpp"

namespace game { namespace map {

    class Universe;

    /** Container for minefields.

        MinefieldType is implemented as an ObjectVector using the minefield Id as index.
        However, not all slots are occupied by objects, only slots that are actually used are created.
        Minefields that are never scanned are not created.

        Because objects that are part of a Game/Turn/Universe shall never be deleted,
        minefields that are once created are not deleted, only marked as deleted.
        This way, pointers never get invalid. */
    class MinefieldType : public ObjectVector<Minefield>,
                          public ObjectVectorType<Minefield>
    {
     public:
        /** Default constructor. Make empty container. */
        MinefieldType();

        /** Destructor. */
        ~MinefieldType();

        // ObjectVectorType:
        virtual bool isValid(const Minefield& obj) const;

        /** Mark minefield deleted.
            \param id Minefield Id */
        void erase(Id_t id);

        /** Declare that all minefields of a player are known with current data.
            This means alternatively that if we have a minefield of this player in the history,
            and did not scan it this turn, it is gone.
            This happens for Winplan result files (KORE minefields).
            Must be called before internalCheck() to have any effect.

            \param player Player number */
        void setAllMinefieldsKnown(int player);

        /** Internal check/postprocess.
            Postprocess all minefields (in particular, mine decay) and delete those that are gone.
            \param turnNr current turn
            \param host   host version
            \param config host configuration */
        void internalCheck(int currentTurn, const game::HostVersion& host, const game::config::HostConfiguration& config);

        /** Add report from a message.
            This will add/update minefields as required.
            \param info Message information */
        void addMessageInformation(const game::parser::MessageInformation& info);

     private:
        // Players for which we ought to know all minefields
        PlayerSet_t m_allMinefieldsKnown;
    };

} }

#endif
