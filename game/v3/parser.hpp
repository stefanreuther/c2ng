/**
  *  \file game/v3/parser.hpp
  *  \brief Class game::v3::Parser
  */
#ifndef C2NG_GAME_V3_PARSER_HPP
#define C2NG_GAME_V3_PARSER_HPP

#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace v3 {

    /** v3 Parser Utilities.
        Implements information gathering from various sources for v3:
        - util.dat (machine-readable miscellaneous scans)
        - message parsing

        This bundles the existing classes to an easier interface. */
    class Parser {
     public:
        /** Constructor.
            \param tx Translator
            \param log Logger
            \param game Game. Updates will be applied to its currentTurn().
            \param player Player number
            \param root Root (playerList(), hostConfiguration())
            \param session Session (must have ship list) */
        Parser(afl::string::Translator& tx, afl::sys::LogListener& log,
               Game& game, int player, Root& root, Session& session);

        /** Load util.dat file.
            \param in The util.dat file
            \param charset Character set */
        void loadUtilData(afl::io::Stream& in, afl::charset::Charset& charset);

        /** Parse messages.
            \param in The msgparse.ini file
            \param inbox Loaded inbox */
        void parseMessages(afl::io::Stream& in, game::msg::Inbox& inbox);

     private:
        class DataInterface;

        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
        Game& m_game;
        int m_player;
        Root& m_root;
        Session& m_session;
    };

} }

#endif
