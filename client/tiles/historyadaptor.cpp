/**
  *  \file client/tiles/historyadaptor.cpp
  *  \brief Class client::tiles::HistoryAdaptor
  */

#include "client/tiles/historyadaptor.hpp"

namespace {
    int pickTurnNumber(const game::map::ShipLocationInfos_t& infos)
    {
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            if (infos[i].position.isValid()) {
                return infos[i].turnNumber;
            }
        }
        return 0;
    }
}

client::tiles::HistoryAdaptor::HistoryAdaptor(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& reply)
    : m_shipId(),
      m_locations(),
      m_turnNumber(),
      m_proxy(gameSender, reply),
      conn_proxyChange(m_proxy.sig_change.add(this, &HistoryAdaptor::onChange))
{ }

client::tiles::HistoryAdaptor::~HistoryAdaptor()
{ }

game::proxy::HistoryShipProxy&
client::tiles::HistoryAdaptor::proxy()
{
    return m_proxy;
}

game::Id_t
client::tiles::HistoryAdaptor::getShipId() const
{
    return m_shipId;
}

const game::map::ShipLocationInfos_t&
client::tiles::HistoryAdaptor::getPositionList() const
{
    return m_locations;
}

int
client::tiles::HistoryAdaptor::getTurnNumber() const
{
    return m_turnNumber;
}

void
client::tiles::HistoryAdaptor::setTurnNumber(int turnNumber)
{
    if (turnNumber != m_turnNumber) {
        m_turnNumber = turnNumber;
        sig_turnChange.raise();
    }
}

const game::map::ShipLocationInfo*
client::tiles::HistoryAdaptor::getCurrentTurnInformation() const
{
    size_t pos;
    if (findTurnNumber(m_locations, m_turnNumber).get(pos)) {
        return &m_locations[pos];
    } else {
        return 0;
    }
}

void
client::tiles::HistoryAdaptor::onChange(const game::proxy::HistoryShipProxy::Status& st)
{
    // Update turn number
    if (st.turnNumber.get(m_turnNumber)) {
        // Proxy has provided a new turn number
    } else {
        // Proxy did not provide a turn number. If ship changed, or current turn no longer valid, pick one.
        if (m_shipId != st.shipId || !findTurnNumber(st.locations, m_turnNumber).isValid()) {
            m_turnNumber = pickTurnNumber(st.locations);
        }
    }

    // Update other fields
    m_shipId = st.shipId;
    m_locations = st.locations;

    sig_listChange.raise();
    sig_turnChange.raise();
}

afl::base::Optional<size_t>
client::tiles::findTurnNumber(const game::map::ShipLocationInfos_t& infos, int turnNumber)
{
    for (size_t i = 0, n = infos.size(); i < n; ++i) {
        if (infos[i].turnNumber == turnNumber) {
            return i;
        }
    }
    return afl::base::Nothing;
}
