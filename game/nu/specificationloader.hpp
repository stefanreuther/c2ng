/**
  *  \file game/nu/specificationloader.hpp
  */
#ifndef C2NG_GAME_NU_SPECIFICATIONLOADER_HPP
#define C2NG_GAME_NU_SPECIFICATIONLOADER_HPP

#include "game/specificationloader.hpp"
#include "game/nu/gamestate.hpp"
#include "afl/base/ptr.hpp"
#include "afl/data/access.hpp"
#include "afl/sys/loglistener.hpp"

namespace game { namespace nu {

    class SpecificationLoader : public game::SpecificationLoader {
     public:
        SpecificationLoader(afl::base::Ref<GameState> gameState,
                            afl::string::Translator& tx,
                            afl::sys::LogListener& log);

        ~SpecificationLoader();

        virtual std::auto_ptr<Task_t> loadShipList(game::spec::ShipList& list, Root& root, std::auto_ptr<StatusTask_t> then);

     private:
        afl::base::Ref<GameState> m_gameState;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;

        void loadHulls(game::spec::ShipList& list, afl::data::Access p);
        void loadBeams(game::spec::ShipList& list, afl::data::Access p);
        void loadTorpedoes(game::spec::ShipList& list, afl::data::Access p);
        void loadEngines(game::spec::ShipList& list, afl::data::Access p);
        void loadDefaultHullAssignments(game::spec::ShipList& list, afl::data::Access players, afl::data::Access races);
        void loadRaceHullAssignments(game::spec::ShipList& list, afl::data::Access racehulls, int player);

        void loadRaceNames(Root& root, afl::data::Access players, afl::data::Access races);
    };

} }

#endif
