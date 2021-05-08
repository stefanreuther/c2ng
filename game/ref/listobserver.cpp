/**
  *  \file game/ref/listobserver.cpp
  */

#include "game/ref/listobserver.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/ref/configuration.hpp"
#include "game/ref/sortpredicate.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

game::ref::ListObserver::ListObserver()
    : m_mainList(),
      m_extraList(),
      m_resultList(),
      m_pConfigurationSelection(&REGULAR),
      m_pSession(0)
{ }

void
game::ref::ListObserver::setList(const List& list)
{
    m_mainList = list;
    updateResultList();
}

void
game::ref::ListObserver::setExtra(const UserList& list)
{
    m_extraList = list;
    updateResultList();
}

void
game::ref::ListObserver::setSession(Session& session)
{
    m_pSession = &session;
    updateResultList();

    if (Game* g = m_pSession->getGame().get()) {
        conn_viewpointTurnChange = g->sig_viewpointTurnChange.add(this, &ListObserver::onViewpointTurnChange);
        onViewpointTurnChange();
    }
}

void
game::ref::ListObserver::setConfigurationSelection(const ConfigurationSelection& sel)
{
    m_pConfigurationSelection = &sel;
    updateResultList();
}

game::ref::Configuration
game::ref::ListObserver::getConfig() const
{
    Configuration fig;
    if (m_pSession != 0) {
        fetchConfiguration(*m_pSession, *m_pConfigurationSelection, fig);
    }
    return fig;
}

void
game::ref::ListObserver::setConfig(const Configuration& config)
{
    if (m_pSession != 0) {
        storeConfiguration(*m_pSession, *m_pConfigurationSelection, config);
    }
    updateResultList();
}

const game::ref::UserList&
game::ref::ListObserver::getList()
{
    return m_resultList;
}

void
game::ref::ListObserver::updateResultList()
{
    UserList newList;

    if (m_pSession != 0) {
        // Sorting
        Configuration fig;
        fetchConfiguration(*m_pSession, *m_pConfigurationSelection, fig);
        afl::base::Deleter del;
        SortPredicate& firstPredicate  = createSortPredicate(fig.order.first,  *m_pSession, del);
        SortPredicate& secondPredicate = createSortPredicate(fig.order.second, *m_pSession, del);

        // Main list
        m_mainList.sort(firstPredicate.then(secondPredicate));
        newList.add(m_mainList, *m_pSession, firstPredicate, secondPredicate);

        // Extra list
        if (newList.size() != m_mainList.size()
            && m_extraList.size() != 0
            && m_extraList.get(0)->type != UserList::DividerItem)
        {
            newList.add(UserList::DividerItem, m_pSession->translator().translateString("Other"), Reference(), false, game::map::Object::NotPlayable, util::SkinColor::Static);
        }
        newList.add(m_extraList);
    }

    if (newList != m_resultList) {
        m_resultList = newList;
        sig_listChange.raise();
    }
}

void
game::ref::ListObserver::onViewpointTurnChange()
{
    conn_universeChange.disconnect();

    if (m_pSession != 0) {
        afl::base::Ptr<Game> g = m_pSession->getGame();
        if (g.get() != 0) {
            afl::base::Ptr<Turn> t = g->getViewpointTurn();
            if (t.get() != 0) {
                conn_universeChange = t->universe().sig_universeChange.add(this, &ListObserver::onUniverseChange);
                onUniverseChange();
            }
        }
    }
}

void
game::ref::ListObserver::onUniverseChange()
{
    updateResultList();
}
