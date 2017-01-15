/**
  *  \file game/historyturn.hpp
  *  \brief Class game::HistoryTurn
  */
#ifndef C2NG_GAME_HISTORYTURN_HPP
#define C2NG_GAME_HISTORYTURN_HPP

#include "game/timestamp.hpp"
#include "afl/base/ptr.hpp"
#include "afl/base/uncopyable.hpp"

namespace game {

    class Turn;

    /** Historic turn.
        Manages the status of a historic turn, and optionally that turn's data.

        The lifecycle of such an object is:
        - create it
        - populate its status using HistoryTurnList::initFromTurnScores(), HistoryTurnList::initFromTurnLoader(), TurnLoader::getHistoryStatus()
        - if it is loadable, try to load the turn and pass back the result using handleLoadSucceeded(), handleLoadFailed()

        Invariants:
        - a HistoryTurn object always represents the same turn
        - once given a Turn object, it will not change or drop that object

        Note that the setStatus method does not prevent transitions that violate the second invariant. */
    class HistoryTurn : private afl::base::Uncopyable {
     public:
        /** Turn status. */
        enum Status {
            Unknown,            ///< I don't know.
            Unavailable,        ///< I know it is not available. \see TurnLoader::Negative
            StronglyAvailable,  ///< I'm certain it's available. \see TurnLoader::StronglyPositive
            WeaklyAvailable,    ///< I guess it's available. \see TurnLoader::WeaklyPositive
            Failed,             ///< Loading failed.
            Loaded              ///< It is loaded.
        };

        /** Constructor.
            \param nr Turn number */
        explicit HistoryTurn(int nr);

        /** Destructor. */
        ~HistoryTurn();

        /** Get turn number.
            \return turn number */
        int getTurnNumber() const;

        /** Set timestamp.
            The object starts with a default timestamp (!Timestamp::isValid()).
            \param ts Turn timestamp */
        void setTimestamp(const Timestamp& ts);

        /** Get timestamp.
            \return Turn timestamp */
        const Timestamp& getTimestamp() const;

        /** Set status.
            The object starts in status Unknown.
            \param st Status */
        void setStatus(Status st);

        /** Get status.
            \return Status */
        Status getStatus() const;

        /** Check whether this turn can be loaded.
            \return true if if makes sense to load this turn (WeaklyAvailable, StronglyAvailable, Unknown) */
        bool isLoadable() const;

        /** Handle successful load.
            If this turn was loadable, remembers the given Turn object, and switches to state Loaded.
            \param turn Turn, must not be null */
        void handleLoadSucceeded(afl::base::Ref<Turn> turn);

        /** Handle unsuccessful load.
            Depending on the turn's status, enters Failed or Unavailable status. */
        void handleLoadFailed();

        /** Get turn.
            \return Turn, can be null if not loaded */
        afl::base::Ptr<Turn> getTurn() const;

     private:
        const int m_turnNumber;
        Timestamp m_timestamp;
        Status m_status;
        afl::base::Ptr<Turn> m_turn;
    };

}

#endif
