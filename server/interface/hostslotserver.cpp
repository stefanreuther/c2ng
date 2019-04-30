/**
  *  \file server/interface/hostslotserver.cpp
  *  \brief Class server::interface::HostSlotServer
  */

#include "server/interface/hostslotserver.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

server::interface::HostSlotServer::HostSlotServer(HostSlot& impl)
    : m_implementation(impl)
{ }

server::interface::HostSlotServer::~HostSlotServer()
{ }

bool
server::interface::HostSlotServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "SLOTADD") {
        /* @q SLOTADD game:GID slot:Int... (Host Command)
           Add slots to a game.
           If the slots are already part of the game, changes nothing.
           Slots can only be added to games that have not yet been mastered.

           Permissions: admin-access to game

           @uses game:$GID:player:$P:status */
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());

        afl::data::IntegerList_t slots;
        while (args.getNumArgs() > 0) {
            slots.push_back(toInteger(args.getNext()));
        }
        m_implementation.add(gameId, slots);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SLOTRM") {
        /* @q SLOTRM game:GID slot:Int... (Host Command)
           Remove slots from a game.
           If the slots are not already part of the game, changes nothing.
           Slots can only be removed if they do not have any players,
           from games that have not yet been mastered,

           Permissions: admin-access to game

           @uses game:$GID:player:$P:status */
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());

        afl::data::IntegerList_t slots;
        while (args.getNumArgs() > 0) {
            slots.push_back(toInteger(args.getNext()));
        }
        m_implementation.remove(gameId, slots);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SLOTLS") {
        /* @q SLOTLS game:GID (Host Command)
           List slots of a game.

           Permissions: read-access to game

           @uses game:$GID:player:$P:status
           @retval IntList list of slots */
        args.checkArgumentCount(1);

        afl::data::IntegerList_t slots;
        m_implementation.getAll(toInteger(args.getNext()), slots);

        afl::data::Vector::Ref_t v = afl::data::Vector::create();
        v->pushBackElements(slots);
        result.reset(new afl::data::VectorValue(v));

        return true;
    } else {
        return false;
    }
}
