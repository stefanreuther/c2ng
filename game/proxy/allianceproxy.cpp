/**
  *  \file game/proxy/allianceproxy.cpp
  *  \brief Class game::proxy::AllianceProxy
  */

#include "game/proxy/allianceproxy.hpp"
#include "game/game.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

game::proxy::AllianceProxy::AllianceProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

game::proxy::AllianceProxy::Status
game::proxy::AllianceProxy::getStatus(WaitIndicator& ind)
{
    class Query : public util::Request<Session> {
     public:
        Query(Status& status)
            : m_status(status)
            { }
        void handle(Session& session)
            {
                if (Game* pGame = session.getGame().get()) {
                    pGame->currentTurn().alliances().postprocess();

                    m_status.alliances = pGame->currentTurn().alliances();
                    m_status.viewpointPlayer = pGame->getViewpointPlayer();
                }
                if (const Root* pRoot = session.getRoot().get()) {
                    for (int i = 1; i <= MAX_PLAYERS; ++i) {
                        if (const Player* pl = pRoot->playerList().get(i)) {
                            if (pl->isReal()) {
                                m_status.playerNames.set(i, pl->getName(Player::ShortName, session.translator()));
                                m_status.players += i;
                            }
                        }
                    }
                }
            }

     private:
        Status& m_status;
    };

    Status result;
    Query q(result);
    ind.call(m_gameSender, q);
    return result;
}

void
game::proxy::AllianceProxy::setAlliances(const game::alliance::Container& alliances)
{
    class Query : public util::Request<Session> {
     public:
        Query(const game::alliance::Container& data)
            : m_data(data)
            { }
        void handle(Session& session)
            {
                if (Game* pGame = session.getGame().get()) {
                    // Update alliances
                    pGame->currentTurn().alliances().copyFrom(m_data);

                    // Update teams
                    if (const Root* pRoot = session.getRoot().get()) {
                        if (pRoot->userConfiguration()[game::config::UserConfiguration::Team_AutoSync]()) {
                            pGame->synchronizeTeamsFromAlliances();
                        }
                        if (pRoot->userConfiguration()[game::config::UserConfiguration::Team_SyncTransfer]()) {
                            pGame->teamSettings().synchronizeDataTransferConfigurationFromTeams();
                        }
                    }
                }
            }
     private:
        const game::alliance::Container m_data;
    };
    m_gameSender.postNewRequest(new Query(alliances));
}
