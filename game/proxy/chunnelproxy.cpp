/**
  *  \file game/proxy/chunnelproxy.cpp
  *  \brief Class game::proxy::ChunnelProxy
  *
  *  FIXME: as of 20200102, this is a low-fi version:
  *
  *  - postCandidateRequest should create a trampoline to report changes
  *  - getCandidates should actually be implemented as a ReferenceListProxy::Initializer_t descendant,
  *    so we can use ReferenceListProxy and its features (sort menu, tagging, etc.)
  */

#include "game/proxy/chunnelproxy.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/chunnelmission.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/ref/list.hpp"
#include "game/ref/nullpredicate.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "util/math.hpp"

using game::proxy::ChunnelProxy;
using game::Game;
using game::Id_t;
using game::Root;
using game::config::HostConfiguration;
using game::map::Ship;
using game::map::Point;
using game::map::Universe;
using game::spec::ShipList;

namespace {

    /** Add a candidate location. Sorted insert. */
    void addCandidateLocation(ChunnelProxy::CandidateList& result, const ChunnelProxy::Candidate& cand)
    {
        std::vector<ChunnelProxy::Candidate>::iterator
            pos = result.candidates.begin(),
            end = result.candidates.end();

        while (pos != end && cand < *pos) {
            ++pos;
        }

        if (pos == end || *pos != cand) {
            result.candidates.insert(pos, cand);
        }
    }

    /* Build list of candidate locations. */
    void buildCandidateLocationList(ChunnelProxy::CandidateList& result, game::Session& session, const Id_t shipId)
    {
        Game& g = game::actions::mustHaveGame(session);
        const Root& r = game::actions::mustHaveRoot(session);
        const ShipList& sl = game::actions::mustHaveShipList(session);

        // Minimum distance
        // @change PCC2 hardcodes a "isPHost" check here. This is not required because the default is 100, which is correct for THost as well.
        result.minDistance = r.hostConfiguration()[HostConfiguration::MinimumChunnelDistance]();

        // Possible targets
        Universe& univ = g.currentTurn().universe();
        if (const Ship* initiator = univ.ships().get(shipId)) {
            const game::map::AnyShipType ty(univ.ships());
            for (Id_t id = ty.findNextIndex(0); id != 0; id = ty.findNextIndex(id)) {
                if (const Ship* mate = univ.ships().get(id)) {
                    if (game::map::isValidChunnelMate(*initiator, *mate, univ.config(), r, g.shipScores(), sl)) {
                        Point initPos, matePos;
                        initiator->getPosition(initPos);
                        mate->getPosition(matePos);
                        addCandidateLocation(result, ChunnelProxy::Candidate(univ.config().getSimpleNearestAlias(matePos, initPos)));
                    }
                }
            }
        }
    }

    /* Build list of candidates at location. */
    void buildCandidateList(game::ref::UserList& result, const Id_t shipId, const Point pos, game::Session& session)
    {
        Game& g = game::actions::mustHaveGame(session);
        const Root& r = game::actions::mustHaveRoot(session);
        const ShipList& sl = game::actions::mustHaveShipList(session);

        // Build raw list
        game::ref::List list;
        Universe& univ = g.currentTurn().universe();
        const Point canonicalPosition = univ.config().getCanonicalLocation(pos);
        if (const Ship* initiator = univ.ships().get(shipId)) {
            const game::map::AnyShipType ty(univ.ships());
            for (Id_t id = ty.findNextIndex(0); id != 0; id = ty.findNextIndex(id)) {
                const Ship* mate = univ.ships().get(id);
                Point matePos;
                if (mate != 0 && mate->getPosition(matePos) && matePos == canonicalPosition && game::map::isValidChunnelMate(*initiator, *mate, univ.config(), r, g.shipScores(), sl)) {
                    list.add(game::Reference(game::Reference::Ship, id));
                }
            }
        }

        // Convert list
        result.add(list, session, game::ref::NullPredicate(), game::ref::NullPredicate());
    }

    afl::data::StringList_t setupChunnel(game::Session& session, const Id_t fromShipId, const Id_t toShipId)
    {
        Game& g = game::actions::mustHaveGame(session);
        const Root& r = game::actions::mustHaveRoot(session);
        const ShipList& sl = game::actions::mustHaveShipList(session);

        Universe& univ = g.currentTurn().universe();
        Ship& initiator = game::actions::mustExist(univ.ships().get(fromShipId));
        Ship& mate = game::actions::mustExist(univ.ships().get(toShipId));

        game::map::setupChunnel(initiator, mate, univ, r.hostConfiguration(), sl);

        // Check whether chunnel works
        game::map::ChunnelMission msn;
        if (msn.check(initiator, univ, g.shipScores(), sl, r)) {
            return game::map::formatChunnelFailureReasons(msn.getFailureReasons(), session.translator());
        } else {
            afl::data::StringList_t result;
            result.push_back(session.translator()("Chunnel is impossible"));
            return result;
        }
    }

}

/*
 *  Candidate operators
 */

bool
game::proxy::ChunnelProxy::Candidate::operator==(const Candidate& b) const
{
    return pos == b.pos;
}

bool
game::proxy::ChunnelProxy::Candidate::operator!=(const Candidate& b) const
{
    return !operator==(b);
}

bool
game::proxy::ChunnelProxy::Candidate::operator<(const Candidate& b) const
{
    int dy = b.pos.getY() - pos.getY();
    if (dy != 0) {
        return dy < 0;
    }

    return b.pos.getX() < pos.getX();
}

/*
 *  CandidateList operators
 */

bool
game::proxy::ChunnelProxy::CandidateList::operator==(const CandidateList& b) const
{
    return minDistance == b.minDistance
        && candidates == b.candidates;
}

bool
game::proxy::ChunnelProxy::CandidateList::operator!=(const CandidateList& b) const
{
    return !operator==(b);
}

/*
 *  ChunnelProxy
 */

// Constructor.
game::proxy::ChunnelProxy::ChunnelProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_gameSender(gameSender),
      m_reply(reply, *this)
{ }

// Destructor.
game::proxy::ChunnelProxy::~ChunnelProxy()
{ }

// Asynchronous request for possible chunnel targets.
void
game::proxy::ChunnelProxy::postCandidateRequest(Id_t shipId)
{
    /* Reply (Session -> UI) */
    class Reply : public util::Request<ChunnelProxy> {
     public:
        Reply(Session& session, Id_t shipId)
            : m_result()
            { buildCandidateLocationList(m_result, session, shipId); }

        void handle(ChunnelProxy& proxy)
            { proxy.sig_candidateListUpdate.raise(m_result); }

     private:
        CandidateList m_result;
    };

    /* Query (UI -> Session) */
    class Query : public util::Request<Session> {
     public:
        Query(Id_t shipId, util::RequestSender<ChunnelProxy> reply)
            : m_shipId(shipId), m_reply(reply)
            { }

        void handle(Session& session)
            { m_reply.postNewRequest(new Reply(session, m_shipId)); }

     private:
        Id_t m_shipId;
        util::RequestSender<ChunnelProxy> m_reply;
    };

    m_gameSender.postNewRequest(new Query(shipId, m_reply.getSender()));
}

// Synchronous request for possible chunnel targets.
void
game::proxy::ChunnelProxy::getCandidates(WaitIndicator& link, Id_t shipId, game::map::Point pos, game::ref::UserList& list)
{
    class Query : public util::Request<Session> {
     public:
        Query(Id_t shipId, Point pos, game::ref::UserList& list)
            : m_shipId(shipId), m_pos(pos), m_result(list)
            { }

        void handle(Session& session)
            { buildCandidateList(m_result, m_shipId, m_pos, session); }

     private:
        Id_t m_shipId;
        Point m_pos;
        game::ref::UserList& m_result;
    };

    Query q(shipId, pos, list);
    link.call(m_gameSender, q);
}

// Synchronous request to set up a chunnel.
afl::data::StringList_t
game::proxy::ChunnelProxy::setupChunnel(WaitIndicator& link, Id_t fromShipId, Id_t toShipId)
{
    class Query : public util::Request<Session> {
     public:
        Query(Id_t fromShipId, Id_t toShipId, afl::data::StringList_t& result)
            : m_fromShipId(fromShipId), m_toShipId(toShipId), m_result(result)
            { }

        void handle(Session& session)
            { m_result = ::setupChunnel(session, m_fromShipId, m_toShipId); }

     private:
        Id_t m_fromShipId;
        Id_t m_toShipId;
        afl::data::StringList_t& m_result;
    };

    afl::data::StringList_t result;
    Query q(fromShipId, toShipId, result);
    link.call(m_gameSender, q);

    return result;
}
