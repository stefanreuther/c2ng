/**
  *  \file client/objectobserver.cpp
  */

#include "client/objectobserver.hpp"
#include "game/game.hpp"
#include "client/objectlistener.hpp"

client::ObjectObserver::ObjectObserver(std::auto_ptr<ObjectCursorFactory> f)
    : m_factory(f),
      m_observer(),
      m_pSession(),
      conn_objectChange(),
      m_listeners()
{ }

client::ObjectObserver::~ObjectObserver()
{ }

void
client::ObjectObserver::init(game::Session& s)
{
    if (game::map::ObjectCursor* c = m_factory->getCursor(s)) {
        m_observer.reset(new game::map::ObjectObserver(*c));
        conn_objectChange = m_observer->sig_objectChange.add(this, &ObjectObserver::onObjectChange);
        m_pSession = &s; // FIXME
    }
}

void
client::ObjectObserver::done(game::Session& /*s*/)
{
    m_observer.reset();
}

void
client::ObjectObserver::addNewListener(game::Session& s, ObjectListener* pl)
{
    m_listeners.pushBackNew(pl);
    if (m_observer.get() != 0) {
        game::map::Object* p = m_observer->getCurrentObject();
        pl->handle(s, p);
    }
}

void
client::ObjectObserver::onObjectChange()
{
    if (m_observer.get() != 0) {
        game::map::Object* p = m_observer->getCurrentObject();
        for (size_t i = 0; i < m_listeners.size(); ++i) {
            m_listeners[i]->handle(*m_pSession, p);
        }
    }
}
