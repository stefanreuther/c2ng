/**
  *  \file game/parser/format.cpp
  *
  *  FIXME: reconsider naming, placement and responsibilities of this file
  */

#include <cstring>
#include "game/parser/format.hpp"
#include "util/rich/linkattribute.hpp"
#include "afl/string/format.hpp"
#include "game/map/point.hpp"
#include "game/parser/messagetemplate.hpp"

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
}


void
game::parser::formatMessage(Format& out, const String_t& in, const PlayerList& players)
{
    // ex WMessageDisplay::onChangeMessage (most of it)
    // FIXME: what to use as link syntax?
    // Reset
    out = Format();

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

    // Reply information
    // FIXME: this should normally be mailbox specific: only the v3 inbox knows that it uses these kind of tags
    MessageLines_t lines;
    splitMessage(lines, in);
    switch (getMessageHeaderInformation(lines, MsgHdrKind)) {
     case 'r':
        if (Player* p = players.getPlayerFromCharacter(char(getMessageHeaderInformation(lines, MsgHdrSubId)))) {
            if (p->getId() == 0) {
                out.reply = out.replyAll = players.getAllPlayers();
            } else {
                // FIXME: correct parsing of reply all
                out.reply = out.replyAll = PlayerSet_t(p->getId());
            }
        }
        break;

     case 'g':
     case 'h':
        out.reply = out.replyAll = PlayerSet_t(0);
        break;

     default:
        break;
    }
}
