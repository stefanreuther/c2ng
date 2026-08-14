/**
  *  \file server/console/clientpool.cpp
  *  \brief Class server::console::ClientPool
  */

#include "server/console/clientpool.hpp"

#include "afl/except/assertionfailedexception.hpp"
#include "afl/net/name.hpp"
#include "server/ports.hpp"

/*
 *  Entry - just a structure storing our data
 */

struct server::console::ClientPool::Entry {
    const String_t name;
    afl::net::Name address;
    std::auto_ptr<afl::net::resp::Client> client;

    Entry(const String_t& name, uint16_t defaultPort)
        : name(name),
          address(DEFAULT_ADDRESS, defaultPort),
          client()
        { }
};

/*
 *  ClientPool
 */

server::console::ClientPool::ClientPool(afl::net::NetworkStack& stack)
    : m_networkStack(stack),
      m_entries()
{ }

server::console::ClientPool::~ClientPool()
{ }

server::console::ClientPool::Index_t
server::console::ClientPool::create(String_t name, uint16_t defaultPort)
{
    m_entries.pushBackNew(new Entry(name, defaultPort));
    return m_entries.size()-1;
}

server::console::ClientPool::Index_t
server::console::ClientPool::find(String_t name) const
{
    for (Index_t i = 0, n = m_entries.size(); i < n; ++i) {
        Entry& e = *m_entries[i];
        if (e.name == name) {
            return i;
        }
    }
    throw afl::except::AssertionFailedException("Unknown name: " + name, "ClientPool.find");
}

afl::net::resp::Client&
server::console::ClientPool::get(Index_t index)
{
    afl::except::checkAssertion(index < m_entries.size(), "Bad index", "ClientPool.get");
    Entry& e = *m_entries[index];
    if (e.client.get() == 0) {
        e.client.reset(new afl::net::resp::Client(m_networkStack, e.address));
    }
    return *e.client;
}

void
server::console::ClientPool::reset(Index_t index)
{
    if (index < m_entries.size()) {
        m_entries[index]->client.reset();
    }
}

bool
server::console::ClientPool::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex server::console::ConnectionContextFactory::handleConfiguration(const String_t& key, const String_t& value)
    // ex ConnectionContext::checkConfig
    for (Index_t i = 0, n = m_entries.size(); i < n; ++i) {
        Entry& e = *m_entries[i];
        if (afl::string::strCaseCompare(key, e.name + ".host") == 0) {
            e.address.setName(value);
            return true;
        }
        if (afl::string::strCaseCompare(key, e.name + ".port") == 0) {
            e.address.setService(value);
            return true;
        }
    }
    return false;
}
