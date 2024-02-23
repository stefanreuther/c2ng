/**
  *  \file game/game.cpp
  *  \brief Class game::Game
  */

#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/alliance/container.hpp"

game::Game::Game()
    : sig_viewpointTurnChange(),
      m_currentTurn(*new Turn()),
      m_planetScores(),
      m_shipScores(),
      m_teamSettings(),
      m_viewpointTurnNumber(0),
      m_scores(),
      m_cursors(),
      m_selections(),
      m_mapConfiguration(),
      m_messageConfiguration(),
      m_expressionLists()
{
    m_cursors.setUniverse(&m_currentTurn->universe(), &m_mapConfiguration);
}

game::Game::~Game()
{ }

game::Turn&
game::Game::viewpointTurn()
{
    if (m_viewpointTurnNumber != 0 && m_viewpointTurnNumber != currentTurn().getTurnNumber()) {
        if (HistoryTurn* ht = m_previousTurns.get(m_viewpointTurnNumber)) {
            afl::base::Ptr<Turn> t = ht->getTurn();
            if (t.get() != 0) {
                return *t;
            }
        }
    }

    // setViewpointTurnNumber will have made sure that we only end up here if
    // m_viewpointTurnNumber actually points at the current turn.
    // Otherwise, this is a fail-safe.
    return currentTurn();
}

const game::Turn&
game::Game::viewpointTurn() const
{
    return const_cast<Game&>(*this).viewpointTurn();
}

int
game::Game::getViewpointTurnNumber() const
{
    if (m_viewpointTurnNumber == 0) {
        return currentTurn().getTurnNumber();
    } else {
        return m_viewpointTurnNumber;
    }
}

void
game::Game::setViewpointTurnNumber(int nr)
{
    // Validate
    bool ok;
    if (nr == 0 || nr == currentTurn().getTurnNumber()) {
        ok = true;
    } else if (HistoryTurn* ht = m_previousTurns.get(nr)) {
        ok = (ht->getTurn().get() != 0);
    } else {
        ok = false;
    }

    if (ok) {
        // Change turn number
        Turn& oldTurn = viewpointTurn();
        m_viewpointTurnNumber = nr;
        Turn& newTurn = viewpointTurn();

        // Update
        if (&oldTurn != &newTurn) {
            // Transfer selection to new turn
            // FIXME: the limitToExistingObjects() will unmark objects that don't exist in the new turn.
            // It would be nice if we could avoid that.
            // However, the copyFrom() will already unmark nonexistant objects,
            // effectively doing the equivalent of limitToExistingObjects().
            // Until we can somehow avoid that, keep the limitToExistingObjects().
            m_selections.copyFrom(oldTurn.universe(), m_selections.getCurrentLayer());

            m_selections.copyTo(newTurn.universe(), m_selections.getCurrentLayer());
            m_selections.limitToExistingObjects(newTurn.universe(), m_selections.getCurrentLayer());

            // Change cursor
            m_cursors.setUniverse(&newTurn.universe(), &m_mapConfiguration);
            sig_viewpointTurnChange.raise();

            // We may have updated selection totals, e.g. objects not existing in new turn
            m_selections.sig_selectionChange.raise();
        }
    }
}

void
game::Game::addMessageInformation(const game::parser::MessageInformation& info,
                                  game::config::HostConfiguration& config,
                                  HostVersion host,
                                  util::AtomTable& atomTable,
                                  afl::base::Optional<size_t> msgNr,
                                  bool isLoading,
                                  afl::string::Translator& tx,
                                  afl::sys::LogListener& log)
{
    // ex GUniverse::addMessageInformation
    using game::parser::MessageInformation;

    // Do not accept information that claims to be newer than us
    if (info.getTurnNumber() > currentTurn().getTurnNumber()) {
        return;
    }

    // Dispatch
    switch (info.getObjectType()) {
     case MessageInformation::Ship:
     case MessageInformation::ExtraShip:
        // Ship: add normally, with no claim to reliability (=empty source set).
        // To add information to be treated as reliable (e.g. target.dat file), add it to the ship directly.
        if (game::map::Ship* pShip = currentTurn().universe().ships().get(info.getObjectId())) {
            pShip->addMessageInformation(info, PlayerSet_t());
            if (const size_t* pNr = msgNr.get()) {
                pShip->messages().add(*pNr);
            }
            if (!isLoading) {
                pShip->internalCheck(currentTurn().universe().getAvailablePlayers(), currentTurn().getTurnNumber());
            }
        }
        break;

     case MessageInformation::Planet:
     case MessageInformation::Starbase:
     case MessageInformation::ExtraPlanet:
        // Planet: add normally
        if (game::map::Planet* pPlanet = currentTurn().universe().planets().get(info.getObjectId())) {
            pPlanet->addMessageInformation(info);
            if (const size_t* pNr = msgNr.get()) {
                pPlanet->messages().add(*pNr);
            }
            if (!isLoading) {
                pPlanet->internalCheck(mapConfiguration(), currentTurn().universe().getAvailablePlayers(), currentTurn().getTurnNumber(), tx, log);
            }
        }
        break;

     case MessageInformation::Minefield:
     case MessageInformation::ExtraMinefield:
        // Minefield: add normally. MinefieldType will deal with details.
        currentTurn().universe().minefields().addMessageInformation(info);
        currentTurn().universe().minefields().internalCheck(currentTurn().getTurnNumber(), host, config);
        break;

     case MessageInformation::IonStorm:
        // Ion storm: only add current turn's data; last turn's weather forecast is worthless
        if (info.getTurnNumber() == currentTurn().getTurnNumber()) {
            if (game::map::IonStorm* pStorm = currentTurn().universe().ionStorms().get(info.getObjectId())) {
                pStorm->addMessageInformation(info);
            }
        }
        break;

     case MessageInformation::Ufo:
     case MessageInformation::Wormhole:
        // Ufo, Wormhole: add normally. UfoType will deal with details.
        currentTurn().universe().ufos().addMessageInformation(info);
        break;

     case MessageInformation::Explosion:
        // Explosion: only add current turn's data; ExplosionType deals with details.
        if (info.getTurnNumber() == currentTurn().getTurnNumber()) {
            currentTurn().universe().explosions().addMessageInformation(info);
        }
        break;

     case MessageInformation::Configuration:
        // Configuration: add it. Ignore the age here.
        for (MessageInformation::Iterator_t i = info.begin(); i != info.end(); ++i) {
            if (game::parser::MessageConfigurationValue_t* cv = dynamic_cast<game::parser::MessageConfigurationValue_t*>(*i)) {
                try {
                    config.setOption(cv->getIndex(),
                                     cv->getValue(),
                                     game::config::ConfigurationOption::Game);
                }
                catch (...) {
                    // Ignore any error.
                }
            }
        }
        break;

     case MessageInformation::PlayerScore:
        // Score: we can add past scores only if we already know its timestamp.
        if (info.getTurnNumber() == currentTurn().getTurnNumber()) {
            scores().addMessageInformation(info, currentTurn().getTimestamp());
        } else if (const game::score::TurnScore* ts = scores().getTurn(info.getTurnNumber())) {
            scores().addMessageInformation(info, ts->getTimestamp());
        } else {
            // Cannot add this guy.
        }
        break;

     case MessageInformation::Alliance:
        // Alliance: add it. Ignore the age here.
        // ex game/msgglobal.cc:mergeAllies
        for (MessageInformation::Iterator_t i = info.begin(); i != info.end(); ++i) {
            if (game::parser::MessageAllianceValue_t* cv = dynamic_cast<game::parser::MessageAllianceValue_t*>(*i)) {
                game::alliance::Container& allies = currentTurn().alliances();
                if (game::alliance::Offer* p = allies.getMutableOffer(allies.find(cv->getIndex()))) {
                    p->merge(cv->getValue());
                }
            }
        }
        break;

     case MessageInformation::MarkerDrawing:
     case MessageInformation::CircleDrawing:
     case MessageInformation::LineDrawing:
     case MessageInformation::RectangleDrawing:
        // Drawing
        currentTurn().universe().drawings().addMessageInformation(info, atomTable);
        break;

     case MessageInformation::NoObject:
        break;
    }
}

void
game::Game::synchronizeTeamsFromAlliances()
{
    // ex game/team.cc:syncTeamsFromAlliances, team.pas:SyncTeams
    // @change This does NOT check the preferences option
    using game::alliance::Level;

    const game::alliance::Container& allies = currentTurn().alliances();
    TeamSettings& teams = teamSettings();
    const int me = getViewpointPlayer();       // FIXME: was: getRealPlayerId();
    const int myTeam = teams.getPlayerTeam(me);

    if (!allies.getLevels().empty()) {
        // We actually have alliances
        for (int i = 1; i <= MAX_PLAYERS; ++i) {
            if (i != me) {
                // It's a relation to another player
                if (teams.getPlayerTeam(i) == myTeam) {
                    // They are on my team. Are we allied? If not, remove them.
                    // (only check our offers; if we offer an alliance, we consider them on our team.)
                    if (!allies.isAny(i, Level::IsOffer, true)) {
                        teams.removePlayerTeam(i);
                    }
                } else {
                    // They are not on my team.
                    if (allies.isAny(i, Level::IsOffer, true)) {
                        teams.setPlayerTeam(i, myTeam);
                    }
                }
            }
        }
    }
}

void
game::Game::notifyListeners()
{
    // Some parts may see current, some see viewpoint.
    // Thus, notify both.

    // Current turn
    Turn* t1 = &*m_currentTurn;
    t1->notifyListeners();

    // Viewpoint turn
    Turn* t2 = &viewpointTurn();
    if (t2 != t1) {
        t2->notifyListeners();
    }
}

bool
game::Game::isGameObject(const game::vcr::Object& obj, const game::spec::HullVector_t& hulls) const
{
    // ex WVcrSelectorWindowRealGameInterface::isGameObject
    // FIXME: 20210417 Is this a nice place for this function?
    const game::map::Universe& univ = viewpointTurn().universe();
    if (!obj.isPlanet()) {
        const game::map::Ship* sh = univ.allShips().getObjectByIndex(obj.getId());
        int hullId;
        return sh != 0
            && sh->getHull().get(hullId)
            && obj.canBeHull(hulls, hullId);
    } else {
        const game::map::Planet* pl = univ.allPlanets().getObjectByIndex(obj.getId());
        return pl != 0;
    }
    return false;
}
