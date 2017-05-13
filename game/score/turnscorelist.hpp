/**
  *  \file game/score/turnscorelist.hpp
  *  \brief Class game::score::TurnScoreList
  */
#ifndef C2NG_GAME_SCORE_TURNSCORELIST_HPP
#define C2NG_GAME_SCORE_TURNSCORELIST_HPP

#include "game/score/turnscore.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/score/scoreid.hpp"

namespace game { namespace score {

    /** List of score file records.
        The score file contains per-turn, per-player scores of different types.
        TurnScoreList contains:
        - a mapping of score types and optional descriptions to physical indexes into the TurnScore objects
        - a list of turns that needs not be exhaustive (i.e. can have gaps)

        Unlike PCC 1.x, PCC2 and c2ng always read the score file into memory completely, and always write it out completely.
        The file format has room for future expansion, so we store a flag to avoid rewriting a file that contains features we don't understand. */
    class TurnScoreList {
     public:
        /** Slot for a particular score series.
            For example: "number of freighters" is always stored in the same slot of a TurnScore. */
        typedef TurnScore::Slot_t Slot_t;

        /** Index of a turn in the score list. */
        typedef size_t Index_t;

        // FIXME: here?
        struct Description {
            String_t name;               ///< Name of score. Identifies the score to humans.
            ScoreId_t scoreId;   ///< Type of score. Identifies the score to programs.
            int16_t turnLimit;           ///< Turns to keep win limit.
            int32_t winLimit;            ///< Win limit. If somebody exceeds this limit for turn_limit turns, he wins. -1=no such limit.
            // FIXME: replace winLimit by InlineOptional

            Description()
                : name(), scoreId(0), turnLimit(0), winLimit(-1)
                { }
            Description(const String_t& name, ScoreId_t scoreId, int16_t turnLimit, int32_t winLimit)
                : name(name), scoreId(scoreId), turnLimit(turnLimit), winLimit(winLimit)
                { }
        };

        /** Constructor.
            Makes an empty TurnScoreList with a default schema. */
        TurnScoreList();

        /** Destructor. */
        ~TurnScoreList();

        /** Reset content.
            Clears the TurnScoreList to the same state it had after construction. */
        void clear();

        /*
         *  Regular Access
         */

        /** Add a score type.
            If a score of that type does not exist yet, adds it to the schema;
            otherwise, returns the existing slot number.
            \param id Score Id
            \return slot number */
        Slot_t addSlot(ScoreId_t id);

        /** Get a score slot by type.
            \param id  [in] Score Id
            \param out [out] Slot number
            \retval true Score Id was known, \c out was set
            \retval false Score Id was not known, \c out not set */
        bool getSlot(ScoreId_t id, Slot_t& out) const;

        /** Add a score description.
            A score description provides additional meta-information for a score Id.
            \param d Description
            \retval true This description was added anew, or updated an existing description
            \retval false This description was already known; no change */
        bool addDescription(const Description& d);

        /** Get a score description.
            \param id Score Id
            \return Description if known; otherwise, null */
        const Description* getDescription(ScoreId_t id) const;

        /** Add a turn.
            If the given turn does not yet exist, it is created.
            If the turn exists with the same timestamp, it is returned.
            If it exists with a different timestamp, it is cleared first (re-host case).
            This is the only way to obtain a mutable TurnScore object.
            \param turnNr Turn number
            \param time Timestamp
            \return TurnScore for that turn */
        TurnScore& addTurn(int turnNr, const Timestamp& time);

        /** Get a turn.
            \param turnNr Turn number
            \return TurnScore object; null if none known for this turn. */
        const TurnScore* getTurn(int turnNr) const;


        /*
         *  Iteration / Raw Access
         */

        /** Get number of turns stored.
            \return number of turns */
        size_t getNumTurns() const;

        /** Get turn by index.
            \param index Index [0,getNumTurns())
            \return TurnScore object; null if index out of range */
        const TurnScore* getTurnByIndex(size_t index) const;

        /** Get number of descriptions stored.
            \return number of descriptions */
        size_t getNumDescriptions() const;

        /** Get description by index.
            \param index Index [0,getNumDescriptions())
            \return Description; null if index out of range */
        const Description* getDescriptionByIndex(size_t index) const;

        /** Get number of score types stored.
            \return number of score types */
        size_t getNumScores() const;

        /** Get score Id by index.
            \param index [in] Index [0,getNumScores())
            \param result [out] Score Id
            \retval true Index ok, \c result has been set
            \retval false Index out of range, \c result not set */
        bool getScoreByIndex(size_t index, ScoreId_t& result) const;

        /** Set "future features" flag.
            This flag is for convenience of the user and does not affect TurnScoreList's behaviour.
            It is used by Loader to mark information from a file we could not entirely understand.
            \param flag Flag */
        void setFutureFeatures(bool flag);

        /** Get "future features" flag.
            \return flag */
        bool hasFutureFeatures() const;

     private:
        /** Score Id / Slot mapping.
            Accessing a score with a given ScoreId_t will access the TurnScore object
            with the index x such that m_slotMapping[x] == ScoreId_t. */
        std::vector<ScoreId_t> m_slotMapping;

        /** Score descriptions.
            Optional descriptions of score rows.
            Each description contains the score Id it describes,
            there is no relation of this sequence and m_slotMapping. */
        std::vector<Description> m_scoreDescriptions;

        /** Future Features / Write-protection flag.
            Set when the score file contains a future feature. */
        bool m_fileUsedFutureFeatures;

        /** All score records. */
        afl::container::PtrVector<TurnScore> m_turnScores;
    };

} }

#endif
