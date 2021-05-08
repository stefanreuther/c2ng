/**
  *  \file game/actions/multitransfersetup.cpp
  *  \brief Class game::actions::MultiTransferSetup
  */

#include "game/actions/multitransfersetup.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/actions/preconditions.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/shipstorage.hpp"
#include "game/ref/configuration.hpp"
#include "game/ref/list.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"

using game::map::Object;
using game::map::Planet;
using game::map::PlanetStorage;
using game::map::Ship;
using game::map::ShipStorage;

namespace {
    /** Add newly-constructed cargo container to transfer.
        \param tx   [in/out] Cargo transfer
        \param what [in] Cargo type
        \param cnt  [in] Newly-created cargo container
        \retval true Container was added to the transfer
        \retval false Container cannot contain desired cargo type and was deleted */
    bool tryAdd(game::actions::CargoTransfer& tx, game::Element::Type what, game::CargoContainer* cnt)
    {
        if (cnt->canHaveElement(what)) {
            tx.addNew(cnt);
            return true;
        } else {
            delete cnt;
            return false;
        }
    }
}

game::actions::MultiTransferSetup::MultiTransferSetup()
    : m_shipId(0),
      m_fleet(false),
      m_element(Element::Neutronium)
{ }

void
game::actions::MultiTransferSetup::setShipId(Id_t shipId)
{
    m_shipId = shipId;
}

void
game::actions::MultiTransferSetup::setFleetOnly(bool flag)
{
    m_fleet = flag;
}

void
game::actions::MultiTransferSetup::setElementType(Element::Type type)
{
    m_element = type;
}

game::Id_t
game::actions::MultiTransferSetup::getShipId() const
{
    return m_shipId;
}

bool
game::actions::MultiTransferSetup::isFleetOnly() const
{
    return m_fleet;
}

game::Element::Type
game::actions::MultiTransferSetup::getElementType() const
{
    return m_element;
}

game::ElementTypes_t
game::actions::MultiTransferSetup::getSupportedElementTypes(const game::map::Universe& univ,
                                                            const game::spec::ShipList& shipList) const
{
    afl::string::NullTranslator tx;
    ElementTypes_t result;
    if (const Ship* sh = univ.ships().get(m_shipId)) {
        game::map::Point shipPos;
        int shipOwner;
        if (sh->isPlayable(Object::Playable) && sh->getPosition(shipPos) && sh->getOwner(shipOwner)) {
            ShipStorage storage(*const_cast<Ship*>(sh), shipList, tx);
            for (Element::Type ty = Element::begin(), last = Element::end(shipList); ty != last; ++ty) {
                if (storage.canHaveElement(ty)) {
                    result += ty;
                }
            }
        }
    }
    return result;
}

game::actions::MultiTransferSetup::Result
game::actions::MultiTransferSetup::build(CargoTransfer& action, game::map::Universe& univ, Session& session) const
{
    // ex buildTransferList
    Result result;

    // Environment
    game::spec::ShipList& shipList = mustHaveShipList(session);
    Root& root = mustHaveRoot(session);
    afl::string::Translator& tx = session.translator();

    // First object always is hold space
    action.addHoldSpace(tx("Hold space"));

    // Collect game objects
    if (Ship* sh = univ.ships().get(m_shipId)) {
        game::map::Point shipPos;
        int shipOwner;
        if (sh->isPlayable(Object::Playable) && sh->getPosition(shipPos) && sh->getOwner(shipOwner)) {
            // Collect all ships in a list
            afl::base::Deleter del;
            game::ref::List list;
            list.addObjectsAt(univ, shipPos, game::ref::List::Options_t(), 0);
            list.sort(createSortPredicate(game::ref::CARGO_TRANSFER, session, del));

            // Add applicable ships
            for (size_t i = 0, n = list.size(); i < n; ++i) {
                if (Ship* s2 = dynamic_cast<Ship*>(univ.getObject(list[i]))) {
                    game::map::Point s2Pos;
                    int s2Owner;
                    if (s2->isPlayable(Object::Playable) && s2->getPosition(s2Pos) && s2->getOwner(s2Owner)
                        && s2Pos == shipPos && s2Owner == shipOwner
                        && (!m_fleet || s2->getFleetNumber() == sh->getFleetNumber()))
                    {
                        // Let's use this ship
                        if (s2->getId() == m_shipId) {
                            result.thisShipIndex = action.getNumContainers();
                        }
                        tryAdd(action, m_element, new ShipStorage(*s2, shipList, tx));
                    }
                }
            }

            // Is there a planet?
            if (Id_t planetId = univ.findPlanetAt(shipPos)) {
                if (Planet* pl = univ.planets().get(planetId)) {
                    int planetOwner;
                    if (pl->isPlayable(Object::Playable) && pl->getOwner(planetOwner) && planetOwner == shipOwner) {
                        // We play this planet, so use it.
                        if (tryAdd(action, m_element, new PlanetStorage(*pl, root.hostConfiguration(), tx))) {
                            result.extensionIndex = action.getNumContainers() - 1;
                        }
                    }
                }
            }
        }

        // Check availability of cargo
        int32_t totalCargo = 0;
        for (size_t i = 0, n = action.getNumContainers(); i < n; ++i) {
            totalCargo += action.get(i)->getAmount(m_element);
        }
        if (totalCargo == 0) {
            // No cargo, action is pointless
            result.status = NoCargo;
        } else if (action.getNumContainers() < 3) {
            // Need at least 3 units (hold space + 2 units) for the action to make any sense
            result.status = NoPeer;
        } else {
            result.status = Success;
        }
    }

    return result;
}
