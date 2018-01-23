/**
  *  \file game/unitscoredefinitionlist.hpp
  *  \brief Class game::UnitScoreDefinitionList
  */
#ifndef C2NG_GAME_UNITSCOREDEFINITIONLIST_HPP
#define C2NG_GAME_UNITSCOREDEFINITIONLIST_HPP

#include "afl/string/string.hpp"
#include "game/unitscorelist.hpp"

namespace game {

    /*
     *  Well-Known Score Identifiers
     */

    /** Score identifier: Experience level. */
    static const int16_t ScoreId_ExpLevel  = 1;

    /** Score identifier: Experience points. */
    static const int16_t ScoreId_ExpPoints = 2;


    /** Definition of per-unit scores.
        Most importantly, per-unit scores are used for unit experience in PHost.

        Although unit scores are generally stored indexed by type, then unit
        (i.e. a global list of score types, associated with a list of units' scores),
        we separate these two: a UnitScoreDefinitionList object defines all scores,
        and each object contains a list of applicable stores in a UnitScoreList,
        indexed by indexes managed by the appropriate UnitScoreDefinitionList
        (similar to the interpreter properties split into a NameMap and a couple of Segments).

        This requires us to split up stuff we load, and gather it up again when we save it,
        but it allows us to easily clone an object with score and assign it a new score,
        either for loading past chart.cc files or for performing host updates.

        It also needs a more memory (i.e. 1500 vector<Item>, many empty,
        instead of one or two definitions with one vector<Item> each),
        but this is not so much an issue today as it was in PCC 1.x. */
    class UnitScoreDefinitionList {
     public:
        /** Definition of a unit score. */
        struct Definition {
            String_t name;             /**< Name of the score. */
            int16_t id;                /**< Identifier of the score. */
            int16_t limit;             /**< Limit of the score (for informative purposes). */
        };

        /** Index identifying a score. */
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
