/**
  *  \file game/map/historyshiptype.hpp
  *  \brief Class game::map::HistoryShipType
  */
#ifndef C2NG_GAME_MAP_HISTORYSHIPTYPE_HPP
#define C2NG_GAME_MAP_HISTORYSHIPTYPE_HPP

#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    /** History starships type.
        Contains all starships that have history (even if they are not visible now). */
    class HistoryShipType : public ObjectVectorType<Ship> {
     public:
        /** Constructor.
            \param vec Ship vector */
        explicit HistoryShipType(ObjectVector<Ship>& vec);

        // ObjectVectorType:
        virtual bool isValid(const Ship& p) const;

        /** Find next ship at position, after index.
            Optionally, limit search to only marked ships.
            Will return any ship that is or has been at the given position.
            The returned ship is guaranteed to exist.
            \param [in]  pos    Find ships that are/were at this position
            \param [in]  id     Find ships with Id greater than this
            \param [in]  marked true to return only marked ships
            \param [out] turn   On success, receives the turn number
            \return found ship Id; 0 if none */
        Id_t findNextShipAtNoWrap(Point pos, Id_t id, bool marked, int& turn);

        /** Find previous ship at position, after index.
            Optionally, limit search to only marked ships.
            Will return any ship that is or has been at the given position.
            The returned ship is guaranteed to exist.
            \param [in]  pos    Find ships that are/were at this position
            \param [in]  id     Find ships with Id less than this
            \param [in]  marked true to return only marked ships
            \param [out] turn   On success, receives the turn number
            \return found ship Id; 0 if none */
        Id_t findPreviousShipAtNoWrap(Point pos, Id_t id, bool marked, int& turn);

        /** Find next ship at position, after index, with wrap.
            Optionally, limit search to only marked ships.
            Will return any ship that is or has been at the given position.
            The returned ship is guaranteed to exist.
            \param [in]  pos    Find ships that are/were at this position
            \param [in]  id     Start search at this Id
            \param [in]  marked true to return only marked ships
            \param [out] turn   On success, receives the turn number
            \return found ship Id; 0 if none */
        Id_t findNextShipAtWrap(Point pos, Id_t id, bool marked, int& turn);

        /** Find previous ship at position, after index, with wrap.
            Optionally, limit search to only marked ships.
            Will return any ship that is or has been at the given position.
            The returned ship is guaranteed to exist.
            \param [in]  pos    Find ships that are/were at this position
            \param [in]  id     Start search at this Id
            \param [in]  marked true to return only marked ships
            \param [out] turn   On success, receives the turn number
            \return found ship Id; 0 if none */
        Id_t findPreviousShipAtWrap(Point pos, Id_t id, bool marked, int& turn);

     private:
        bool acceptShip(Point pos, Id_t id, bool marked, int& turn);
    };

} }

#endif
