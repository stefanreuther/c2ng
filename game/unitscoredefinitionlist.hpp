/**
  *  \file game/unitscoredefinitionlist.hpp
  */
#ifndef C2NG_GAME_UNITSCOREDEFINITIONLIST_HPP
#define C2NG_GAME_UNITSCOREDEFINITIONLIST_HPP

#include "afl/string/string.hpp"
#include "game/unitscorelist.hpp"

namespace game {

    // FIXME: here?
    static const int16_t ScoreId_ExpLevel  = 1;
    static const int16_t ScoreId_ExpPoints = 2;

    class UnitScoreDefinitionList {
     public:
        struct Definition {
            String_t name;
            int16_t id;
            int16_t limit;
        };

        typedef UnitScoreList::Index_t Index_t;

        /** Constructor. */
        UnitScoreDefinitionList();

        /** Destructor. */
        ~UnitScoreDefinitionList();

        /** Add a score definition.
            Does nothing if the definition already exists,
            in this case this only returns the existing index.
            \param def Score definition
            \return index for the score definition, get(return)->id = def.id */
        Index_t add(const Definition& def);

        /** Get score definition by index.
            \param index Index to look up, [0, getNumScores()).
            \return score definition; null if index out of bounds */
        const Definition* get(Index_t index) const;

        /** Get number of scores stored. */
        Index_t getNumScores() const;

        /** Look up score by identifier.
            \param id [in] score_type field of the definition to look up
            \param index [out] Index
            \return true if found, false if not found */
        bool lookup(int16_t id, Index_t& index) const;

     private:
        std::vector<Definition> m_definitions;
    };

}

#endif
