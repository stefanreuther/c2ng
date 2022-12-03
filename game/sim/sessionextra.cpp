/**
  *  \file game/sim/sessionextra.cpp
  *  \brief Game/Simulator Session
  */

#include "game/sim/sessionextra.hpp"
#include "game/extra.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/transfer.hpp"
#include "game/turn.hpp"

using afl::base::Ref;
using game::map::Universe;
using game::spec::ShipList;

namespace game { namespace sim { namespace {

    /*
     *  Session Extra
     *
     *  The simulator session is stored as a Ref to allow eventually using it a script object.
     */

    struct SessionExtra : public Extra {
        SessionExtra(Ref<Session> p)
            : simSession(p)
            { }

        Ref<Session> simSession;
    };


    /*
     *  Implementation of GameInterface to connect a game::sim::Session with a game::Session
     */

    class GameInterfaceImpl : public GameInterface {
     public:
        GameInterfaceImpl(game::Session& session)
            : m_session(session)
            { }

        virtual bool hasGame() const
            {
                return m_session.getGame().get() != 0;
            }

        virtual bool hasShip(Id_t shipId) const
            {
                const Game* g = m_session.getGame().get();
                if (g != 0) {
                    const game::map::Ship* sh = g->currentTurn().universe().ships().get(shipId);
                    return sh != 0 && sh->isVisible();
                }
                return false;
            }

        virtual String_t getPlanetName(Id_t id) const
            {
                // ex GSimulatorRealGameInterface::getPlanetName
                const Game* g = m_session.getGame().get();
                if (g) {
                    const game::map::Planet* pl = g->currentTurn().universe().planets().get(id);
                    if (pl) {
                        return pl->getName(m_session.translator());
                    }
                }
                return String_t();
            }

        virtual Id_t getMaxPlanetId() const
            {
                const Game* g = m_session.getGame().get();
                if (g != 0) {
                    return g->currentTurn().universe().planets().size();
                } else {
                    return 0;
                }
            }

        virtual int getShipOwner(Id_t id) const
            {
                // ex GSimulatorRealGameInterface::getShipOwner
                if (const Game* g = m_session.getGame().get()) {
                    if (const game::map::Ship* sh = g->currentTurn().universe().ships().get(id)) {
                        return sh->getOwner().orElse(0);
                    }
                }
                return 0;
            }

        virtual Id_t getMaxShipId() const
            {
                const Game* g = m_session.getGame().get();
                if (g != 0) {
                    return g->currentTurn().universe().ships().size();
                } else {
                    return 0;
                }
            }

        virtual bool copyShipFromGame(Ship& out) const
            {
                // ex GSimulatorRealGameInterface::updateFromGame
                Game* g = m_session.getGame().get();
                Root* r = m_session.getRoot().get();
                ShipList* sl = m_session.getShipList().get();
                if (g != 0 && r != 0 && sl != 0) {
                    game::map::Ship* sh = g->currentTurn().universe().ships().get(out.getId());
                    if (sh != 0) {
                        return Transfer(g->shipScores(), g->planetScores(), *sl, r->hostConfiguration(), r->hostVersion(), m_session.translator())
                            .copyShipFromGame(out, *sh);
                    }
                }
                return false;
            }

        virtual bool copyShipToGame(const Ship& in)
            {
                Game* g = m_session.getGame().get();
                Root* r = m_session.getRoot().get();
                ShipList* sl = m_session.getShipList().get();
                if (g != 0 && r != 0 && sl != 0) {
                    Universe& univ = g->currentTurn().universe();
                    game::map::Ship* sh = univ.ships().get(in.getId());
                    if (sh != 0) {
                        return Transfer(g->shipScores(), g->planetScores(), *sl, r->hostConfiguration(), r->hostVersion(), m_session.translator())
                            .copyShipToGame(*sh, in, univ, g->mapConfiguration());
                    }
                }
                return false;
            }

        virtual Relation getShipRelation(const Ship& in) const
            {
                // ex GSimulatorRealGameInterface::getShipRelation
                Game* g = m_session.getGame().get();
                Root* r = m_session.getRoot().get();
                ShipList* sl = m_session.getShipList().get();
                if (g != 0 && r != 0 && sl != 0) {
                    game::map::Ship* sh = g->currentTurn().universe().ships().get(in.getId());
                    if (sh != 0) {
                        int owner = 0;
                        int hull = 0;
                        if (!sh->getRealOwner().get(owner) || owner != in.getOwner()) {
                            // Exists but with wrong owner: universe ship is different
                            return Unknown;
                        } else if (!sh->getHull().get(hull) || hull != in.getHullType()) {
                            // Exists but with wrong type: universe ship is different
                            return Unknown;
                        } else if (sh->isPlayable(game::map::Object::Playable)) {
                            // Playable
                            return Playable;
                        } else if (sh->isVisible()) {
                            // Foreign
                            return ReadOnly;
                        } else {
                            // Nonexistant/invisible (history) ship
                            return Unknown;
                        }
                    }
                }
                return Unknown;
            }

        virtual bool copyPlanetFromGame(Planet& out) const
            {
                // ex GSimulatorRealGameInterface::updateFromGame
                Game* g = m_session.getGame().get();
                Root* r = m_session.getRoot().get();
                ShipList* sl = m_session.getShipList().get();
                if (g != 0 && r != 0 && sl != 0) {
                    game::map::Planet* pl = g->currentTurn().universe().planets().get(out.getId());
                    if (pl != 0) {
                        return Transfer(g->shipScores(), g->planetScores(), *sl, r->hostConfiguration(), r->hostVersion(), m_session.translator())
                            .copyPlanetFromGame(out, *pl);
                    }
                }
                return false;
            }

        virtual bool copyPlanetToGame(const Planet& out)
            {
                // ex GSimulatorRealGameInterface::updateToGame (was not implemented)
                Game* g = m_session.getGame().get();
                Root* r = m_session.getRoot().get();
                ShipList* sl = m_session.getShipList().get();
                if (g != 0 && r != 0 && sl != 0) {
                    game::map::Planet* pl = g->currentTurn().universe().planets().get(out.getId());
                    if (pl != 0) {
                        return Transfer(g->shipScores(), g->planetScores(), *sl, r->hostConfiguration(), r->hostVersion(), m_session.translator())
                            .copyPlanetToGame(*pl, out);
                    }
                }
                return false;
            }

        virtual Relation getPlanetRelation(const Planet& in) const
            {
                // ex GSimulatorRealGameInterface::isGamePlanet
                Game* g = m_session.getGame().get();
                if (g != 0) {
                    const game::map::Planet* pl = g->currentTurn().universe().planets().get(in.getId());
                    if (pl != 0) {
                        if (pl->isPlayable(game::map::Planet::Playable)) {
                            return Playable;
                        } else if (pl->hasAnyPlanetData()) {
                            return ReadOnly;
                        } else {
                            return Unknown;
                        }
                    }
                }
                return Unknown;
            }

     private:
        game::Session& m_session;
    };

    ExtraIdentifier<game::Session, SessionExtra> SIM_ID;

} } }

afl::base::Ref<game::sim::Session>
game::sim::getSimulatorSession(game::Session& session)
{
    SessionExtra* px = session.extra().get(SIM_ID);
    if (px == 0) {
        px = session.extra().setNew(SIM_ID, new SessionExtra(*new Session()));
        px->simSession->setNewGameInterface(new GameInterfaceImpl(session));
    }
    return px->simSession;
}
