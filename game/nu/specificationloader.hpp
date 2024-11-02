/**
  *  \file game/nu/specificationloader.hpp
  *  \brief Class game::nu::SpecificationLoader
  */
#ifndef C2NG_GAME_NU_SPECIFICATIONLOADER_HPP
#define C2NG_GAME_NU_SPECIFICATIONLOADER_HPP

#include "afl/base/ptr.hpp"
#include "afl/data/access.hpp"
#include "afl/io/directory.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/nu/gamestate.hpp"
#include "game/specificationloader.hpp"

namespace game { namespace nu {

    /** SpecificationLoader implementation for planets.nu.

        Nu does not distinguish between (player-independant) specification and (player-specific) result files.
        We therefore always load the result (stored in GameState), and extract the player-specific specs from it. */
    class SpecificationLoader : public game::SpecificationLoader {
     public:
        /** Constructor.

            @param defaultSpecificationDirectory   PCC2 default specs. For local definitions (e.g. hull functions), and openSpecificationFile().
            @param gameState    Game state
            @param tx           Translator
            @param log          Logger */
        SpecificationLoader(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                            afl::base::Ref<GameState> gameState,
                            afl::string::Translator& tx,
                            afl::sys::LogListener& log);

        /** Destructor. */
        ~SpecificationLoader();

        // SpecificationLoader:
        virtual std::auto_ptr<Task_t> loadShipList(game::spec::ShipList& list, Root& root, std::auto_ptr<StatusTask_t> then);
        virtual afl::base::Ref<afl::io::Stream> openSpecificationFile(const String_t& fileName);

     private:
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        afl::base::Ref<GameState> m_gameState;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;

        void loadHullFunctionDefinitions(game::spec::ShipList& list);
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
