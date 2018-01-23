/**
  *  \file server/host/talkadapter.cpp
  *  \brief Class server::host::TalkAdapter
  */

#include "server/host/talkadapter.hpp"
#include "server/host/game.hpp"
#include "afl/string/format.hpp"
#include "afl/string/char.hpp"

using server::interface::HostGame;

namespace {
    /* Simplify a newsgroup name.
       \param s Raw name */
    String_t simplifyNewsgroupName(const String_t s)
    {
        String_t result;
        bool needsp = false;
        for (String_t::size_type i = 0; i < s.size(); ++i) {
            const char ch = s[i];
            if (afl::string::charIsAlphanumeric(ch)) {
                if (needsp) {
                    result += '-';
                }
                result += afl::string::charToLower(ch);
                needsp = false;
            } else {
                needsp = (result.size() != 0);
            }
        }
        return result;
    }

    /* Make a sort key for a game name.
       This is required for forums.

       We want that things like "Pleiades 17" sort correctly (numerically),
       but we don't want to sort forums by game Id.
       We therefore rewrite game names for the keys: each sequence of numbers
       is prefixed by a digit count, i.e. "Pleiades 17" is turned into "Pleiades 00217".
       This way, games sort in numerical order when lexical sort is applied.
       For safety, we limit game names to 999 characters.
       Nobody is going to use more, and thus we have the safety that a three-digit
       sequence length is enough.

       \param gameName Game name
        \return Sort key */
    String_t makeSortKey(const String_t& gameName)
    {
        // Limit complexity
        String_t::size_type max = gameName.size();
        if (max > 999) {
            max = 999;
        }

        // Generate result
        String_t result;
        String_t::size_type i = 0;
        while (i < max) {
            if (gameName[i] >= '0' && gameName[i] <= '9') {
                // Special handling for numbers
                while (i < max && gameName[i] == '0') {
                    ++i;
                }

                String_t::size_type j = i;
                while (i < max && gameName[i] >= '0' && gameName[i] <= '9') {
                    ++i;
                }

                result.append(afl::string::Format("%03d", i - j));
                result.append(gameName, j, i-j);
            } else {
                result += afl::string::charToLower(gameName[i]);
                ++i;
            }
        }
        return result;
    }

    /* Configure the parent of a forum.
       \param req [in/out] List of configuration items
       \param isPublic [in] true for public forum */
    void setParent(afl::data::StringList_t& req, const bool isPublic)
    {
        req.push_back("parent");
        req.push_back(isPublic ? "active" : "active-unlisted");
    }

    /* Configure the name of a forum.
       \param req [in/out] List of configuration items
       \param gameName [in] Name of game/forum */
    void setName(afl::data::StringList_t& req, const String_t& gameName)
    {
        req.push_back("name"); req.push_back(gameName);
        req.push_back("key");  req.push_back(makeSortKey(gameName));
    }

    /* Configure the permissions of a forum.
       \param req [in/out] List of configuration items
       \param isPublic [in] true for public forum
       \param gameId [in] Game Id */
    void setPermissions(afl::data::StringList_t& req, const bool isPublic, const int32_t gameId)
    {
        if (isPublic) {
            req.push_back("readperm");   req.push_back("all");
            req.push_back("writeperm");  req.push_back("-u:anon,p:allowpost");
            req.push_back("answerperm"); req.push_back("-u:anon,p:allowpost");
        } else {
            const String_t perm = afl::string::Format("g:%d", gameId);
            req.push_back("readperm");   req.push_back(perm);
            req.push_back("writeperm");  req.push_back(perm);
            req.push_back("answerperm"); req.push_back(perm);
        }
    }
}


// Constructor.
server::host::TalkAdapter::TalkAdapter(server::interface::TalkForum& forum)
    : TalkListener(),
      m_forum(forum)
{ }



/** Game started.
    Called whenever a game enters an active state (becomes visible to users).
    In this case, a forum shall be created for the game.
    \param game      Game
    \param gameType  Game type (game.getType())

    Implementation:
    - if it's public, make a public forum
    - if it's private or unlisted, make an unlisted, private forum
    - otherwise, don't make a forum */
void
server::host::TalkAdapter::handleGameStart(Game& game, server::interface::HostGame::Type gameType)
{
    // ex talkHandleGameStart
    const bool isPublic = (gameType == HostGame::PublicGame);
    const bool isPrivate = (gameType == HostGame::PrivateGame || gameType == HostGame::UnlistedGame);
    if ((!isPublic && !isPrivate) || game.forumId().get() != 0 || game.forumDisabled().get() != 0) {
        // It is neither public nor private, or already has a forum, or should not have one.
        return;
    }

    // Set up forum
    const String_t gameName = game.getName();

    afl::data::StringList_t req;
    setParent(req, isPublic);
    setPermissions(req, isPublic, game.getId());
    setName(req, gameName);
    req.push_back("description"); req.push_back(afl::string::Format("forum:Forum for [game]%d[/game] (#%0$d)", game.getId()));
    req.push_back("newsgroup");   req.push_back("planetscentral.games." + simplifyNewsgroupName(afl::string::Format("%d-%s", game.getId(), gameName)));

    // Create it
    const int32_t forumId = m_forum.add(req);
    game.forumId().set(forumId);
}

/** Game finished.
    Called whenever a game finishes.
    In this case, a forum shall be retired.
    \param game      Game
    \param gameType  Game type (game.getType())

    Implementation: if the game has a forum which is in an "active" category,
    move it into the corresponding "finished" category. If the forum has already
    been manually moved, does nothing. */
void
server::host::TalkAdapter::handleGameEnd(Game& game, server::interface::HostGame::Type /*gameType*/)
{
    // ex talkHandleGameEnd
    // Get forum Id
    const int32_t forumId = game.forumId().get();
    if (forumId == 0) {
        return;
    }

    // Update forum
    String_t parent = m_forum.getStringValue(forumId, "parent");
    if (parent.size() >= 6 && parent.compare(0, 6, "active", 6) == 0) {
        parent.replace(0, 6, "finished", 8);

        afl::data::StringList_t req;
        req.push_back("parent");
        req.push_back(parent);
        m_forum.configure(forumId, req);
    }
}

/** Game name changed.
    This may affect the forum name.
    \param game      Game
    \param newName   New name (game.getName()) */
void
server::host::TalkAdapter::handleGameNameChange(Game& game, const String_t& newName)
{
    // ex talkHandleGameNameChange
    // Get forum Id
    const int32_t forumId = game.forumId().get();
    if (forumId == 0) {
        return;
    }

    // Update forum
    afl::data::StringList_t req;
    setName(req, newName);
    m_forum.configure(forumId, req);
}

/** Game type changed.
    This may affect the forum.
    \param game      Game
    \param gameState Game state (game.getState())
    \param gameType  Game type (game.getType())

    Implementation: if the game has a forum, and it is in an "active" category,
    move it to its proper "active" category. If the forum already is in another
    category (by having been moved manually, or by being in a "finished" category,
    this does not change the forum. */
void
server::host::TalkAdapter::handleGameTypeChange(Game& game, server::interface::HostGame::State /*gameState*/, server::interface::HostGame::Type gameType)
{
    // ex talkHandleGameTypeChange
    // Get forum Id
    const int32_t forumId = game.forumId().get();
    if (forumId == 0) {
        return;
    }

    // Check existing parent. Only if it's active, move it.
    String_t parent = m_forum.getStringValue(forumId, "parent");
    if (parent.size() >= 6 && parent.compare(0, 6, "active", 6) == 0) {
        // Update forum
        bool isPublic = (gameType == HostGame::PublicGame);

        afl::data::StringList_t req;
        setParent(req, isPublic);
        setPermissions(req, isPublic, game.getId());
        m_forum.configure(forumId, req);
    }
}

