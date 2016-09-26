/**
  *  \file game/spec/hullfunctionassignmentlist.hpp
  */
#ifndef C2NG_GAME_SPEC_HULLFUNCTIONASSIGNMENTLIST_HPP
#define C2NG_GAME_SPEC_HULLFUNCTIONASSIGNMENTLIST_HPP

#include "game/spec/modifiedhullfunctionlist.hpp"
#include "game/playerset.hpp"
#include "game/spec/hullfunctionlist.hpp"

namespace game { namespace spec {

    class Hull;
    class BasicHullFunctionList;

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

        HullFunctionAssignmentList();

        ~HullFunctionAssignmentList();

        void clear();

        size_t getNumEntries() const;

        void change(ModifiedHullFunctionList::Function_t function, PlayerSet_t add, PlayerSet_t remove);

        Entry* find(ModifiedHullFunctionList::Function_t function);

        void remove(ModifiedHullFunctionList::Function_t function);

        Entry* get(size_t i);

        void getAll(HullFunctionList& out,
                    const ModifiedHullFunctionList& definitions,
                    const game::config::HostConfiguration& config,
                    const Hull& hull,
                    PlayerSet_t playerLimit,
                    ExperienceLevelSet_t levelLimit,
                    HullFunction::Kind kind);

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
