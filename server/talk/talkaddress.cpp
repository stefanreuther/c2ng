/**
  *  \file server/talk/talkaddress.cpp
  *  \brief Class server::talk::TalkAddress
  */

#include <memory>
#include "server/talk/talkaddress.hpp"
#include "util/stringparser.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "server/talk/textnode.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"
#include "server/talk/render/render.hpp"
#include "server/talk/user.hpp"
#include "afl/string/format.hpp"

using afl::string::Format;

namespace {
    bool isGameOK(server::talk::Root& root, int32_t gameId)
    {
        // ex mail/edit.cgi:isGameOK
        //     # This is a very weak permission check, intended to filter out just the most obvious evils.
        //     # - the game must exist
        //     # - the game must not be deleted (=users from a deleted game do not receive messages).
        //     #   We need not check for joining/preparing; if the game has no users yet, no messages will
        //     #   be created.
        //     # xref planetscentral/host/game.cc, hasPermission
        return root.gameRoot().intSetKey("all").contains(gameId)
            && root.gameRoot().subtree(gameId).stringKey("state").get() != "deleted";
    }

    bool isSlotInGame(server::talk::Root& root, int32_t gameId, int32_t slotNr)
    {
        return root.gameRoot().subtree(gameId).subtree("player").subtree(slotNr).hashKey("status").intField("slot").get() != 0;
    }
}


server::talk::TalkAddress::TalkAddress(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::talk::TalkAddress::parse(afl::base::Memory<const String_t> in, afl::data::StringList_t& out)
{
    while (const String_t* p = in.eat()) {
        out.push_back(parseReceiver(*p));
    }
}

void
server::talk::TalkAddress::render(afl::base::Memory<const String_t> in, afl::data::StringList_t& out)
{
    while (const String_t* p = in.eat()) {
        if (m_session.renderOptions().getFormat() == "raw") {
            out.push_back(renderRawReceiver(*p));
        } else {
            std::auto_ptr<TextNode> node(new TextNode(TextNode::maParagraph, TextNode::miParFragment));
            if (renderReceiver(*p, *node)) {
                out.push_back(server::talk::render::renderText(node, server::talk::render::Context(m_root, m_session.getUser()), m_session.renderOptions(), m_root));
            } else {
                out.push_back(String_t());
            }
        }
    }
}

String_t
server::talk::TalkAddress::parseReceiver(const String_t& in)
{
    // ex mail/edit.cgi:processReceivers, sort-of
    util::StringParser p(in);
    if (p.parseEnd()) {
        // blank, ignore
        return String_t();
    } else if (p.parseString("g:")) {
        int gameNr, slotNr;
        if (p.parseInt(gameNr)) {
            if (p.parseEnd()) {
                // game name
                if (isGameOK(m_root, gameNr)) {
                    return Format("g:%d", gameNr);
                } else {
                    return String_t();
                }
            } else if (p.parseString(":") && p.parseInt(slotNr) && p.parseEnd()) {
                // game + player name
                // We check actual slot presence
                if (isGameOK(m_root, gameNr) && isSlotInGame(m_root, gameNr, slotNr)) {
                    return Format("g:%d:%d", gameNr, slotNr);
                } else {
                    return String_t();
                }
            } else {
                // error
                return String_t();
            }
        } else {
            // error
            return String_t();
        }
    } else {
        // User name
        String_t userId = m_root.getUserIdFromLogin(in);
        if (!userId.empty()) {
            return "u:" + userId;
        } else {
            return String_t();
        }
    }
}

bool
server::talk::TalkAddress::renderReceiver(const String_t& in, TextNode& out)
{
    // ex mail/view.cgi:formatReceivers, sort-of
    // xref talkpm.cpp:parseReceiver (FIXME: merge?)
    util::StringParser p(in);
    if (p.parseEnd()) {
        // blank, ignore
        return false;
    } else if (p.parseString("g:")) {
        int gameNr, slotNr;
        if (p.parseInt(gameNr)) {
            if (p.parseEnd()) {
                // game name
                if (isGameOK(m_root, gameNr)) {
                    out.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "players of "));
                    out.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, Format("%d", gameNr)));
                    return true;
                } else {
                    return false;
                }
            } else if (p.parseString(":") && p.parseInt(slotNr) && p.parseEnd()) {
                // game + player name
                if (isGameOK(m_root, gameNr) && isSlotInGame(m_root, gameNr, slotNr)) {
                    out.children.pushBackNew(new TextNode(TextNode::maPlain, 0, Format("player %d in ", slotNr)));
                    out.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, Format("%d", gameNr)));
                    return true;
                } else {
                    return false;
                }
            } else {
                // error
                return false;
            }
        } else {
            // error
            return false;
        }
    } else if (p.parseString("u:")) {
        // User name
        String_t userName = User(m_root, p.getRemainder()).getLoginName();
        if (userName.empty()) {
            // User does not exist
            return false;
        } else {
            // OK
            out.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, userName));
        }
        return true;
    } else {
        return false;
    }
}

String_t
server::talk::TalkAddress::renderRawReceiver(const String_t& in)
{
    util::StringParser p(in);
    if (p.parseEnd()) {
        // blank, ignore
        return String_t();
    } else if (p.parseString("g:")) {
        int gameNr, slotNr;
        if (p.parseInt(gameNr)) {
            if (p.parseEnd()) {
                // game name
                if (isGameOK(m_root, gameNr)) {
                    return Format("g:%d", gameNr);
                } else {
                    return String_t();
                }
            } else if (p.parseString(":") && p.parseInt(slotNr) && p.parseEnd()) {
                // game + player name
                if (isGameOK(m_root, gameNr) && isSlotInGame(m_root, gameNr, slotNr)) {
                    return Format("g:%d:%d", gameNr, slotNr);
                } else {
                    return String_t();
                }
            } else {
                // error
                return String_t();
            }
        } else {
            // error
            return String_t();
        }
    } else if (p.parseString("u:")) {
        // User name
        return User(m_root, p.getRemainder()).getLoginName();
    } else {
        return String_t();
    }
}
