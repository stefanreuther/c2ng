/**
  *  \file game/unitscorelist.hpp
  *  \brief Class game::UnitScoreList
  */
#ifndef C2NG_GAME_UNITSCORELIST_HPP
#define C2NG_GAME_UNITSCORELIST_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "game/types.hpp"

namespace game {

    class UnitScoreDefinitionList;

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
            \param [in]  index Score to get
            \param [out] value Score value
            \param [out] turn  Turn number
            \retval true Value was found
            \retval false Value not found, value/turn unchanged */
        bool get(Index_t index, int16_t& value, int16_t& turn) const;

        /** Get score, given a Id.
            Resolves the Id using the UnitScoreDefinitionList, and returns the value.
            If the score does not exist, returns Nothing.
            \param id   Score Id
            \param defs Definitions
            \return score */
        NegativeProperty_t getScoreById(int16_t id, const UnitScoreDefinitionList& defs) const;

     private:
        struct Item {
            int16_t turn;     // 0 means slot is unused
            int16_t value;
        };
        std::vector<Item> m_items;
    };

}

#endif
