/**
  *  \file game/v3/udata/parser.hpp
  */
#ifndef C2NG_GAME_V3_UDATA_PARSER_HPP
#define C2NG_GAME_V3_UDATA_PARSER_HPP

#include "afl/charset/charset.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/game.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/spec/shiplist.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/udata/reader.hpp"
#include "util/atomtable.hpp"
#include "util/vector.hpp"

namespace game { namespace v3 { namespace udata {

    class Parser : public Reader {
     public:
        /** Constructor.
            \param game       Game. Data will be stored here (mostly, its currentTurn(), but also scores and score definitions)
            \param playerNr   Player number.
            \param config     Host configuration. Could be updated by received data.
            \param shipList   Ship list. The modified hull function list could be updated by received data.
            \param atomTable  Atom table (for marker tags).
            \param cs         Character set. Used for decoding strings.
            \param tx         Translator
            \param log        Logger */
        Parser(Game& game,
               int playerNr,
               game::config::HostConfiguration& config,
               game::spec::ShipList& shipList,
               util::AtomTable& atomTable,
               afl::charset::Charset& cs,
               afl::string::Translator& tx,
               afl::sys::LogListener& log);

        virtual ~Parser();

        // Reader:
        virtual bool handleRecord(uint16_t recordId, afl::base::ConstBytes_t data);
        virtual void handleError(afl::io::Stream& in);
        virtual void handleEnd();

     private:
        enum Scope {
            ShipScope,
            PlanetScope
        };

        // Fixed data
        Game& m_game;
        const int m_player;
        game::config::HostConfiguration& m_hostConfiguration;
        game::spec::ShipList& m_shipList;
        util::AtomTable& m_atomTable;
        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;

        // Dynamic data
        util::Vector<uint8_t,Id_t> m_destroyedShips;
        std::vector<game::v3::structures::Util7Battle> m_battleResults;

        int getTurnNumber() const;

        void markShipKilled(Id_t id);
        void processAlliances(const game::v3::structures::Util22Alliance& allies);
        void processEnemies(uint16_t enemies);
        void processScoreRecord(afl::base::ConstBytes_t data, Scope scope, UnitScoreDefinitionList& defs);
        void processMessageInformation(const game::parser::MessageInformation& info);
    };

} } }

#endif
