/**
  *  \file game/sim/sessionextra.cpp
  *  \brief Game/Simulator Session
  */

#include "game/sim/sessionextra.hpp"
#include "game/alliance/level.hpp"
#include "game/alliance/offer.hpp"
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
using game::alliance::Level;
using game::alliance::Offer;
using game::map::Universe;
using game::spec::ShipList;

namespace {
    bool isActiveOffer(Offer::Type offer, Offer::Type recipient)
    {
        return offer == Offer::Yes
            || (offer == Offer::Conditional && Offer::isOffer(recipient));
    }
}

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
                    const game::map::Ship* sh = g->viewpointTurn().universe().ships().get(shipId);
                    return sh != 0 && sh->isVisible();
                }
                return false;
            }

        virtual String_t getPlanetName(Id_t id) const
            {
                // ex GSimulatorRealGameInterface::getPlanetName
                const Game* g = m_session.getGame().get();
                if (g) {
                    const game::map::Planet* pl = g->viewpointTurn().universe().planets().get(id);
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
                    return g->viewpointTurn().universe().planets().size();
                } else {
                    return 0;
                }
            }

        virtual int getShipOwner(Id_t id) const
            {
                // ex GSimulatorRealGameInterface::getShipOwner
                if (const Game* g = m_session.getGame().get()) {
                    if (const game::map::Ship* sh = g->viewpointTurn().universe().ships().get(id)) {
                        return sh->getOwner().orElse(0);
                    }
                }
                return 0;
            }

        virtual Id_t getMaxShipId() const
            {
                const Game* g = m_session.getGame().get();
                if (g != 0) {
                    return g->viewpointTurn().universe().ships().size();
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
                    game::map::Ship* sh = g->viewpointTurn().universe().ships().get(out.getId());
                    if (sh != 0) {
                        return Transfer(g->shipScores(), g->planetScores(), *sl, r->hostConfiguration(), m_session.translator())
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
                    Universe& univ = g->viewpointTurn().universe();
                    game::map::Ship* sh = univ.ships().get(in.getId());
                    if (sh != 0) {
                        return Transfer(g->shipScores(), g->planetScores(), *sl, r->hostConfiguration(), m_session.translator())
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
                    game::map::Ship* sh = g->viewpointTurn().universe().ships().get(in.getId());
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

        virtual afl::base::Optional<game::map::Point> getShipPosition(const Ship& in) const
            {
                Game* g = m_session.getGame().get();
                if (g != 0) {
                    const game::map::Ship* sh = g->viewpointTurn().universe().ships().get(in.getId());
                    if (sh != 0) {
                        return sh->getPosition();
                    }
                }
                return afl::base::Nothing;
            }

        virtual bool copyPlanetFromGame(Planet& out) const
            {
                // ex GSimulatorRealGameInterface::updateFromGame
                Game* g = m_session.getGame().get();
                Root* r = m_session.getRoot().get();
                ShipList* sl = m_session.getShipList().get();
                if (g != 0 && r != 0 && sl != 0) {
                    game::map::Planet* pl = g->viewpointTurn().universe().planets().get(out.getId());
                    if (pl != 0) {
                        return Transfer(g->shipScores(), g->planetScores(), *sl, r->hostConfiguration(), m_session.translator())
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
                    game::map::Planet* pl = g->viewpointTurn().universe().planets().get(out.getId());
                    if (pl != 0) {
                        return Transfer(g->shipScores(), g->planetScores(), *sl, r->hostConfiguration(), m_session.translator())
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
                    const game::map::Planet* pl = g->viewpointTurn().universe().planets().get(in.getId());
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

        virtual afl::base::Optional<game::map::Point> getPlanetPosition(const Planet& in) const
            {
                Game* g = m_session.getGame().get();
                if (g != 0) {
                    const game::map::Planet* pl = g->viewpointTurn().universe().planets().get(in.getId());
                    if (pl != 0) {
                        return pl->getPosition();
                    }
                }
                return afl::base::Nothing;
            }

        virtual void getPlayerRelations(PlayerBitMatrix& alliances, PlayerBitMatrix& enemies) const
            {
                // GSimulatorRealGameInterface::setDefaultRelations
                alliances.clear();
                enemies.clear();

                Game* g = m_session.getGame().get();
                if (g != 0) {
                    // Lo-fi defaults from teams
                    const TeamSettings& teams = g->teamSettings();
                    for (int a = 1; a <= MAX_PLAYERS; ++a) {
                        for (int b = 1; b <= MAX_PLAYERS; ++b) {
                            if (a != b && teams.getPlayerTeam(a) != 0 && teams.getPlayerTeam(a) == teams.getPlayerTeam(b)) {
                                alliances.set(a, b, true);
                            }
                        }
                    }

                    // liveAllies is not necessarily in sync with command messages; update it.
                    game::alliance::Container& liveAllies = g->viewpointTurn().alliances();
                    liveAllies.postprocess();

                    const game::alliance::Levels_t& levels = liveAllies.getLevels();
                    const int me = g->getViewpointPlayer();
                    for (size_t i = 0, n = levels.size(); i < n; ++i) {
                        if (levels[i].hasFlag(Level::IsCombat)) {
                            // It's the combat level.
                            // Do NOT validate the NeedsOffer/IsOffer relationship here
                            // assuming that a possible alliance is completed.
                            const Offer* offer = liveAllies.getOffer(i);
                            assert(offer != 0);
                            for (int playerNr = 1; playerNr <= MAX_PLAYERS; ++playerNr) {
                                if (isActiveOffer(offer->theirOffer.get(playerNr), offer->newOffer.get(playerNr))) {
                                    // Player offers to us
                                    alliances.set(playerNr, me, true);
                                }
                                if (isActiveOffer(offer->newOffer.get(playerNr), offer->theirOffer.get(playerNr))) {
                                    // We offer to player
                                    alliances.set(me, playerNr, true);
                                }
                            }
                        }
                        if (levels[i].hasFlag(Level::IsEnemy)) {
                            // It's the persistent enemy order
                            const Offer* offer = liveAllies.getOffer(i);
                            assert(offer != 0);
                            for (int playerNr = 1; playerNr <= MAX_PLAYERS; ++playerNr) {
                                if (Offer::isOffer(offer->theirOffer.get(playerNr))) {
                                    // Player attacks us
                                    enemies.set(playerNr, me, true);
                                }
                                if (Offer::isOffer(offer->newOffer.get(playerNr))) {
                                    // We attack them
                                    enemies.set(me, playerNr, true);
                                }
                            }
                        }
                    }
                }
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

void
game::sim::initSimulatorSession(game::Session& session)
{
    // ex initSimulatorSession(), GSimOptions::loadDefaults() (part)
    const Game* g = session.getGame().get();
    const Root* r = session.getRoot().get();
    if (g != 0 && r != 0) {
        Session& simSession = *getSimulatorSession(session);
        simSession.configuration() = Configuration();
        simSession.configuration().setModeFromHostVersion(r->hostVersion(), g->getViewpointPlayer(), r->hostConfiguration());
    }
}
