/**
  *  \file game/unitscorelist.hpp
  *  \brief Class game::UnitScoreList
  */
#ifndef C2NG_GAME_UNITSCORELIST_HPP
#define C2NG_GAME_UNITSCORELIST_HPP

#include <vector>
#include "afl/base/types.hpp"

namespace game {

    /** Unit score list.
        Contains score values for one unit.
        Each score is annotated with a turn number.
        Score values can be missing.

        Scores are identified by an index.
        The indexes are defined by the unit type's UnitScoreDefinitionList (see Index_t). */
    class UnitScoreList {
     public:
        /** Index identifying a score. */
        typedef size_t Index_t;

        /** Constructor.
            Makes blank object. */
        UnitScoreList();

        /** Destructor. */
        ~UnitScoreList();

        /** Set score value.
            \param index Score to set
            \param value Score value
            \param turn  Turn of this report */
        void set(Index_t index, int16_t value, int16_t turn);

        /** Merge score value.
            Like set(), but only modifies the score if this report's turn is the same or higher than the stored value.
            \param index Score to set
            \param value Score value
            \param turn  Turn of this report */
        void merge(Index_t index, int16_t value, int16_t turn);

        /** Get score value.
            \param index [in] Score to get
            \param value [out] Score value
            \param value [out] Turn number
            \retval true Value was found
            \retval false Value not found, value/turn unchanged */
        bool get(Index_t index, int16_t& value, int16_t& turn) const;

     private:
        struct Item {
            int16_t turn;     // 0 means slot is unused
            int16_t value;
        };
        std::vector<Item> m_items;
    };

}

#endif
