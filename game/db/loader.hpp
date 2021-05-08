/**
  *  \file game/db/loader.hpp
  *  \brief Class game::db::Loader
  */
#ifndef C2NG_GAME_DB_LOADER_HPP
#define C2NG_GAME_DB_LOADER_HPP

#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "game/game.hpp"
#include "game/map/drawingcontainer.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "interpreter/vmio/valueloader.hpp"
#include "interpreter/world.hpp"

namespace game { namespace db {

    class DrawingAtomMap;

    /** Database loader.
        Contains methods to load and save a chartX.cc file. */
    class Loader {
     public:
        /** Constructor.
            \param cs Game character set
            \param world World (for properties, logger)
            \param tx Translator */
        Loader(afl::charset::Charset& cs, interpreter::World& world, afl::string::Translator& tx);

        /** Load starchart database file.
            \param in Stream to read from
            \param turn Turn to populate with information
            \param game Game to populate with information (required for score definitions)
            \param acceptProperties true to accept properties, false to skip them.

            Properties are not stored in the trn proper but in the global World.
            Use acceptProperties=false to ignore properties from the file and avoid overwriting global properties. */
        void load(afl::io::Stream& in, Turn& turn, Game& game, bool acceptProperties);

        /** Save starchart database file.
            \param out Stream to write to
            \param turn Turn to save
            \param game Game to read (required for score definitions)
            \param shipList Ship list (required for hull definitions / ship masses) */
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
        void loadPropertyRecord(afl::io::Stream& in, Scope scope, const game::map::Universe& univ, const afl::data::NameMap& dbNames, afl::data::NameMap& liveNames, interpreter::vmio::ValueLoader& valueLoader);
        void loadUnitScoreRecord(afl::io::Stream& in, Scope scope, game::map::Universe& univ, UnitScoreDefinitionList& defs);
        void saveDrawings(afl::io::Stream& out, const game::map::DrawingContainer& container, const DrawingAtomMap& map);
        void savePropertyRecord(afl::io::Stream& out, uint16_t type, game::Id_t id, const afl::data::Segment* pData);
        void saveUnitScores(afl::io::Stream& out, uint16_t type, Scope scope, const UnitScoreDefinitionList& defs, const game::map::Universe& univ);
    };

} }

#endif
