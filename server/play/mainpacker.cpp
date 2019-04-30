/**
  *  \file server/play/mainpacker.cpp
  */

#include "server/play/mainpacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "game/interface/globalcontext.hpp"
#include "game/spec/shiplist.hpp"

server::play::MainPacker::MainPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::MainPacker::buildValue() const
{
    // ex ServerMainWriter::write
    game::interface::GlobalContext ctx(m_session);

    afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
    addValue(*hv, ctx, "MY.INMSGS", "MY.INMSGS");
    addValue(*hv, ctx, "MY.OUTMSGS", "MY.OUTMSGS");
    addValue(*hv, ctx, "MY.RACE$", "MY.RACE");
    addValue(*hv, ctx, "MY.RACE.ID", "MY.RACE.ID");
    addValue(*hv, ctx, "MY.RACE.MISSION", "MY.RACE.MISSION");
    addValue(*hv, ctx, "MY.VCRS", "MY.VCRS");
    addValue(*hv, ctx, "SYSTEM.GAMETYPE$", "SYSTEM.GAMETYPE");
    addValue(*hv, ctx, "SYSTEM.HOST", "SYSTEM.HOST");
    addValue(*hv, ctx, "SYSTEM.HOST$", "SYSTEM.HOST$");
    addValue(*hv, ctx, "SYSTEM.HOSTVERSION", "SYSTEM.HOSTVERSION");
    addValue(*hv, ctx, "SYSTEM.REGSTR1", "SYSTEM.REGSTR1");
    addValue(*hv, ctx, "SYSTEM.REGSTR2", "SYSTEM.REGSTR2");
    addValue(*hv, ctx, "SYSTEM.VERSION", "SYSTEM.VERSION");
    addValue(*hv, ctx, "TURN", "TURN");
    addValue(*hv, ctx, "TURN.DATE", "TURN.DATE");
    addValue(*hv, ctx, "TURN.TIME", "TURN.TIME");

    // Number of hulls is needed for downloading all hulls
    if (game::spec::ShipList* sl = m_session.getShipList().get()) {
        addValueNew(*hv, makeIntegerValue(sl->hulls().size()), "NUMHULLS");
    }

    // FIXME: Global properties
    // w.startObject("PROP");
    // writeProperties(w);
    // w.endObject();

    return new afl::data::HashValue(hv);
}

String_t
server::play::MainPacker::getName() const
{
    return "main";
}
