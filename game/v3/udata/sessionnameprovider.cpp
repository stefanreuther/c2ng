/**
  *  \file game/v3/udata/sessionnameprovider.cpp
  *  \brief Class game::v3::udata::SessionNameProvider
  */

#include "game/v3/udata/sessionnameprovider.hpp"
#include "game/tables/nativegovernmentname.hpp"
#include "game/spec/shiplist.hpp"
#include "game/tables/nativeracename.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/map/planet.hpp"

game::v3::udata::SessionNameProvider::SessionNameProvider(Session& session)
    : m_session(session)
{ }

String_t
game::v3::udata::SessionNameProvider::getName(Type type, int id) const
{
    switch (type) {
     case HullFunctionName:
        if (const game::spec::ShipList* sl = m_session.getShipList().get()) {
            if (const game::spec::BasicHullFunction* hf = sl->basicHullFunctions().getFunctionById(id)) {
                return hf->getName();
            }
        }
        break;

     case HullName:
        if (const game::spec::ShipList* sl = m_session.getShipList().get()) {
            if (const game::spec::Hull* h = sl->hulls().get(id)) {
                return h->getName(sl->componentNamer());
            }
        }
        break;

     case NativeGovernmentName:
        return game::tables::NativeGovernmentName(m_session.translator()).get(id);

     case NativeRaceName:
        return game::tables::NativeRaceName(m_session.translator()).get(id);

     case PlanetName:
        if (Game* g = m_session.getGame().get()) {
            if (const game::map::Planet* pl = g->currentTurn().universe().planets().get(id)) {
                return pl->getName(m_session.translator());
            }
        }
        break;

     case ShortRaceName:
        if (Root* r = m_session.getRoot().get()) {
            return r->playerList().getPlayerName(id, Player::ShortName, m_session.translator());
        }
        break;
    }
    return String_t();
}
