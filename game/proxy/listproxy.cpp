/**
  *  \file game/proxy/listproxy.cpp
  *  \brief Class game::proxy::ListProxy
  */

#include "game/proxy/listproxy.hpp"
#include "afl/data/integervalue.hpp"
#include "game/game.hpp"
#include "game/map/movementpredictor.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/turn.hpp"

using afl::data::IntegerValue;
using afl::data::Value;
using game::Element;
using game::config::HostConfiguration;
using game::map::Object;
using game::map::Planet;
using game::map::Point;
using game::map::Ship;
using game::map::Universe;
using game::ref::List;
using game::spec::Cost;
using game::spec::CostSummary;
using game::spec::ShipList;

namespace {
    bool hasRemoteControl(const game::Root& r)
    {
        return r.hostConfiguration()[HostConfiguration::CPEnableRemote]();
    }

    void buildCurrentCargoSummary(game::Session& session, const List& in, CostSummary& out)
    {
        if (game::Game* pGame = session.getGame().get()) {
            Universe& univ = pGame->viewpointTurn().universe();
            for (size_t i = 0, n = in.size(); i < n; ++i) {
                if (const Ship* ship = dynamic_cast<const game::map::Ship*>(univ.getObject(in[i]))) {
                    if (ship->isPlayable(Object::ReadOnly)) {
                        Cost cargo;
                        cargo.set(Cost::Tritanium,  ship->getCargo(Element::Tritanium).orElse(0));
                        cargo.set(Cost::Duranium,   ship->getCargo(Element::Duranium).orElse(0));
                        cargo.set(Cost::Molybdenum, ship->getCargo(Element::Molybdenum).orElse(0));
                        cargo.set(Cost::Supplies,   ship->getCargo(Element::Supplies).orElse(0));
                        cargo.set(Cost::Money,      ship->getCargo(Element::Money).orElse(0));
                        out.add(CostSummary::Item(ship->getId(), 1, ship->getName(game::LongName, session.translator(), session.interface()), cargo));
                    }
                }
            }
        }
    }

    void buildNextCargoSummary(game::Session& session, const List& in, CostSummary& out)
    {
        game::Root* root = session.getRoot().get();
        ShipList* list = session.getShipList().get();
        game::Game* g = session.getGame().get();
        if (root != 0 && list != 0 && g != 0) {
            // Compute movement
            const Universe& univ = g->viewpointTurn().universe();
            game::map::MovementPredictor pred;
            pred.computeMovement(univ, *g, *list, *root);

            // Build list
            for (size_t i = 0, n = in.size(); i < n; ++i) {
                if (const Ship* ship = dynamic_cast<const Ship*>(univ.getObject(in[i]))) {
                    Cost cargo;
                    if (pred.getShipCargo(ship->getId(), cargo)) {
                        out.add(CostSummary::Item(ship->getId(), 1, ship->getName(game::LongName, session.translator(), session.interface()), cargo));
                    }
                }
            }
        }
    }

    enum Flavor {
        Current,
        Next
    };
}

struct game::proxy::ListProxy::State {
    Flavor flavor : 8;
    bool isUniquePlayable;
    bool hasRemoteControl;
    bool hasExcludedShip;
    bool hasHidingPlanet;
    List list;
    String_t hidingPlanetName;

    State(Flavor flavor)
        : flavor(flavor), isUniquePlayable(), hasRemoteControl(), hasExcludedShip(), hasHidingPlanet(), list(), hidingPlanetName()
        { }
};

game::proxy::ListProxy::ListProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender),
      m_state(new State(Current))
{ }

game::proxy::ListProxy::~ListProxy()
{ }

void
game::proxy::ListProxy::buildCurrent(WaitIndicator& ind, game::map::Point pos, game::ref::List::Options_t options, Id_t excludeShip)
{
    class ListBuilder : public util::Request<Session> {
     public:
        ListBuilder(State& state, Point pos, List::Options_t options, Id_t& excludeShip)
            : m_state(state), m_pos(pos), m_options(options), m_excludeShip(excludeShip)
            { }

        virtual void handle(Session& session)
            {
                if (Game* g = session.getGame().get()) {
                    const Universe& univ = g->viewpointTurn().universe();
                    m_state.list.addObjectsAt(univ, g->mapConfiguration().getCanonicalLocation(m_pos), m_options, m_excludeShip);

                    // Verify that the ship to be excluded is actually eligible.
                    // This is needed to pick the correct error message.
                    if (const Ship* pShip = univ.ships().get(m_excludeShip)) {
                        Point excludePos;
                        m_state.hasExcludedShip = (pShip->getPosition().get(excludePos) && excludePos == m_pos);
                    }

                    // Remember planet
                    if (const Planet* pPlanet = univ.planets().get(univ.findPlanetAt(g->mapConfiguration().getCanonicalLocation(m_pos)))) {
                        if (!pPlanet->isPlayable(Object::Playable)) {
                            m_state.hasHidingPlanet = true;
                            m_state.hidingPlanetName = pPlanet->getName(session.translator());
                        }
                    }
                }

                setCommon(session, m_state);
            }

     private:
        State& m_state;
        const Point m_pos;
        const List::Options_t m_options;
        const Id_t m_excludeShip;
    };

    std::auto_ptr<State> newState(new State(Current));
    ListBuilder b(*newState, pos, options, excludeShip);
    ind.call(m_gameSender, b);
    m_state = newState;
}

void
game::proxy::ListProxy::buildNext(WaitIndicator& ind, game::map::Point pos, Id_t fromShip, game::ref::List::Options_t options)
{
    class NextBuilder : public util::Request<game::Session> {
     public:
        NextBuilder(State& st, Point pos, Id_t fromShip, List::Options_t options)
            : m_state(st), m_pos(pos), m_fromShip(fromShip), m_options(options)
            { }

        virtual void handle(game::Session& session)
            {
                // ex doListNextShips (part)
                Root* root = session.getRoot().get();
                ShipList* list = session.getShipList().get();
                Game* g = session.getGame().get();
                if (root != 0 && list != 0 && g != 0) {
                    // Compute movement
                    const Universe& univ = g->viewpointTurn().universe();
                    game::map::MovementPredictor pred;
                    pred.computeMovement(univ, *g, *list, *root);

                    // If looking at a ship, resolve its position
                    bool posOK;
                    Point pos;
                    if (m_fromShip != 0) {
                        posOK = pred.getShipPosition(m_fromShip).get(pos);
                    } else {
                        posOK = true;
                        pos = m_pos;
                    }

                    // Build list
                    if (posOK) {
                        pos = g->mapConfiguration().getCanonicalLocation(pos);

                        const game::map::AnyShipType& ty(univ.allShips());
                        for (Id_t id = ty.findNextIndex(0); id != 0; id = ty.findNextIndex(id)) {
                            const Ship* sh = univ.ships().get(id);
                            Point shPos;

                            if (sh != 0
                                && pred.getShipPosition(id).get(shPos)
                                && g->mapConfiguration().getCanonicalLocation(shPos) == pos
                                && (m_options.contains(List::IncludeForeignShips) || sh->isPlayable(Object::ReadOnly))
                                && (!m_options.contains(List::SafeShipsOnly) || sh->isReliablyVisible(0)))
                            {
                                m_state.list.add(Reference(Reference::Ship, id));
                            }
                        }

                        // If list is not empty, AND we're coming from a ship, place scanner.
                        // (Otherwise, we're likely coming from a context where the scanner is already at the correct place.)
                        if (m_fromShip != 0) {
                            try {
                                game::interface::UserInterfacePropertyStack& uiProps = session.uiPropertyStack();

                                // Consider current position to place cursor correctly across wrap
                                std::auto_ptr<Value> chartX(uiProps.get(game::interface::iuiChartX));
                                std::auto_ptr<Value> chartY(uiProps.get(game::interface::iuiChartY));
                                const IntegerValue* chartXIV = dynamic_cast<const IntegerValue*>(chartX.get());
                                const IntegerValue* chartYIV = dynamic_cast<const IntegerValue*>(chartY.get());

                                if (chartXIV != 0 && chartYIV != 0) {
                                    Point adjPos = g->mapConfiguration().getSimpleNearestAlias(pos, Point(chartXIV->getValue(), chartYIV->getValue()));
                                    IntegerValue xv(adjPos.getX());
                                    IntegerValue yv(adjPos.getY());
                                    uiProps.set(game::interface::iuiScanX, &xv);
                                    uiProps.set(game::interface::iuiScanY, &yv);
                                }
                            }
                            catch (...) {
                                // set() may fail; don't deprive user of this functionality then
                            }
                        }
                    }
                }

                setCommon(session, m_state);
            }

     private:
        State& m_state;
        const Point m_pos;
        const Id_t m_fromShip;
        const List::Options_t m_options;
    };

    std::auto_ptr<State> newState(new State(Next));
    NextBuilder b(*newState, pos, fromShip, options);
    ind.call(m_gameSender, b);
    m_state = newState;
}

game::spec::CostSummary
game::proxy::ListProxy::getCargoSummary(WaitIndicator& ind)
{
    class Task : public util::Request<Session> {
     public:
        Task(State& st, CostSummary& result)
            : m_state(st), m_result(result)
            { }

        virtual void handle(Session& session)
            {
                switch (m_state.flavor) {
                 case Current:
                    buildCurrentCargoSummary(session, m_state.list, m_result);
                    break;
                 case Next:
                    buildNextCargoSummary(session, m_state.list, m_result);
                    break;
                }
            }
     private:
        State& m_state;
        CostSummary& m_result;
    };

    CostSummary result;
    Task t(*m_state, result);
    ind.call(m_gameSender, t);
    return result;
}

const game::ref::List
game::proxy::ListProxy::getList() const
{
    return m_state->list;
}

bool
game::proxy::ListProxy::isCurrent() const
{
    return m_state->flavor == Current;
}

bool
game::proxy::ListProxy::isUniquePlayable() const
{
    return m_state->isUniquePlayable;
}

bool
game::proxy::ListProxy::hasRemoteControl() const
{
    return m_state->hasRemoteControl;
}

bool
game::proxy::ListProxy::hasExcludedShip() const
{
    return m_state->hasExcludedShip;
}

bool
game::proxy::ListProxy::hasHidingPlanet() const
{
    return m_state->hasHidingPlanet;
}

const String_t&
game::proxy::ListProxy::getHidingPlanetName() const
{
    return m_state->hidingPlanetName;
}

void
game::proxy::ListProxy::setCommon(Session& session, State& st)
{
    // Check unique playability
    if (st.list.size() == 1) {
        if (Game* g = session.getGame().get()) {
            if (const Object* pObj = g->viewpointTurn().universe().getObject(st.list[0])) {
                st.isUniquePlayable = (pObj->isPlayable(Object::ReadOnly));
            }
        }
    }

    // Check remote control
    if (Root* r = session.getRoot().get()) {
        st.hasRemoteControl = ::hasRemoteControl(*r);
    }
}
