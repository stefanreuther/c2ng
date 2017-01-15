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

namespace game { namespace db {

    class DrawingAtomMap;

    class Loader {
     public:
        Loader(afl::charset::Charset& cs, interpreter::World& world);

        void load(afl::io::Stream& in, Turn& turn, Game& game, bool acceptProperties);

     private:
        enum Scope {
            ShipScope,
            PlanetScope
        };

        afl::charset::Charset& m_charset;
        interpreter::World& m_world;

        afl::sys::LogListener& log();

        void loadDrawings(afl::io::Stream& in, game::map::DrawingContainer& container, const DrawingAtomMap& map);

        void loadPropertyRecord(afl::io::Stream& in, Scope scope, game::map::Universe& univ, const afl::data::NameMap& dbNames, afl::data::NameMap& liveNames, interpreter::vmio::ValueLoader& valueLoader);

        void loadUnitScoreRecord(afl::io::Stream& in, Scope scope, game::map::Universe& univ, UnitScoreDefinitionList& defs);
    };

} }

#endif
