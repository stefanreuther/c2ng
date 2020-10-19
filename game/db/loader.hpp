/**
  *  \file game/db/loader.hpp
  */
#ifndef C2NG_GAME_DB_LOADER_HPP
#define C2NG_GAME_DB_LOADER_HPP

#include "afl/io/stream.hpp"
#include "game/turn.hpp"
#include "afl/charset/charset.hpp"
#include "interpreter/world.hpp"
#include "game/map/drawingcontainer.hpp"
#include "interpreter/vmio/valueloader.hpp"
#include "game/game.hpp"
#include "game/spec/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace db {

    class DrawingAtomMap;

    class Loader {
     public:
        Loader(afl::charset::Charset& cs, interpreter::World& world, afl::string::Translator& tx);

        void load(afl::io::Stream& in, Turn& turn, Game& game, bool acceptProperties);

        void save(afl::io::Stream& out, const Turn& turn, const Game& game, const game::spec::ShipList& shipList);

     private:
        enum Scope {
            ShipScope,
            PlanetScope
        };

        afl::charset::Charset& m_charset;
        interpreter::World& m_world;
        afl::string::Translator& m_translator;

        afl::sys::LogListener& log();

        void loadDrawings(afl::io::Stream& in, game::map::DrawingContainer& container, const DrawingAtomMap& map);

        void loadPropertyRecord(afl::io::Stream& in, Scope scope, game::map::Universe& univ, const afl::data::NameMap& dbNames, afl::data::NameMap& liveNames, interpreter::vmio::ValueLoader& valueLoader);

        void loadUnitScoreRecord(afl::io::Stream& in, Scope scope, game::map::Universe& univ, UnitScoreDefinitionList& defs);

        void saveDrawings(afl::io::Stream& out, const game::map::DrawingContainer& container, const DrawingAtomMap& map);

        void savePropertyRecord(afl::io::Stream& out, uint16_t type, game::Id_t id, const afl::data::Segment* pData);

        void saveUnitScores(afl::io::Stream& out, uint16_t type, Scope scope, const UnitScoreDefinitionList& defs, const game::map::Universe& univ);

    };

} }

#endif
