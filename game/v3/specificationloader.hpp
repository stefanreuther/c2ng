/**
  *  \file game/v3/specificationloader.hpp
  */
#ifndef C2NG_GAME_V3_SPECIFICATIONLOADER_HPP
#define C2NG_GAME_V3_SPECIFICATIONLOADER_HPP

#include "game/spec/shiplist.hpp"
#include "afl/io/directory.hpp"
#include "afl/charset/charset.hpp"
#include "game/playerlist.hpp"
#include "game/hostversion.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/specificationloader.hpp"

namespace game { namespace v3 {

    class SpecificationLoader : public game::SpecificationLoader {
     public:
        SpecificationLoader(afl::charset::Charset& charset,
                            afl::string::Translator& tx,
                            afl::sys::LogListener& log);

        virtual void loadShipList(game::spec::ShipList& list, game::Root& root);

        void loadBeams(game::spec::ShipList& list, afl::io::Directory& dir);
        void loadLaunchers(game::spec::ShipList& list, afl::io::Directory& dir);
        void loadEngines(game::spec::ShipList& list, afl::io::Directory& dir);
        void loadHulls(game::spec::ShipList& list, afl::io::Directory& dir);
        void loadHullAssignments(game::spec::ShipList& list, afl::io::Directory& dir);
        void loadHullFunctions(game::spec::ShipList& list, afl::io::Directory& dir,
                               const game::HostVersion& host,
                               const game::config::HostConfiguration& config);
        void loadFriendlyCodes(game::spec::ShipList& list, afl::io::Directory& dir);
        void loadMissions(game::spec::ShipList& list, afl::io::Directory& dir);

     private:
        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };

} }

#endif
