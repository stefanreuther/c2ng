/**
  *  \file server/interface/hosthistoryserver.cpp
  *  \brief Class server::interface::HostHistoryServer
  */

#include <stdexcept>
#include "server/interface/hosthistoryserver.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/errors.hpp"
#include "server/interface/hosthistory.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::HostHistoryServer::HostHistoryServer(HostHistory& impl)
    : m_implementation(impl)
{ }

server::interface::HostHistoryServer::~HostHistoryServer()
{ }

bool
server::interface::HostHistoryServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "HISTEVENTS") {
        /* @q HISTEVENTS [GAME game:GID] [USER user:UID] [LIMIT n:Int] (Host Command)
           Get global events.

           Returns an array of events, starting with the newest.
           Each event is represented as a hash.

           Events are selected by the given parameters.
           Note that these are not just filters;
           querying a game's or user's events will return more events than querying the global event list.

           @retkey time:Time            Event time
           @retkey event:Str            Event. For example, "game-join". See {global:history} for more values.
           @retkey game:GID             Game Id (optional)
           @retkey gameName:Str         Game name (optional)
           @retkey user:UID             User Id (optional)
           @retkey slot:Int             Slot number (optional)
           @retkey state:HostGameState  Game state (optional)

           @uses global:history, game:$GID:history, user:$UID:history */
        // Parse args
        HostHistory::EventFilter filter;
        while (args.getNumArgs() > 0) {
            String_t key = afl::string::strUCase(toString(args.getNext()));
            if (key == "GAME") {
                args.checkArgumentCountAtLeast(1);
                filter.gameId = toInteger(args.getNext());
            } else if (key == "USER") {
                args.checkArgumentCountAtLeast(1);
                filter.userId = toString(args.getNext());
            } else if (key == "LIMIT") {
                args.checkArgumentCountAtLeast(1);
                filter.limit = toInteger(args.getNext());
            } else {
                throw std::runtime_error(SYNTAX_ERROR);
            }
        }

        // Produce output
        afl::container::PtrVector<HostHistory::Event> es;
        m_implementation.getEvents(filter, es);

        // Format output
        Vector::Ref_t v = Vector::create();
        for (size_t i = 0; i < es.size(); ++i) {
            if (const HostHistory::Event* pe = es[i]) {
                Hash::Ref_t h = Hash::create();
                h->setNew("time", makeIntegerValue(pe->time));
                h->setNew("event", makeStringValue(pe->eventType));
                addOptionalIntegerKey(*h, "game", pe->gameId);
                addOptionalStringKey(*h, "gameName", pe->gameName);
                addOptionalStringKey(*h, "user", pe->userId);
                addOptionalIntegerKey(*h, "slot", pe->slotNumber);
                if (const HostGame::State* p = pe->gameState.get()) {
                    h->setNew("state", makeStringValue(HostGame::formatState(*p)));
                }
                v->pushBackNew(new HashValue(h));
            }
        }
        result.reset(new VectorValue(v));
        return true;
    } else if (upcasedCommand == "HISTTURN") {
        /* @q HISTTURN game:GID [LIMIT n:Int] [UNTIL turn:Int] [SINCETIME t:Time] [SCORE name:Str] [STATUS] [PLAYER] (Host Command)
           Get turn history.

           Returns an array of turns, starting with the oldest (i.e. ascending turn number).
           Each turn is represented as a hash.

           Turns are selected by the LIMIT/UNTIL/SINCETIME options.
           The returned content is selected by the SCORE/STATUS/PLAYER options.

           @retkey turn:Int             Turn number
           @retkey players:StrList      Players (optional)
           @retkey turns:IntList        Turn states (optional)
           @retkey scores:IntList       Scores (optional)
           @retkey time:Time            Turn time
           @retkey timestamp:Str        Turn timestamp

           @uses game:$GID:turn:$TURN:scores, game:$GID:turn:$TURN:info */
        // Parse args
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());
        HostHistory::TurnFilter filter;
        while (args.getNumArgs() > 0) {
            String_t key = afl::string::strUCase(toString(args.getNext()));
            if (key == "LIMIT") {
                args.checkArgumentCountAtLeast(1);
                filter.limit = toInteger(args.getNext());
            } else if (key == "UNTIL") {
                args.checkArgumentCountAtLeast(1);
                filter.endTurn = toInteger(args.getNext());
            } else if (key == "SINCETIME") {
                args.checkArgumentCountAtLeast(1);
                filter.startTime = toInteger(args.getNext());
            } else if (key == "SCORE") {
                args.checkArgumentCountAtLeast(1);
                filter.scoreName = toString(args.getNext());
            } else if (key == "STATUS") {
                filter.reportStatus = true;
            } else if (key == "PLAYER") {
                filter.reportPlayers = true;
            } else {
                throw std::runtime_error(SYNTAX_ERROR);
            }
        }

        // Produce output
        afl::container::PtrVector<HostHistory::Turn> ts;
        m_implementation.getTurns(gameId, filter, ts);

        // Format output
        Vector::Ref_t v = Vector::create();
        for (size_t i = 0; i < ts.size(); ++i) {
            if (const HostHistory::Turn* pt = ts[i]) {
                Hash::Ref_t h = Hash::create();
                h->setNew("turn", makeIntegerValue(pt->turnNumber));
                h->setNew("time", makeIntegerValue(pt->time));
                h->setNew("timestamp", makeStringValue(pt->timestamp));
                if (!pt->slotPlayers.empty()) {
                    Vector::Ref_t vv = Vector::create();
                    vv->pushBackElements(pt->slotPlayers);
                    h->setNew("players", new VectorValue(vv));
                }
                if (!pt->slotStates.empty()) {
                    Vector::Ref_t vv = Vector::create();
                    vv->pushBackElements(pt->slotStates);
                    h->setNew("turns", new VectorValue(vv));
                }
                if (!pt->slotScores.empty()) {
                    Vector::Ref_t vv = Vector::create();
                    vv->pushBackElements(pt->slotScores);
                    h->setNew("scores", new VectorValue(vv));
                }
                v->pushBackNew(new HashValue(h));
            }
        }
        result.reset(new VectorValue(v));
        return true;
    } else {
        return false;
    }
}
