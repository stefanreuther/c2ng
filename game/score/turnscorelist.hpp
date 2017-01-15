/**
  *  \file game/score/turnscorelist.hpp
  */
#ifndef C2NG_GAME_SCORE_TURNSCORELIST_HPP
#define C2NG_GAME_SCORE_TURNSCORELIST_HPP

#include "game/score/turnscore.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/score/scoreid.hpp"

namespace game { namespace score {

    // /** Statistics file reading. PCC 1.x does lazy parsing of statistics files,
    // and I originally planned that for PCC2 as well. The file format is
    // designed to support such a type of access. However, this would have meant
    // quite some complexity and several states to distinguish between all
    // operations that can possibly be done on a statistics file. Since
    // the file will be a few k only, it makes sense to simply slurp it into
    // memory completely and write it out completely as well. To avoid losing
    // files written by a future version, we manage a flag saying so. */
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
        };
        
        TurnScoreList();

        ~TurnScoreList();

        void clear();


        Slot_t addSlot(ScoreId_t id);

        bool getSlot(ScoreId_t id, Slot_t& out) const;

        bool addDescription(const Description& d);

        const Description* getDescription(ScoreId_t id) const;

        TurnScore& addTurn(int turnNr, const Timestamp& time);

        const TurnScore* getTurn(int turnNr) const;

        void setFutureFeatures(bool flag);

        bool hasFutureFeatures() const;

     private:
    // /** Definitions of the records.
    //     Each entry describes one row of scores.
    //     Accessing a GStatRecord with (x,player) returns the score whose Id
    //     is record_defs[x]. */
        std::vector<ScoreId_t> m_slotMapping;

        /** Score descriptions.
            Optional descriptions of score rows.
            Each description contains the score Id it describes,
            there is no relation of this sequence and m_slotMapping. */
        std::vector<Description> m_scoreDescriptions;        

        // /** Write-protection marker.
        //     Set when the file contains a future feature. */
        bool m_fileUsedFutureFeatures;

        /** All score records. */
        afl::container::PtrVector<TurnScore> m_turnScores;
    };

} }

#endif
