/**
  *  \file game/game.cpp
  *  \brief Class game::Game
  */

#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/alliance/container.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/anyplanettype.hpp"

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
      m_messageConfiguration(),
      m_expressionLists()
{
    m_cursors.setUniverse(&m_currentTurn->universe());
}

game::Game::~Game()
{ }

game::Turn&
game::Game::currentTurn()
{
    return *m_currentTurn;
}

const game::Turn&
game::Game::currentTurn() const
{
    return *m_currentTurn;
}

game::HistoryTurnList&
game::Game::previousTurns()
{
    return m_previousTurns;
}

const game::HistoryTurnList&
game::Game::previousTurns() const
{
    return m_previousTurns;
}

game::UnitScoreDefinitionList&
game::Game::planetScores()
{
    return m_planetScores;
}

const game::UnitScoreDefinitionList&
game::Game::planetScores() const
{
    return m_planetScores;
}

game::UnitScoreDefinitionList&
game::Game::shipScores()
{
    return m_shipScores;
}

const game::UnitScoreDefinitionList&
game::Game::shipScores() const
{
    return m_shipScores;
}

int
game::Game::getViewpointPlayer() const
{
    return m_teamSettings.getViewpointPlayer();
}

void
game::Game::setViewpointPlayer(int playerNr)
{
    m_teamSettings.setViewpointPlayer(playerNr);
}

afl::base::Ptr<game::Turn>
game::Game::getViewpointTurn() const
{
    // FIXME: restrict setViewpointTurnNumber so that this never returns null
    if (m_viewpointTurnNumber == 0 || m_viewpointTurnNumber == currentTurn().getTurnNumber()) {
        return m_currentTurn.asPtr();
    } else if (HistoryTurn* ht = m_previousTurns.get(m_viewpointTurnNumber)) {
        return ht->getTurn();
    } else {
        return 0;
    }
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
    // Change turn number
    Turn* oldTurn = getViewpointTurn().get();
    m_viewpointTurnNumber = nr;
    Turn* newTurn = getViewpointTurn().get();

    // Update
    if (oldTurn != newTurn) {
        // Transfer selection to new turn
        // FIXME: the limitToExistingObjects() will unmark objects that don't exist in the new turn.
        // It would be nice if we could avoid that.
        // However, the copyFrom() will already unmark nonexistant objects,
        // effectively doing the equivalent of limitToExistingObjects().
        // Until we can somehow avoid that, keep the limitToExistingObjects().
        if (oldTurn != 0) {
            m_selections.copyFrom(oldTurn->universe(), m_selections.getCurrentLayer());
        }
        if (newTurn != 0) {
            m_selections.copyTo(newTurn->universe(), m_selections.getCurrentLayer());
            m_selections.limitToExistingObjects(newTurn->universe(), m_selections.getCurrentLayer());
        }

        // Change cursor
        m_cursors.setUniverse(newTurn != 0 ? &newTurn->universe() : 0);
        sig_viewpointTurnChange.raise();
    }
}

game::TeamSettings&
game::Game::teamSettings()
{
    return m_teamSettings;
}

const game::TeamSettings&
game::Game::teamSettings() const
{
    return m_teamSettings;
}

game::score::TurnScoreList&
game::Game::scores()
{
    return m_scores;
}

const game::score::TurnScoreList&
game::Game::scores() const
{
    return m_scores;
}

game::map::Cursors&
game::Game::cursors()
{
    return m_cursors;
}

game::map::Selections&
game::Game::selections()
{
    return m_selections;
}

game::msg::Configuration&
game::Game::messageConfiguration()
{
    return m_messageConfiguration;
}

const game::msg::Configuration&
game::Game::messageConfiguration() const
{
    return m_messageConfiguration;
}

game::config::ExpressionLists&
game::Game::expressionLists()
{
    return m_expressionLists;
}

const game::config::ExpressionLists&
game::Game::expressionLists() const
{
    return m_expressionLists;
}

void
game::Game::addMessageInformation(const game::parser::MessageInformation& info, game::config::HostConfiguration& config, afl::base::Optional<size_t> msgNr)
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
        // Ship: add normally, with no claim to reliability (=empty source set).
        // To add information to be treated as reliable (e.g. target.dat file), add it to the ship directly.
        if (game::map::Ship* pShip = currentTurn().universe().ships().get(info.getObjectId())) {
            pShip->addMessageInformation(info, PlayerSet_t());
            if (const size_t* pNr = msgNr.get()) {
                pShip->messages().add(*pNr);
            }
        }
        break;

     case MessageInformation::Planet:
     case MessageInformation::Starbase:
        // Planet: add normally
        if (game::map::Planet* pPlanet = currentTurn().universe().planets().get(info.getObjectId())) {
            pPlanet->addMessageInformation(info);
            if (const size_t* pNr = msgNr.get()) {
                pPlanet->messages().add(*pNr);
            }
        }
        break;

     case MessageInformation::Minefield:
        // Minefield: add normally. MinefieldType will deal with details.
        currentTurn().universe().minefields().addMessageInformation(info);
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

     case MessageInformation::NoObject:
        break;
    }
}

void
game::Game::synchronizeTeamsFromAlliances()
{
    // ex game/team.cc:syncTeamsFromAlliances
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
    // FIXME: as of 20180101, the script side only operates on "current", but the C++ GUI side partially displays "viewpoint".
    // Thus, notify both.

    // Current turn
    Turn* t1 = &*m_currentTurn;
    t1->notifyListeners();

    // Viewpoint turn
    Turn* t2 = getViewpointTurn().get();
    if (t2 != 0 && t2 != t1) {
        t2->notifyListeners();
    }
}

bool
game::Game::isGameObject(const game::vcr::Object& obj, const game::spec::HullVector_t& hulls) const
{
    // ex WVcrSelectorWindowRealGameInterface::isGameObject
    // FIXME: 20210417 Is this a nice place for this function?
    if (Turn* t = getViewpointTurn().get()) {
        if (!obj.isPlanet()) {
            const game::map::Ship* sh = game::map::AnyShipType(t->universe()).getObjectByIndex(obj.getId());
            int hullId;
            return sh != 0
                && sh->getHull().get(hullId)
                && obj.canBeHull(hulls, hullId);
        } else {
            const game::map::Planet* pl = game::map::AnyPlanetType(t->universe()).getObjectByIndex(obj.getId());
            return pl != 0;
        }
    }
    return false;
}
