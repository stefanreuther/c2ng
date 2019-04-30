/**
  *  \file server/interface/hostturnserver.cpp
  */

#include <stdexcept>
#include "server/interface/hostturnserver.hpp"
#include "server/interface/hostturn.hpp"
#include "server/errors.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;

server::interface::HostTurnServer::HostTurnServer(HostTurn& impl)
    : m_implementation(impl)
{ }

server::interface::HostTurnServer::~HostTurnServer()
{ }

bool
server::interface::HostTurnServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "TRN") {
        /* @q TRN content:Blob [GAME game:GID] [SLOT slot:Int] [MAIL email:Str] [INFO info:Str] (Host Command)
           Submit a turn file.
           The turn file is automatically matched to the game using the timestamp.
           The %game and %slot parameters can be used to determine the intended recipient
           and cause the file to be rejected if it does not match.

           In player context, the player must be permitted to submit this turn file.
           If admin context, the %email parameter can be used to auto-detect the player;
           if it is not given, the turn file is accepted unconditionally.

           The %info parameter is written to the logfile.

           Permissions: admin or user themselves.

           @retkey status:HostTurnStatus (new turn file status)
           @retkey output:Str (turn checker output)
           @retkey game:GID (detected game Id, same as %game parameter if specified)
           @retkey slot:Int (detected slot number, same as %slot parameter if specified)
           @retkey previous:HostTurnStatus (previous turn file state)
           @retkey user:UID (can be blank)
           @retkey name:Str (game name, since 2.40.6)
           @retkey allowtemp:Bool (allow TRNMARKTEMP command, since 2.40.6)
           @retkey turn:Int (turn number, since 2.40.6)

           @err 404 Game does not exist
           @err 403 Permission denied (only admin can use MAIL; user does not play this race)
           @err 407 Mail mismatch (email address does not match player of this race)
           @err 412 Wrong game state (game is not running)

           @uses game:bytime:$TIMESTAMP, user:active, PUT (File Command) */

        // Parse commands
        args.checkArgumentCountAtLeast(1);
        String_t trnData = toString(args.getNext());
        afl::base::Optional<int32_t> game;
        afl::base::Optional<int32_t> slot;
        afl::base::Optional<String_t> mail;
        afl::base::Optional<String_t> info;
        while (args.getNumArgs() > 0) {
            args.checkArgumentCountAtLeast(1);
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "GAME") {
                args.checkArgumentCountAtLeast(1);
                game = toInteger(args.getNext());
            } else if (keyword == "SLOT") {
                args.checkArgumentCountAtLeast(1);
                slot = toInteger(args.getNext());
            } else if (keyword == "MAIL") {
                args.checkArgumentCountAtLeast(1);
                mail = toString(args.getNext());
            } else if (keyword == "INFO") {
                args.checkArgumentCountAtLeast(1);
                info = toString(args.getNext());
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        // Execute
        HostTurn::Result r = m_implementation.submit(trnData, game, slot, mail, info);

        // Produce result
        Hash::Ref_t h = Hash::create();
        h->setNew("status",   makeIntegerValue(r.state));
        h->setNew("output",   makeStringValue(r.output));
        h->setNew("game",     makeIntegerValue(r.gameId));
        h->setNew("slot",     makeIntegerValue(r.slot));
        h->setNew("previous", makeIntegerValue(r.previousState));
        h->setNew("user",     makeStringValue(r.userId));
        h->setNew("name",     makeStringValue(r.gameName));
        h->setNew("turn",     makeIntegerValue(r.turnNumber));
        h->setNew("allowtemp", makeIntegerValue(r.allowTemp));
        result.reset(new HashValue(h));
        return true;
    } else if (upcasedCommand == "TRNMARKTEMP") {
        /* @q TRNMARKTEMP game:GID slot:Int flag:Int (Host Command)
           Mark turn temporary.
           - flag=1: this is a temporary turn that will not trigger "run host when turns are in"
           - flag=0: this is a final turn

           Permissions: admin or user themselves.

           @err 412 Slot not in use (slot is not in use)
           @err 412 Bad turn state (no turn file available for this slot)
           @err 403 Permission denied (caller is not admin and does not play this slot) */
        args.checkArgumentCount(3);
        int32_t gameId = toInteger(args.getNext());
        int32_t slotId = toInteger(args.getNext());
        int32_t enable = toInteger(args.getNext());
        m_implementation.setTemporary(gameId, slotId, enable != 0);
        result.reset(makeStringValue("OK"));
        return true;
    } else {
        return false;
    }
}
