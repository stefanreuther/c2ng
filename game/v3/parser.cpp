/**
  *  \file game/v3/parser.cpp
  *  \brief Class game::v3::Parser
  */

#include "game/v3/parser.hpp"
#include "game/parser/datainterface.hpp"
#include "game/parser/messageparser.hpp"
#include "game/turn.hpp"
#include "game/v3/udata/parser.hpp"

using afl::string::strTrim;
using afl::string::strCaseCompare;

/*
 *  DataInterface implementation for real game
 */
class game::v3::Parser::DataInterface : public game::parser::DataInterface {
 public:
    DataInterface(int playerNr,
                  const Root& root,
                  const game::spec::ShipList& shipList,
                  afl::string::Translator& tx)
        : m_playerNumber(playerNr),
          m_root(root),
          m_shipList(shipList),
          m_translator(tx)
        { }

    virtual int getPlayerNumber() const
        { return m_playerNumber; }

    virtual int parseName(Name which, const String_t& name) const
        {
            switch (which) {
             case ShortRaceName:
                return parsePlayerName(Player::OriginalShortName, name);
             case LongRaceName:
                return parsePlayerName(Player::OriginalLongName, name);
             case AdjectiveRaceName:
                return parsePlayerName(Player::OriginalAdjectiveName, name);
             case HullName:
                return parseHullName(name);
            }
            return 0;
        }

    virtual String_t expandRaceNames(String_t tpl) const
        { return m_root.playerList().expandNames(tpl, true, m_translator); }

 private:
    int parsePlayerName(Player::Name which, const String_t& name) const
        {
            // ex readmsg.pas:LazyCompare
            // FIXME: space in 'name' should match any in 'p->getName'
            // (host sanitized extended character)
            const PlayerList& pp = m_root.playerList();
            for (Player* p = pp.getFirstPlayer(); p != 0; p = pp.getNextPlayer(p)) {
                if (strCaseCompare(name, strTrim(p->getName(which, m_translator))) == 0) {
                    return p->getId();
                }
            }
            return 0;
        }

    int parseHullName(const String_t& name) const
        {
            const game::spec::ComponentVector<game::spec::Hull>& hh = m_shipList.hulls();
            for (game::spec::Hull* h = hh.findNext(0); h != 0; h = hh.findNext(h->getId())) {
                if (strCaseCompare(name, h->getName(m_shipList.componentNamer())) == 0) {
                    return h->getId();
                }
            }
            return 0;
        }

    int m_playerNumber;
    const Root& m_root;
    const game::spec::ShipList& m_shipList;
    afl::string::Translator& m_translator;
};

// Constructor.
game::v3::Parser::Parser(afl::string::Translator& tx, afl::sys::LogListener& log, Game& game, int player, Root& root, game::spec::ShipList& shipList, util::AtomTable& atomTable)
    : m_translator(tx),
      m_log(log),
      m_game(game),
      m_player(player),
      m_root(root),
      m_shipList(shipList),
      m_atomTable(atomTable)
{ }

// Load util.dat file.
void
game::v3::Parser::loadUtilData(afl::io::Stream& in, afl::charset::Charset& charset)
{
    game::v3::udata::Parser(m_game, m_player, m_root.hostConfiguration(), m_shipList, m_atomTable, charset, m_translator, m_log).read(in);
}

// Parse messages.
void
game::v3::Parser::parseMessages(afl::io::Stream& in, const game::msg::Inbox& inbox)
{
    // ex game/msgglobal.cc:parseMessages (remotely related)

    // For now, we load the message definitions every time we parse an inbox.
    // This avoids having to have yet another stateful object.
    // This would be inadequate only for a program that repeatedly parses different inboxes;
    // a regular client works fine with this restriction.

    // Load message definitions
    game::parser::MessageParser p;
    p.load(in, m_translator, m_log);

    // Parse messages
    DataInterface gdi(m_player, m_root, m_shipList, m_translator);
    for (size_t i = 0, n = inbox.getNumMessages(); i < n; ++i) {
        String_t text = inbox.getMessageText(i, m_translator, m_root.playerList());

        afl::container::PtrVector<game::parser::MessageInformation> info;
        p.parseMessage(text, gdi, m_game.currentTurn().getTurnNumber(), info, m_translator, m_log);

        for (size_t ii = 0, in = info.size(); ii < in; ++ii) {
            m_game.addMessageInformation(*info[ii], m_root.hostConfiguration(), m_atomTable, i);
        }
    }
}
