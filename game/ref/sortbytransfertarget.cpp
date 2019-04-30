/**
  *  \file game/ref/sortbytransfertarget.cpp
  */

#include "game/ref/sortbytransfertarget.hpp"
#include "afl/string/format.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "util/math.hpp"

using afl::string::Format;
using game::map::Ship;
using game::map::Planet;
using game::Reference;

namespace {
    Reference checkTransporter(const Ship& ship, Ship::Transporter tr)
    {
        if (ship.isTransporterActive(tr)) {
            game::Id_t id;
            if (ship.getTransporterTargetId(tr).get(id)) {
                switch (tr) {
                 case Ship::TransferTransporter:
                    return Reference(Reference::Ship, id);
                 case Ship::UnloadTransporter:
                    return Reference(Reference::Planet, id);
                }
            }
        }
        return Reference();
    }

    int classify(Reference r)
    {
        switch (r.getType()) {
         case Reference::Ship:   return 1;
         case Reference::Planet: return 2;
         default:                return 0;
        }
    }
}


game::ref::SortByTransferTarget::SortByTransferTarget(const game::map::Universe& univ,
                                                      InterpreterInterface& iface,
                                                      game::map::Ship::Transporter transporterId,
                                                      bool checkOther,
                                                      afl::string::Translator& tx)
    : m_universe(univ),
      m_interface(iface),
      m_transporterId(transporterId),
      m_checkOther(checkOther),
      m_translator(tx)
{ }

int
game::ref::SortByTransferTarget::compare(const Reference& a, const Reference& b) const
{
    const Reference ta = getTarget(a);
    const Reference tb = getTarget(b);

    int result = util::compare3(classify(ta), classify(tb));
    if (result == 0) {
        result = util::compare3(ta.getId(), tb.getId());
    }
    if (result == 0) {
        result = util::compare3(classify(a), classify(b));
    }
    if (result == 0) {
        result = util::compare3(a.getId(), b.getId());
    }
    return result;
}

String_t
game::ref::SortByTransferTarget::getClass(const Reference& a) const
{
    const Reference ta = getTarget(a);
    switch (ta.getType()) {
     case Reference::Ship: {
        String_t shipName;
        if (const Ship* pShip = m_universe.ships().get(ta.getId())) {
            shipName = pShip->getName(PlainName, m_translator, m_interface);
        }
        if (shipName.empty()) {
            shipName = Format("#%d", ta.getId());
        }
        return Format(m_translator("Transferring to %s"), shipName);
     }

     case Reference::Planet:
        if (ta.getId() == 0) {
            return m_translator("Jettison");
        } else {
            String_t planetName;
            if (const Planet* pPlanet = m_universe.planets().get(ta.getId())) {
                planetName = pPlanet->getName(PlainName, m_translator, m_interface);
            }
            if (planetName.empty()) {
                planetName = Format("#%d", ta.getId());
            }
            return Format(m_translator("Unloading to %s"), planetName);
        }

     default:
        return String_t();
    }
}

game::Reference
game::ref::SortByTransferTarget::getTarget(const Reference a) const
{
    Reference result;
    if (const Ship* pShip = dynamic_cast<const Ship*>(m_universe.getObject(a))) {
        // Check requested transporter
        result = checkTransporter(*pShip, m_transporterId);

        // Check other transporter if desired
        if (!result.isSet() && m_checkOther) {
            switch (m_transporterId) {
             case Ship::TransferTransporter:
                result = checkTransporter(*pShip, Ship::UnloadTransporter);
                break;
             case Ship::UnloadTransporter:
                result = checkTransporter(*pShip, Ship::TransferTransporter);
                break;
            }
        }
    }
    return result;
}
