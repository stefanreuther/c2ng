/**
  *  \file game/nu/gamestate.cpp
  *  \brief Class game::nu::GameState
  */

#include "game/nu/gamestate.hpp"
#include "afl/base/countof.hpp"
#include "afl/net/headertable.hpp"
#include "afl/string/format.hpp"
#include "game/nu/browserhandler.hpp"

namespace {
    // Race names. Nu has these built-in to its JavaScript.
    const char*const RACE_NAMES[][3] = {
        { "The Solar Federation", "The Feds", "Fed" },
        { "The Lizard Alliance", "The Lizards", "Lizard" },
        { "The Empire of the Birds", "The Bird Men", "Bird Man" },
        { "The Fascist Empire", "The Fascists", "Fascist" },
        { "The Privateer Bands", "The Privateers", "Privateer" },
        { "The Cyborg", "The Cyborg", "Cyborg" },
        { "The Crystal Confederation", "The Crystal People", "Crystalline" },
        { "The Evil Empire", "The Evil Empire", "Empire" },
        { "The Robotic Imperium", "The Robots", "Robotic" },
        { "The Rebel Confederation", "The Rebels", "Rebel" },
        { "The Missing Colonies of Man", "The Colonies", "Colonial" },
        { "The Horwasp Plague", "The Horwasp", "Horwasp" },
    };
}



game::nu::GameState::GameState(BrowserHandler& handler, game::browser::Account& acc, int32_t gameNr, size_t hint)
    : m_handler(handler),
      m_account(acc),
      m_gameNr(gameNr),
      m_hint(hint),
      m_resultValid(false),
      m_result()
{ }

game::nu::GameState::~GameState()
{ }

afl::data::Access
game::nu::GameState::loadResultPreAuthenticated()
{
    if (!m_resultValid) {
        // Try to load the result
        if (const String_t* key = m_account.get("api_key")) {
            afl::net::HeaderTable tab;
            tab.add("gameid", afl::string::Format("%d", m_gameNr));
            tab.add("apikey", *key);
            tab.add("forsave", "true");
            tab.add("activity", "true");  // not sure what this is for...
            m_result = m_handler.callServer(m_account, "/game/loadturn", tab);
            m_resultValid = true;
        } else {
            // FIXME: log this failure. Better yet: handle it.
            // Could happen if we open a game without going through the browser first.
        }
    }
    return m_result;
}

afl::data::Access
game::nu::GameState::loadGameListEntryPreAuthenticated()
{
    afl::data::Access a = m_handler.getGameListPreAuthenticated(m_account);

    // Try the hint
    {
        afl::data::Access guess = a("games")[m_hint];
        if (guess("game")("id").toInteger() == m_gameNr) {
            return guess;
        }
    }

    // No, find it
    for (size_t i = 0, n = a("games").getArraySize(); i < n; ++i) {
        afl::data::Access elem = a("games")[i];
        if (elem("game")("id").toInteger() == m_gameNr) {
            m_hint = i;
            return elem;
        }
    }
    return afl::data::Access();
}

std::auto_ptr<game::Task_t>
game::nu::GameState::login(std::auto_ptr<Task_t> then)
{
    return m_handler.login(m_account, then);
}

void
game::nu::GameState::invalidateResult()
{
    m_resultValid = false;
    m_result.reset();
}

bool
game::nu::GameState::setRaceName(Player& pl, int race)
{
    if (race > 0 && race <= int(countof(RACE_NAMES))) {
        pl.setName(Player::LongName, RACE_NAMES[race-1][0]);
        pl.setName(Player::ShortName, RACE_NAMES[race-1][1]);
        pl.setName(Player::AdjectiveName, RACE_NAMES[race-1][2]);
        return true;
    } else {
        return false;
    }
}
