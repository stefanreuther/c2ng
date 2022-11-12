/**
  *  \file game/msg/format.cpp
  *  \brief Message Formatting
  */

#include <cstring>
#include "game/msg/format.hpp"
#include "afl/string/format.hpp"
#include "game/map/point.hpp"
#include "game/msg/outbox.hpp"
#include "game/parser/messagetemplate.hpp"
#include "util/rich/linkattribute.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"

namespace {
    /** Find X,Y coordinate pair.
        \param str [in] String
        \param n   [in] Index into string, position of a ','
        \param start,end [out] Position of result coordinate in string
        \param pt  [out] Parsed coordinate
        \return true if found, false if this is not a valid coordinate pair */
    bool findXY(const String_t& str, size_t n, size_t& start, size_t& end, game::map::Point& pt)
    {
        // ex client/widgets/msgaction.h:findXY
        /* Punctuation required before/after coordinate pair.
           The idea is to accept things like
              1111,1111
              (1111,1111)
              move to->1111,1111
              ship @1111,1111
           but reject things like
              map size 2000x2000, 50 ly seam
           (which would be interpreted as '2000,50' without the
           punctuation filter). Since this function gets a whole
           message, we must also accept '\n' as whitespace. */
        static const char PUNCT[] = "\n ():@-<=>.!";

        /* Locate X before n */
        int x = 0;
        int xdig = 0;
        int xmul = 1;
        size_t xn = n;
        while (xn > 0 && str[xn-1] == ' ') {
            /* Skip space */
            --xn;
        }
        while (xn > 0 && str[xn-1] >= '0' && str[xn-1] <= '9' && xdig < 5) {
            /* Produce digits */
            x += (str[xn-1] - '0')*xmul;
            xmul *= 10;
            ++xdig;
            --xn;
        }
        if (xdig > 4 || xdig < 3) {
            /* Coordinates cannot have more than 4 digits */
            return false;
        }
        if (xn > 0 && std::strchr(PUNCT, str[xn-1]) == 0) {
            /* Coordinates not preceded by proper punctuation */
            return false;
        }

        /* Locate Y after n */
        int y = 0;
        int ydig = 0;
        size_t yn = n+1;
        while (yn < str.size() && str[yn] == ' ') {
            /* Skip space */
            ++yn;
        }
        while (yn < str.size() && str[yn] >= '0' && str[yn] <= '9' && ydig < 5) {
            /* Produce digits */
            y *= 10;
            y += str[yn] - '0';
            ++ydig;
            ++yn;
        }
        if (ydig > 4 || ydig < 3) {
            /* Coordinates cannot have more than 4 digits */
            return false;
        }
        if (yn < str.size() && std::strchr(PUNCT, str[yn]) == 0) {
            /* Coordinates not followed by proper punctuation */
            return false;
        }

        /* Reject invalid values */
        if (x == 0 || y == 0) {
            return false;
        }

        /* Success */
        pt = game::map::Point(x, y);
        start = xn;
        end = yn;
        return true;
    }

    /** Check for message header. Header lines must fulfill the regexp
        "[A-Z]+ *:", i.e. any single word followed by optional spaces and a
        colon. We want to recognize
          "TO: race"     (THost)
          "TO  : race"   (PHost, English)
          "An  : race"   (PHost, German)
          "CC: race"     (PCC)
        We do not need to recognize "<<<Universal Message>>>", this is handled
        on the outside. We do not need to handle "<CC:" and the blank line
        between host's headers and ours; we see the message in
        "cooked" format after these idiosyncrasies have been resolved.

        \todo This does not recognize Estonian
          " SAAJA: race"
        but I consider Estonian in error here. This does not recognize
        Russian, which uses Cyrillic letters in the headers.

        Original: readmsg.pas, IsHeader */
    bool isHeader(const String_t& line)
    {
        String_t::size_type n = 0;
        while (n < line.size() && std::isalpha(uint8_t(line[n]))) {
            ++n;
        }
        if (n == 0) {
            return false;
        }
        while (n < line.size() && line[n] == ' ') {
            ++n;
        }
        return (n < line.size() && line[n] == ':');
    }

    /** Parse receivers out of a "TO:", "CC:" header. */
    game::PlayerSet_t parseReceivers(const char* str, const game::PlayerList& players, afl::string::Translator& tx)
    {
        // Check for 'TO: The Solar Federation'
        const String_t s = afl::string::strTrim(afl::string::strUCase(str));
        for (game::Player* p = players.getFirstPlayer(); p != 0; p = players.getNextPlayer(p)) {
            if (afl::string::strTrim(afl::string::strUCase(p->getName(game::Player::OriginalLongName, tx))) == s) {
                return game::PlayerSet_t(p->getId());
            }
        }

        // Check for combinations
        game::PlayerSet_t result;
        util::StringParser p(s);
        while (!p.parseEnd()) {
            int i;
            if (p.parseCharacter(' ')) {
                // skip
            } else if (p.parseString("EVERYBODY")) {
                // GH uses 'TO: Everybody' instead of '<<< Universal Message >>>'
                result += players.getAllPlayers();
            } else if (p.parseString("HOST")) {
                // 'TO: Host'
                result += 0;
            } else if (p.parseInt(i) && i > 0 && i <= game::MAX_PLAYERS) {
                // 'TO: 1 2 3'
                result += i;
            } else {
                p.consumeCharacter();
            }
        }
        return result;
    }

    /** Parse 'TO:', 'CC:' headers out of a message. Update replyAll. */
    void parseExtraReceivers(game::PlayerSet_t& replyAll, const game::parser::MessageLines_t& lines, const game::PlayerList& players, afl::string::Translator& tx)
    {
        // ex readmsg.pas:GetOrigReceivers
        for (size_t i = 1 /* after '<<< Subspace Message >>>' */; i < lines.size(); ++i) {
            const char* p;
            if ((p = util::strStartsWith(lines[i], "CC:")) != 0 || (p = util::strStartsWith(lines[i], "TO:")) != 0) {
                // Process 'TO:' for the benefit of GH (which sends only a TO line, no CC line).
                // For in-game messages, we need not process 'TO:' because that'll always be us.
                replyAll |= parseReceivers(p, players, tx);
            } else if (lines[i] == game::msg::Outbox::UNIVERSAL_TEXT) {
                replyAll |= players.getAllPlayers();
            } else if (isHeader(lines[i])) {
                // could be TURN header, skip
            } else {
                // not a header at all, stop
                break;
            }
        }
    }
}


game::msg::Format
game::msg::formatMessage(const String_t& in, const PlayerList& players, afl::string::Translator& tx)
{
    // ex WMessageDisplay::onChangeMessage (most of it)
    // Reset
    Format out;

    // Add text with links
    size_t n = 0;
    size_t p;
    while ((p = in.find(',', n)) != String_t::npos) {
        size_t start, end;
        game::map::Point pt;
        if (findXY(in, p, start, end, pt) && start >= n) {
            // We found a coordinate
            out.text.append(String_t(in, n, start-n));
            out.text.append(util::rich::Text(String_t(in, start, end-start))
                            .withNewAttribute(new util::rich::LinkAttribute(afl::string::Format("%d,%d", pt.getX(), pt.getY()))));
            if (!out.firstLink.isSet()) {
                out.firstLink = pt;
            }
            n = end;
        } else {
            // No coordinate found
            out.text.append(String_t(in, n, p+1-n));
            n = p+1;
        }
    }
    out.text.append(String_t(in, n));

    // Reply information and primary object guess
    // At this place, only extract reliable/simple information; more complex object associations will be done by the message parser.
    // ex readmsg.pas:EvaluateMessageTarget
    game::parser::MessageLines_t lines;
    game::parser::splitMessage(lines, in);
    switch (getMessageHeaderInformation(lines, game::parser::MsgHdrKind)) {
     case 'r': // player-to-player message
        if (Player* p = players.getPlayerFromCharacter(char(getMessageHeaderInformation(lines, game::parser::MsgHdrSubId)))) {
            // Reply to sender
            if (p->getId() == 0) {
                out.reply = players.getAllPlayers();
            } else {
                out.reply = PlayerSet_t(p->getId());
            }

            // Reply all
            out.replyAll = out.reply;
            parseExtraReceivers(out.replyAll, lines, players, tx);
        }
        break;

     case 'g': // configuration
     case 'h': // from host
        out.reply = out.replyAll = PlayerSet_t(0);
        parseExtraReceivers(out.replyAll, lines, players, tx);
        break;

     case 'u': // Ufo
        if (int32_t id = getMessageHeaderInformation(lines, game::parser::MsgHdrBigId)) {
            out.headerLink = Reference(Reference::Ufo, id);
        }
        break;

     case 'p': // planet
     case 't': // terraform
     case 'y': // meteor
     case 'z': // sensor sweep
        if (int32_t id = getMessageHeaderInformation(lines, game::parser::MsgHdrId)) {
            out.headerLink = Reference(Reference::Planet, id);
        }
        break;

     case 'd': // space dock
        if (int32_t id = getMessageHeaderInformation(lines, game::parser::MsgHdrId)) {
            out.headerLink = Reference(Reference::Starbase, id);
        }
        break;

     case 's': // ship
     case 'w': // web mines
        if (int32_t id = getMessageHeaderInformation(lines, game::parser::MsgHdrId)) {
            out.headerLink = Reference(Reference::Ship, id);
        }
        break;

     case 'i': // ion storm
        if (int32_t id = getMessageHeaderInformation(lines, game::parser::MsgHdrId)) {
            out.headerLink = Reference(Reference::Storm, id);
        }
        break;

     case 'l': // mines laid
     case 'm': // mines scanned
        if (int32_t id = getMessageHeaderInformation(lines, game::parser::MsgHdrBigId)) {
            out.headerLink = Reference(Reference::Minefield, id);
        }
        break;

        // Do not handle 'f'. PCC1 applies heuristic to search it in VCRs.
        // Do not handle 'e', 'n'. PCC1 applies heuristic to detect RGA/Pillage.

     default:
        break;
    }

    return out;
}

String_t
game::msg::quoteMessageForReply(const String_t& originalText)
{
    // ex WMessageActionPanel::doReply(), readmsg.pas:QuoteMessage
    // Split message into lines
    game::parser::MessageLines_t lines;
    game::parser::splitMessage(lines, originalText);

    // Skip headers. First line always is (-foo). Also accept plain <<< >>> (before Host 3.2).
    size_t first = 0;
    if (first < lines.size() && (lines[0].size() == 0 || lines[0][0] == '(' || lines[0][0] == '<')) {
        ++first;
    }

    // Skip more headers.
    while (first < lines.size()
           && (lines[first].size() == 0
               || lines[first] == game::msg::Outbox::UNIVERSAL_TEXT
               || isHeader(lines[first])))
    {
        ++first;
    }

    // Quote remainder.
    String_t quotedMessage;
    bool wasEmpty = false;
    while (first < lines.size()) {
        if (lines[first].empty()) {
            wasEmpty = true;
        } else {
            if (wasEmpty) {
                quotedMessage += ">\n";
            }
            quotedMessage += '>';
            if (lines[first].size() > 0 && lines[first][0] != '>') {
                quotedMessage += ' ';
            }
            quotedMessage += lines[first];
            quotedMessage += '\n';
            wasEmpty = false;
        }
        ++first;
    }
    return quotedMessage;
}
