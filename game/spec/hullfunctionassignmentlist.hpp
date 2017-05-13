/**
  *  \file game/spec/hullfunctionassignmentlist.hpp
  *  \brief Class game::spec::HullFunctionAssignmentList
  */
#ifndef C2NG_GAME_SPEC_HULLFUNCTIONASSIGNMENTLIST_HPP
#define C2NG_GAME_SPEC_HULLFUNCTIONASSIGNMENTLIST_HPP

#include "game/spec/modifiedhullfunctionlist.hpp"
#include "game/playerset.hpp"
#include "game/spec/hullfunctionlist.hpp"

namespace game { namespace spec {

    class Hull;
    class BasicHullFunctionList;

    /** Hull function assignment list.
        This stores the assignments of hull functions for a single item (i.e. hull).
        It can store added and removed hull functions;
        removed functions are important to deal with functions that are assigned by default using the host configuration. */
    class HullFunctionAssignmentList {
     public:
        struct Entry {
            ModifiedHullFunctionList::Function_t m_function;
            PlayerSet_t m_addedPlayers;
            PlayerSet_t m_removedPlayers;

            Entry(ModifiedHullFunctionList::Function_t function,
                  PlayerSet_t addedPlayers,
                  PlayerSet_t removedPlayers)
                : m_function(function),
                  m_addedPlayers(addedPlayers),
                  m_removedPlayers(removedPlayers)
                {
                    // ex GHull::SpecialAssignment
                }
        };

        /** Constructor.
            Makes a list that reports all functions as unchanged from default. */
        HullFunctionAssignmentList();

        /** Destructor. */
        ~HullFunctionAssignmentList();

        /** Clear.
            Resets the object into its initial state. */
        void clear();

        /** Get number of entries.
            \return number of entries */
        size_t getNumEntries() const;

        /** Modify hull function assigmnent.
            \param function Function Id
            \param add Allow these players to use it...
            \param remove ...then disallow these players to use it. */
        void change(ModifiedHullFunctionList::Function_t function, PlayerSet_t add, PlayerSet_t remove);

        /** Find entry, given a function Id.
            \param function Function Id
            \return entry if found; null otherwise */
        const Entry* findEntry(ModifiedHullFunctionList::Function_t function) const;

        /** Remove entry, given a function Id.
            Use with care to not remove the dummy entries required to deal with variable default assignments.
            \param function Function Id */
        void removeEntry(ModifiedHullFunctionList::Function_t function);

        /** Get entry, given an index.
            \param i Index [0,getNumEntries())
            \return entry; null if index is out of range */
        const Entry* getEntryByIndex(size_t i) const;

        /** Get all effective assignments as a HullFunctionList.
            This resolves modified functions and fills in all fields of the HullFunction's in the list.
            \param out         [out] Result is appended here
            \param definitions [in] Definitions of modified hull functions
            \param config      [in] Host configuration (used to resolve variable defaults)
            \param hull        [in] Hull (used to resolve variable defaults)
            \param playerLimit [in] Only return hull function assignments that affect players from this set
            \param levelLimit  [in] Only return hull function assignments that affect levels from this set
            \param kind        [in] Use this Kind for results (also used to resolve variable defaults) */
        void getAll(HullFunctionList& out,
                    const ModifiedHullFunctionList& definitions,
                    const game::config::HostConfiguration& config,
                    const Hull& hull,
                    PlayerSet_t playerLimit,
                    ExperienceLevelSet_t levelLimit,
                    HullFunction::Kind kind) const;

        /** Get players that can perform a particular basic function.
            This resolves modified <em>and implied</em> functions (that is, querying for Cloak will also find AdvancedCloak).
            \param basicFunctionId  [in] Basic function Id
            \param definitions      [in] Definitions of modified hull functions
            \param basicDefinitions [in] Definitions of basic hull functions (used to resolve implications)
            \param config           [in] Host configuration (used to resolve variable defaults)
            \param hull             [in] Hull (used to resolve variable defaults)
            \param levelLimit       [in] Only accept assignments available at a level from this set
            \param useDefaults      [in] true to include variable defaults */
        PlayerSet_t getPlayersThatCan(int basicFunctionId,
                                      const ModifiedHullFunctionList& definitions,
                                      const BasicHullFunctionList& basicDefinitions,
                                      const game::config::HostConfiguration& config,
                                      const Hull& hull,
                                      ExperienceLevelSet_t levelLimit,
                                      bool useDefaults) const;

     private:
        std::vector<Entry> m_entries;
    };

} }

#endif
