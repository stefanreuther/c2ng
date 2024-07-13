/**
  *  \file server/play/shipmissionpacker.cpp
  *  \brief Class server::play::ShipMissionPacker
  */

#include <stdexcept>
#include "server/play/shipmissionpacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "server/errors.hpp"
#include "server/types.hpp"

using game::spec::MissionList;
using game::spec::Mission;

namespace {
    int mapParameterType(Mission::ParameterType ty, Mission::ParameterFlagSet_t flags)
    {
        // These are the magic numbers used by PCC2 and exported on its server interface.
        int result = 0;
        switch (ty) {
         case Mission::NoParameter:                   break;
         case Mission::IntegerParameter: result |= 1; break;
         case Mission::PlanetParameter:  result |= 2; break;
         case Mission::ShipParameter:    result |= 3; break;
         case Mission::HereParameter:    result |= 4; break;
         case Mission::BaseParameter:    result |= 5; break;
         case Mission::PlayerParameter:  result |= 6; break;
        }
        if (flags.contains(Mission::NotThisParameter)) {
            result |= 32;
        }
        if (flags.contains(Mission::OwnParameter)) {
            result |= 16;
        }
        return result;
    }
}


server::play::ShipMissionPacker::ShipMissionPacker(game::Session& session, game::Id_t shipId)
    : m_session(session), m_shipId(shipId)
{ }

server::Value_t*
server::play::ShipMissionPacker::buildValue() const
{
    // ex ServerShipMissionWriter::write
    // @change this differs from PCC2 because it does not handle expressions!
    // We therefore send a different set of values.
    const game::Root& root = game::actions::mustHaveRoot(m_session);
    const game::Turn& turn = game::actions::mustHaveGame(m_session).currentTurn();
    const game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);
    const MissionList& ml = sl.missions();

    const game::map::Ship* pShip = turn.universe().ships().get(m_shipId);
    if (pShip == 0) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (MissionList::Iterator_t it = ml.begin(); it != ml.end(); ++it) {
        const Mission& m = *it;
        if (m.worksOn(*pShip, root.hostConfiguration(), root.hostVersion(), root.registrationKey())) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            hv->setNew("id", makeIntegerValue(m.getNumber()));
            hv->setNew("iarg", makeIntegerValue(mapParameterType(m.getParameterType(game::InterceptParameter),
                                                                 m.getParameterFlags(game::InterceptParameter))));
            hv->setNew("targ", makeIntegerValue(mapParameterType(m.getParameterType(game::TowParameter),
                                                                 m.getParameterFlags(game::TowParameter))));
            if (m.getParameterType(game::InterceptParameter) != Mission::NoParameter) {
                hv->setNew("iname", makeStringValue(m.getParameterName(game::InterceptParameter)));
            }
            if (m.getParameterType(game::TowParameter) != Mission::NoParameter) {
                hv->setNew("tname", makeStringValue(m.getParameterName(game::TowParameter)));
            }

            // Following attributes not in PCC2:
            hv->setNew("name", makeStringValue(m.getName()));
            hv->setNew("cond", makeStringValue(m.getConditionExpression()));
            hv->setNew("group", makeStringValue(m.getGroup()));
            if (char ch = m.getHotkey()) {
                hv->setNew("key", makeStringValue(String_t(1, ch)));
            }

            // Following attributes not published for now:
            // - getRaceMask [checked by worksOn]
            // - getFlags [partially checked by worksOn]
            // - getShortName
            // - getWarningExpression()
            // - getLabelExpression()
            // - getSetCommand()
            // These would be reconsidered when we switch to client-side mission processing.

            vv->pushBackNew(new afl::data::HashValue(hv));
        }
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::ShipMissionPacker::getName() const
{
    return afl::string::Format("shipmsn%d", m_shipId);
}
