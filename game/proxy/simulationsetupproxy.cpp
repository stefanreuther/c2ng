/**
  *  \file game/proxy/simulationsetupproxy.cpp
  *  \brief Class game::proxy::SimulationSetupProxy
  *
  *  FIXME: 20201218: This implements some "if-this-then-that" logic, e.g. "clear cloak if ship is set to agg_Kill",
  *  and some range logic, e.g. "Lizards have 150 damage max".
  *  Consider moving that into game/sim/.
  *
  *  FIXME: 20201218: for now, this can only access the game::sim::Session that is associated with a game::Session.
  *  Try to avoid dependencies on the game::Session (other than for initialisation) to allow future expansion.
  */

#include <cmath>
#include "game/proxy/simulationsetupproxy.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/translator.hpp"
#include "game/root.hpp"
#include "game/sim/loader.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/sessionextra.hpp"
#include "game/sim/ship.hpp"
#include "game/spec/basichullfunction.hpp"
#include "util/math.hpp"
#include "game/sim/sort.hpp"

using afl::string::Format;
using game::config::HostConfiguration;
using game::sim::GameInterface;
using game::sim::Object;
using game::sim::Configuration;
using game::sim::Planet;
using game::sim::Setup;
using game::sim::Ship;
using game::spec::BasicHullFunction;
using game::spec::ShipList;
typedef game::proxy::SimulationSetupProxy::Element_t Element_t;

namespace {
    /*
     *  Ad-hoc update post-processing
     *
     *  Some changes need additional postprocessing, i.e. an update to an Id updates not only the object, but also the list.
     *  To keep code general, postprocessing is handled using this set of bits.
     */
    const uint32_t UpdateList               = 1;
    const uint32_t UpdateAggressiveness     = 2;
    const uint32_t UpdateOwner              = 4;
    const uint32_t UpdateInterceptId        = 8;
    const uint32_t UpdateRandomFriendlyCode = 16;
    const uint32_t UpdateDamageShield       = 32;


    /*
     *  Number-to-Element_t formatters
     */

    Element_t describeOwner(int player, const game::Root& root, afl::string::Translator& tx)
    {
        return Element_t(player, root.playerList().getPlayerName(player, game::Player::ShortName, tx));
    }

    Element_t describeExperienceLevel(int level, const game::Root& root, afl::string::Translator& tx)
    {
        return Element_t(level, root.hostConfiguration().getExperienceLevelName(level, tx));
    }

    Element_t describeHull(int hullType, const ShipList& shipList, afl::string::Translator& tx)
    {
        if (hullType == 0) {
            return Element_t(hullType, tx("Custom Ship"));
        } else if (const game::spec::Hull* pHull = shipList.hulls().get(hullType)) {
            return Element_t(hullType, pHull->getName(shipList.componentNamer()));
        } else {
            return Element_t();
        }
    }

    int getHullPicture(int hullType, const ShipList& shipList)
    {
        if (hullType == 0) {
            return 0;
        } else if (const game::spec::Hull* pHull = shipList.hulls().get(hullType)) {
            return pHull->getInternalPictureNumber();
        } else {
            return 0;
        }
    }

    Element_t describeBeam(int beamType, const ShipList& shipList, afl::string::Translator& tx)
    {
        if (beamType == 0) {
            return Element_t(beamType, tx("none"));
        } else if (const game::spec::Beam* pBeam = shipList.beams().get(beamType)) {
            return Element_t(beamType, pBeam->getName(shipList.componentNamer()));
        } else {
            return Element_t();
        }
    }

    Element_t describeTorpedo(int torpedoType, const ShipList& shipList, afl::string::Translator& tx)
    {
        if (torpedoType == 0) {
            return Element_t(torpedoType, tx("none"));
        } else if (const game::spec::TorpedoLauncher* pTorpedo = shipList.launchers().get(torpedoType)) {
            return Element_t(torpedoType, pTorpedo->getName(shipList.componentNamer()));
        } else {
            return Element_t();
        }
    }

    Element_t describeEngine(int engineType, const ShipList& shipList)
    {
        if (const game::spec::Engine* pEngine = shipList.engines().get(engineType)) {
            return Element_t(engineType, pEngine->getName(shipList.componentNamer()));
        } else {
            return Element_t();
        }
    }

    Element_t describeAggressiveness(int aggressiveness, const game::Root& root, afl::string::Translator& tx)
    {
        switch (aggressiveness) {
         case Ship::agg_Kill:
            return Element_t(aggressiveness, tx("Kill Mission"));
         case Ship::agg_Passive:
            return Element_t(aggressiveness, tx("Passive Ship"));
         case Ship::agg_NoFuel:
            return Element_t(aggressiveness, tx("No Fuel"));
         default:
            return Element_t(aggressiveness, Format(tx("Primary Enemy %s"), root.playerList().getPlayerName(aggressiveness, game::Player::ShortName, tx)));
        }
    }

    Element_t describeInterceptId(game::Id_t id, const Setup& setup, afl::string::Translator& tx)
    {
        if (id == 0) {
            return Element_t(id, tx("none"));
        } else if (const Ship* sh = setup.findShipById(id)) {
            return Element_t(id, Format("%s (#%d)", sh->getName(), id));
        } else {
            return Element_t(id, Format(tx("Ship #%d"), id));
        }
    }

    /*
     *  Utilities
     */

    int getMaxDamage(const Object& obj, const HostConfiguration& config)
    {
        // ex GSimObject::getMaxDamage
        if (config.getPlayerRaceNumber(obj.getOwner()) == 2) {
            return 150;
        } else {
            return 99;
        }
    }

    int getMaxShield(const Object& obj, const HostConfiguration& config)
    {
        // ex GSimObject::getMaxShield
        return std::min(100, std::max(0, getMaxDamage(obj, config) - obj.getDamage() + 1));
    }

    int32_t getPopulationFromDefense(int defense)
    {
        if (defense < 50) {
            return defense;
        } else {
            return 50 + util::squareInteger(defense - 50);
        }
    }

    int getDefenseFromPopulation(int32_t pop)
    {
        if (pop <= 50) {
            return pop;
        } else {
            return 50 + int(std::sqrt(double(pop - 50)) + 0.5);
        }
    }
}


/*
 *  Trampoline
 */

class game::proxy::SimulationSetupProxy::Trampoline {
 public:
    typedef util::Request<Trampoline> Request_t;
    typedef util::Request<SimulationSetupProxy> Reply_t;

    Trampoline(util::RequestSender<SimulationSetupProxy> reply, Session& session);
    ~Trampoline();

    // Data export
    void packList(ListItems_t& list);
    void packObject(ObjectInfo& out, const Object& in, afl::string::Translator& tx);

    // List operations
    Slot_t addPlanet(ListItems_t& list);
    Slot_t addShip(Slot_t slot, int count, ListItems_t& list);
    void swapShips(Slot_t a, Slot_t b);
    void removeObject(Slot_t slot);
    void clear();
    void sortShips(SortOrder order);
    Setup::Status copyToGame(Slot_t from, Slot_t to);
    Setup::Status copyFromGame(Slot_t from, Slot_t to);
    bool load(String_t fileName, String_t& errorMessage);
    bool save(String_t fileName, String_t& errorMessage);

    // Unit operations
    void setSlot(Slot_t slot);
    bool getObject(Slot_t slot, ObjectInfo& info);
    bool isDuplicateId(Slot_t slot, Id_t candidate) const;
    void getNumBaseTorpedoes(Slot_t slot, Elements_t& result) const;

    // Setters
    void setFlags(Slot_t slot, uint32_t keep, uint32_t toggle);
    void setSequentialFriendlyCode(Slot_t slot);
    void setId(Slot_t slot, Id_t id);
    void setHullType(Slot_t slot, int hullType, bool afterAdd);
    void setNumBaseTorpedoes(Slot_t slot, const Elements_t& list);

    template<typename Object, typename Property>
    void setProperty(Slot_t slot, void (Object::*set)(Property), Property value, uint32_t updateFlags);

    // Choice inquiry
    void getAbilityChoices(Slot_t slot, AbilityChoices& result);
    void getFriendlyCodeChoices(Slot_t slot, game::spec::FriendlyCodeList::Infos_t& result);
    void getOwnerChoices(Elements_t& result);
    void getExperienceLevelChoices(Elements_t& result);
    void getHullTypeChoices(Elements_t& result);
    void getPrimaryChoices(Slot_t slot, PrimaryChoices& result);
    void getSecondaryChoices(Slot_t slot, SecondaryChoices& result);
    void getEngineTypeChoices(Elements_t& result);
    void getAggressivenessChoices(Elements_t& result);
    void getBaseBeamLevelChoices(Elements_t& result);
    void getBaseTorpedoLevelChoices(Elements_t& result);
    void getPlanetNameChoices(Elements_t& result);
    void getPopulationChoices(Slot_t slot, PopulationChoices& result);

    // Range inquiry
    Range_t getIdRange(Slot_t slot);
    Range_t getDamageRange(Slot_t slot);
    Range_t getShieldRange(Slot_t slot);
    Range_t getCrewRange(Slot_t slot);
    Range_t getInterceptIdRange(Slot_t slot);
    Range_t getBaseDefenseRange(Slot_t slot);
    Range_t getNumBaseFightersRange(Slot_t slot);

    // Configuration
    void getConfiguration(Configuration& config);
    void setConfiguration(const Configuration& config, Configuration::Areas_t areas);

 private:
    Setup& getSetup() const;
    GameInterface* getGameInterface() const;
    const game::spec::ShipList* getShipList() const;
    int getTorpedoPowerScale() const;

    void notifyListeners(bool blockList);

    void sendListChange();
    void sendObjectChange();

    void update(Setup& setup, Object& obj, uint32_t flags);
    void updatePlanetName(Setup& setup);
    void onStructureChange();
    void onPlanetChange();
    void onShipChange(Slot_t slot);

    util::RequestSender<SimulationSetupProxy> m_reply;
    afl::base::Ref<game::sim::Session> m_sim;
    afl::base::Ptr<game::spec::ShipList> m_shipList;
    afl::base::Ptr<Root> m_root;
    afl::string::Translator& m_translator;
    afl::io::FileSystem& m_fileSystem;
    afl::base::SignalConnection conn_structureChange;
    afl::base::SignalConnection conn_planetChange;
    afl::base::SignalConnection conn_shipChange;
    afl::base::Optional<Slot_t> m_observedSlot;
    bool m_suppressStructureChanges;
    bool m_structureChanged;
};

class game::proxy::SimulationSetupProxy::TrampolineFromSession : public afl::base::Closure<Trampoline* (game::Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<SimulationSetupProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(game::Session& session)
        { return new Trampoline(m_reply, session); }
 private:
    util::RequestSender<SimulationSetupProxy> m_reply;
};

inline
game::proxy::SimulationSetupProxy::Trampoline::Trampoline(util::RequestSender<SimulationSetupProxy> reply, Session& session)
    : m_reply(reply),
      m_sim(game::sim::getSimulatorSession(session)),
      m_shipList(session.getShipList()),
      m_root(session.getRoot()),
      m_translator(session.translator()),
      m_fileSystem(session.world().fileSystem()),
      conn_structureChange(), conn_planetChange(), conn_shipChange(),
      m_observedSlot(),
      m_suppressStructureChanges(false),
      m_structureChanged(false)
{
    Setup& setup = m_sim->setup();
    conn_structureChange = setup.sig_structureChange.add(this, &Trampoline::onStructureChange);
    conn_planetChange    = setup.sig_planetChange.add(this, &Trampoline::onPlanetChange);
    conn_shipChange      = setup.sig_shipChange.add(this, &Trampoline::onShipChange);

    updatePlanetName(setup);
}

game::proxy::SimulationSetupProxy::Trampoline::~Trampoline()
{ }

void
game::proxy::SimulationSetupProxy::Trampoline::packList(ListItems_t& list)
{
    // ex WSimList::drawPart (part)
    m_structureChanged = false;

    const Setup& setup = getSetup();
    afl::string::Translator& tx = m_translator;
    for (Slot_t i = 0, n = setup.getNumObjects(); i < n; ++i) {
        const Object* obj = setup.getObject(i);
        ListItem item;
        item.id = obj->getId();
        item.disabled = (obj->getFlags() & Object::fl_Deactivated) != 0;
        item.name = obj->getName();
        if (m_root.get() != 0 && m_shipList.get() != 0) {
            const String_t playerAdjective = m_root->playerList().getPlayerName(obj->getOwner(), Player::AdjectiveName, tx);
            if (const Planet* p = dynamic_cast<const Planet*>(obj)) {
                if (p->hasBase()) {
                    item.info = Format(tx("%s planet+SB"), playerAdjective);
                } else {
                    item.info = Format(tx("%s planet"), playerAdjective);
                }
                item.isPlanet = true;
            }
            if (const Ship* sh = dynamic_cast<const Ship*>(obj)) {
                if (const game::spec::Hull* h = m_shipList->hulls().get(sh->getHullType())) {
                    item.info = Format(tx("%s %s"), playerAdjective, h->getShortName(m_shipList->componentNamer()));
                } else {
                    item.info = Format(tx("%s custom ship"), playerAdjective);
                }
            }
        }
        list.push_back(item);
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::packObject(ObjectInfo& out, const Object& in, afl::string::Translator& tx)
{
    out.id                       = in.getId();
    out.name                     = in.getName();
    out.friendlyCode             = in.getFriendlyCode();
    out.damage                   = in.getDamage();
    out.shield                   = in.getShield();
    out.owner                    = m_root.get() != 0 ? describeOwner(in.getOwner(),                     *m_root, tx) : Element_t();
    out.experienceLevel          = m_root.get() != 0 ? describeExperienceLevel(in.getExperienceLevel(), *m_root, tx) : Element_t();
    out.flags                    = in.getFlags();
    out.flakRatingOverride       = in.getFlakRatingOverride();
    out.flakCompensationOverride = in.getFlakCompensationOverride();
    out.abilities                = (m_root.get() != 0 && m_shipList.get() != 0
                                    ? in.getAbilities(m_sim->configuration(), *m_shipList, m_root->hostConfiguration())
                                    : game::sim::Abilities_t());
    out.hasAnyNonstandardAbility = in.hasAnyNonstandardAbility();

    // Ship properties
    const Ship* sh = dynamic_cast<const Ship*>(&in);
    out.crew           = sh != 0                          ? sh->getCrew()                                                 : 0;
    out.hullType       = sh != 0 && m_shipList.get() != 0 ? describeHull(sh->getHullType(), *m_shipList, tx)              : Element_t();
    out.hullPicture    = sh != 0 && m_shipList.get() != 0 ? getHullPicture(sh->getHullType(), *m_shipList)                : 0;
    out.mass           = sh != 0                          ? sh->getMass()                                                 : 0;
    out.beamType       = sh != 0 && m_shipList.get() != 0 ? describeBeam(sh->getBeamType(), *m_shipList, tx)              : Element_t();
    out.numBeams       = sh != 0                          ? sh->getNumBeams()                                             : 0;
    out.torpedoType    = sh != 0 && m_shipList.get() != 0 ? describeTorpedo(sh->getTorpedoType(), *m_shipList, tx)        : Element_t();
    out.numLaunchers   = sh != 0                          ? sh->getNumLaunchers()                                         : 0;
    out.numBays        = sh != 0                          ? sh->getNumBays()                                              : 0;
    out.ammo           = sh != 0                          ? sh->getAmmo()                                                 : 0;
    out.engineType     = sh != 0 && m_shipList.get() != 0 ? describeEngine(sh->getEngineType(), *m_shipList)              : Element_t();
    out.aggressiveness = sh != 0 && m_root.get() != 0     ? describeAggressiveness(sh->getAggressiveness(), *m_root,  tx) : Element_t();
    out.interceptId    = sh != 0                          ? describeInterceptId(sh->getInterceptId(), m_sim->setup(), tx) : Element_t();

    // Rating defaults
    if (sh != 0
        && (in.getFlags() & Object::fl_RatingOverride) == 0
        && m_root.get() != 0
        && m_shipList.get() != 0)
    {
        out.defaultFlakRating       = sh->getDefaultFlakRating      (m_root->flakConfiguration(), m_sim->configuration(), *m_shipList, m_root->hostConfiguration());
        out.defaultFlakCompensation = sh->getDefaultFlakCompensation(m_root->flakConfiguration(), m_sim->configuration(), *m_shipList, m_root->hostConfiguration());
    } else {
        out.defaultFlakRating = 0;
        out.defaultFlakCompensation = 0;
    }

    // Primary weapons: editable if range is not unit range (=[0,0] range)
    out.allowPrimaryWeapons   = (sh != 0 && m_shipList.get() != 0 && !sh->getNumBeamsRange(*m_shipList).isUnit());

    // Secondary weapons: editable if ranges are not empty, or unit has any weapons and we can edit ammo
    out.allowSecondaryWeapons = (sh != 0
                                 && m_shipList.get() != 0
                                 && (!sh->getNumLaunchersRange(*m_shipList).isUnit()
                                     || !sh->getNumBaysRange(*m_shipList).isUnit()
                                     || sh->getNumLaunchers() != 0
                                     || sh->getNumBays() != 0));

    // Planet properties
    const Planet* pl = dynamic_cast<const Planet*>(&in);
    out.isPlanet         = pl != 0;
    out.defense          = pl != 0                     ? pl->getDefense()                                                     : 0;
    out.baseDefense      = pl != 0                     ? pl->getBaseDefense()                                                 : 0;
    out.baseBeamTech     = pl != 0                     ? pl->getBaseBeamTech()                                                : 0;
    out.baseTorpedoTech  = pl != 0                     ? pl->getBaseTorpedoTech()                                             : 0;
    out.numBaseFighters  = pl != 0                     ? pl->getNumBaseFighters()                                             : 0;
    out.effBaseTorpedoes = pl != 0 && m_shipList.get() ? pl->getNumBaseTorpedoesAsType(pl->getBaseTorpedoTech(), *m_shipList) : 0;
    out.hasBase          = pl != 0 && pl->hasBase();

    // Relation
    const GameInterface* gi = getGameInterface();
    out.relation = (gi != 0
                    ? (pl != 0
                       ? gi->getPlanetRelation(*pl)
                       : sh != 0
                       ? gi->getShipRelation(*sh)
                       : GameInterface::Unknown)
                    : GameInterface::Unknown);

    // Position
    out.position = (gi != 0
                    ? (pl != 0
                       ? gi->getPlanetPosition(*pl)
                       : sh != 0
                       ? gi->getShipPosition(*sh)
                       : afl::base::Nothing)
                    : afl::base::Nothing);
}

inline game::proxy::SimulationSetupProxy::Slot_t
game::proxy::SimulationSetupProxy::Trampoline::addPlanet(ListItems_t& list)
{
    Setup& setup = getSetup();
    setup.addPlanet();
    updatePlanetName(setup);
    Slot_t result = setup.getNumShips();

    notifyListeners(true);
    packList(list);
    return result;
}

inline game::proxy::SimulationSetupProxy::Slot_t
game::proxy::SimulationSetupProxy::Trampoline::addShip(Slot_t slot, int count, ListItems_t& list)
{
    // ex WSimListWithHandler::addShip (part)
    Slot_t result = 0;
    const GameInterface* gi = getGameInterface();
    Setup& setup = getSetup();
    const ShipList* shipList = getShipList();
    if (shipList != 0) {
        // Create first ship
        if (slot >= setup.getNumShips()) {
            Id_t id = setup.findUnusedShipId(1, gi);
            if (Ship* sh = setup.addShip()) {
                // Success
                slot = setup.getNumShips() - 1;
                sh->setHullType(0, *shipList);
                sh->setId(id);
                if (int n = shipList->beams().size()) {
                    sh->setBeamType(n);
                    sh->setNumBeams(1);
                }
                sh->setAggressiveness(Ship::agg_Kill);
                sh->setDefaultName(m_translator);
                --count;
            } else {
                // Fail-safe (cannot happen)
                slot = 0;
                count = 0;
            }
        }

        // Create further ships
        setup.replicateShip(slot, count, gi, m_translator);

        // Place cursor on last created ship
        result = slot + count;

        // Adjust observed ship
        if (Slot_t* p = m_observedSlot.get()) {
            if (*p > slot) {
                *p += count;
            }
        }
    }

    notifyListeners(true);
    packList(list);
    return result;
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::swapShips(Slot_t a, Slot_t b)
{
    Setup& setup = getSetup();
    if (a < setup.getNumShips() && b < setup.getNumShips()) {
        // Do it
        setup.swapShips(a, b);

        // Adjust observed ship
        if (Slot_t* p = m_observedSlot.get()) {
            if (*p == a) {
                *p = b;
            } else if (*p == b) {
                *p = a;
            }
        }
    }
    notifyListeners(false);
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::removeObject(Slot_t slot)
{
    Setup& setup = getSetup();
    if (slot < setup.getNumShips()) {
        setup.removeShip(slot);
    } else {
        setup.removePlanet();
    }

    // Adjust observed ship
    if (Slot_t* p = m_observedSlot.get()) {
        if (*p == slot) {
            m_observedSlot = afl::base::Nothing;
        } else if (*p > slot) {
            --*p;
        }
    }

    notifyListeners(false);
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::clear()
{
    getSetup() = Setup();
    m_observedSlot = afl::base::Nothing;
    notifyListeners(false);
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::sortShips(SortOrder order)
{
    // Save observed object if that is a ship
    Ship* observedObject = 0;
    if (const Slot_t* p = m_observedSlot.get()) {
        observedObject = m_sim->setup().getShip(*p);
    }

    // Determine sort order
    int (*fcn)(const Ship&, const Ship&) = 0;
    switch (order) {
     case SortById:
        fcn = game::sim::compareId;
        break;
     case SortByOwner:
        fcn = game::sim::compareOwner;
        break;
     case SortByHull:
        fcn = game::sim::compareHull;
        break;
     case SortByBattleOrder:
        fcn = (m_sim->configuration().hasAlternativeCombat() ? game::sim::compareBattleOrderPHost : game::sim::compareBattleOrderHost);
        break;
     case SortByName:
        fcn = game::sim::compareName;
        break;
    }
    if (fcn) {
        m_sim->setup().sortShips(fcn);
    }

    // If we were observing an object, re-observe that
    Slot_t newPosition;
    if (observedObject != 0 && m_sim->setup().findIndex(observedObject, newPosition)) {
        m_observedSlot = newPosition;
    }

    notifyListeners(false);
}

inline Setup::Status
game::proxy::SimulationSetupProxy::Trampoline::copyToGame(Slot_t from, Slot_t to)
{
    const Setup& setup = getSetup();
    GameInterface* gi = getGameInterface();

    Setup::Status result(0, 0);
    if (gi != 0) {
        result = setup.copyToGame(*gi, from, to);
    }

    return result;
}

inline Setup::Status
game::proxy::SimulationSetupProxy::Trampoline::copyFromGame(Slot_t from, Slot_t to)
{
    Setup& setup = getSetup();
    const GameInterface* gi = getGameInterface();

    Setup::Status result(0, 0);
    if (gi != 0) {
        result = setup.copyFromGame(*gi, from, to);

        // This may have updated names, so assume structure change
        m_structureChanged = true;
        notifyListeners(false);
    }

    return result;
}

inline bool
game::proxy::SimulationSetupProxy::Trampoline::load(String_t fileName, String_t& errorMessage)
{
    // ex WSimScreen::loadFile (part)
    Setup& setup = getSetup();
    if (m_root.get() != 0) {
        try {
            // Open file
            afl::base::Ref<afl::io::Stream> stream = m_fileSystem.openFile(fileName, afl::io::FileSystem::OpenRead);

            // Load
            game::sim::Setup newSetup;
            game::sim::Loader(m_root->charset(), m_translator).load(*stream, newSetup);

            setup = newSetup;
            m_observedSlot = afl::base::Nothing;
            notifyListeners(false);
            return true;
        }
        catch (afl::except::FileProblemException& e) {
            errorMessage = Format("%s: %s", e.getFileName(), e.what());
            return false;
        }
        catch (std::exception& e) {
            errorMessage = e.what();
            return false;
        }
    } else {
        errorMessage = "<uninitialized>";
        return false;
    }
}

inline bool
game::proxy::SimulationSetupProxy::Trampoline::save(String_t fileName, String_t& errorMessage)
{
    // ex WSimScreen::saveFile (part)
    if (m_root.get() != 0) {
        try {
            afl::base::Ref<afl::io::Stream> stream = m_fileSystem.openFile(fileName, afl::io::FileSystem::Create);
            game::sim::Loader(m_root->charset(), m_translator).save(*stream, getSetup());
            return true;
        }
        catch (afl::except::FileProblemException& e) {
            errorMessage = Format("%s: %s", e.getFileName(), e.what());
            return false;
        }
        catch (std::exception& e) {
            errorMessage = e.what();
            return false;
        }
    } else {
        errorMessage = "<uninitialized>";
        return false;
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::setSlot(Slot_t slot)
{
    m_observedSlot = slot;
    sendObjectChange();
}

inline bool
game::proxy::SimulationSetupProxy::Trampoline::getObject(Slot_t slot, ObjectInfo& info)
{
    const Setup& setup = getSetup();
    if (const Object* obj = setup.getObject(slot)) {
        packObject(info, *obj, m_translator);
        return true;
    }
    return false;
}

inline bool
game::proxy::SimulationSetupProxy::Trampoline::isDuplicateId(Slot_t slot, Id_t candidate) const
{
    bool result = false;
    const Setup& setup = getSetup();
    const Slot_t numShips = setup.getNumShips();
    if (slot < numShips) {
        for (Slot_t i = 0; i < numShips; ++i) {
            if (i != slot) {
                if (const Ship* sh = setup.getShip(i)) {
                    if (sh->getId() == candidate) {
                        result = true;
                        break;
                    }
                }
            }
        }
    }
    return result;
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::getNumBaseTorpedoes(Slot_t slot, Elements_t& result) const
{
    const Setup& setup = getSetup();
    if (m_shipList.get() != 0) {
        if (const Planet* pl = dynamic_cast<const Planet*>(setup.getObject(slot))) {
            int torpId = 1;
            while (const game::spec::TorpedoLauncher* t = m_shipList->launchers().get(torpId)) {
                result.push_back(Element_t(pl->getNumBaseTorpedoes(torpId), t->getName(m_shipList->componentNamer())));
                ++torpId;
            }
        }
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::setFlags(Slot_t slot, uint32_t keep, uint32_t toggle)
{
    Setup& setup = getSetup();
    if (Object* obj = setup.getObject(slot)) {
        uint32_t oldFlags = obj->getFlags();
        uint32_t newFlags = (obj->getFlags() & keep) ^ toggle;
        obj->setFlags(newFlags);

        uint32_t updateFlags = 0;
        if ((oldFlags ^ newFlags) & Object::fl_Deactivated) {
            updateFlags |= UpdateList;
        }

        if ((newFlags & Object::fl_Cloaked) != 0) {
            if (Ship* sh = dynamic_cast<Ship*>(obj)) {
                // Cannot cloak and be aggressive
                if (sh->getAggressiveness() == Ship::agg_Kill || sh->getAggressiveness() == Ship::agg_NoFuel) {
                    sh->setAggressiveness(Ship::agg_Passive);
                }

                // Cannot cloak and intercept
                sh->setInterceptId(0);
            }
        }

        update(setup, *obj, updateFlags);
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::setSequentialFriendlyCode(Slot_t slot)
{
    Setup& setup = getSetup();
    if (Object* obj = setup.getObject(slot)) {
        setup.setSequentialFriendlyCode(slot);
        update(setup, *obj, 0);
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::setId(Slot_t slot, Id_t id)
{
    Setup& setup = getSetup();
    if (Object* obj = setup.getObject(slot)) {
        if (Ship* sh = dynamic_cast<Ship*>(obj)) {
            // Changing the name of a ship: try to update the default name
            bool isDefault = sh->hasDefaultName(m_translator);
            sh->setId(id);
            if (isDefault) {
                sh->setDefaultName(m_translator);
            }
            update(setup, *obj, UpdateList);
        } else {
            // Not a ship, so probably a planet
            obj->setId(id);
            updatePlanetName(setup);
            update(setup, *obj, UpdateList);
        }
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::setHullType(Slot_t slot, int hullType, bool afterAdd)
{
    Setup& setup = getSetup();
    if (m_shipList.get() != 0 && m_root.get() != 0) {
        if (Ship* sh = dynamic_cast<Ship*>(setup.getObject(slot))) {
            // Must update the list because it contains the hull type
            uint32_t updateFlags = UpdateList;

            // Set hull type
            int oldValue = sh->getHullType();
            sh->setHullType(hullType, *m_shipList);

            // Default owner
            if (hullType != oldValue && hullType != 0 && afterAdd) {
                // ex ccsim.pas:SetDefaultOwner
                // Pick default owner if possible
                int defaultOwner = 0;
                bool ownerCanBuildIt = false;
                for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
                    if (m_shipList->hullAssignments().getIndexFromHull(m_root->hostConfiguration(), pl, hullType) != 0) {
                        if (defaultOwner == 0) {
                            defaultOwner = pl;
                        } else {
                            defaultOwner = -1;
                        }
                        if (pl == sh->getOwner()) {
                            ownerCanBuildIt = true;
                        }
                    }
                }
                if (defaultOwner > 0) {
                    // Hull has a default owner.
                    // Set it (unless it already is) and make it aggressive.
                    if (defaultOwner != sh->getOwner()) {
                        sh->setOwner(defaultOwner);
                        sh->setFriendlyCode("?""?""?");
                        updateFlags |= UpdateOwner;
                    }
                } else {
                    // No default owner.
                    // If it's foreign to its current owner, user will most likely change the owner soon.
                    // Clear friendly code to avoid accidental inter-race copy.
                    if (!ownerCanBuildIt) {
                        sh->setFriendlyCode("?""?""?");
                    }
                }

                // Uncloak if it cannot cloak
                if (!sh->isCustomShip()
                    && !sh->hasImpliedFunction(BasicHullFunction::Cloak, *m_shipList, m_root->hostConfiguration())
                    && !sh->hasImpliedFunction(BasicHullFunction::AdvancedCloak, *m_shipList, m_root->hostConfiguration())
                    && !sh->hasImpliedFunction(BasicHullFunction::HardenedCloak, *m_shipList, m_root->hostConfiguration()))
                {
                    sh->setFlags(sh->getFlags() & ~Ship::fl_Cloaked);
                }
            }

            update(setup, *sh, updateFlags);
        }
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::setNumBaseTorpedoes(Slot_t slot, const Elements_t& list)
{
    Setup& setup = getSetup();
    if (Planet* pl = dynamic_cast<Planet*>(setup.getObject(slot))) {
        int torpId = 1;
        for (size_t i = 0, n = list.size(); i < n; ++i) {
            pl->setNumBaseTorpedoes(torpId, list[i].first);
            ++torpId;
        }
        update(setup, *pl, 0);
    }
}

template<typename Object, typename Property>
inline void
game::proxy::SimulationSetupProxy::Trampoline::setProperty(Slot_t slot, void (Object::*set)(Property), Property value, uint32_t updateFlags)
{
    Setup& setup = getSetup();
    if (Object* obj = dynamic_cast<Object*>(setup.getObject(slot))) {
        (obj->*set)(value);
        update(setup, *obj, updateFlags);
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::getAbilityChoices(Slot_t slot, AbilityChoices& result)
{
    const Setup& setup = getSetup();
    const Object* obj = setup.getObject(slot);

    if (obj != 0 && m_root.get() != 0 && m_shipList.get() != 0) {
        // Available abilities
        if (dynamic_cast<const Ship*>(obj) != 0) {
            result.available = game::sim::Abilities_t::allUpTo(game::sim::LAST_ABILITY);
        } else {
            result.available = game::sim::Abilities_t() + game::sim::TripleBeamKillAbility
                + game::sim::DoubleBeamChargeAbility
                + game::sim::DoubleTorpedoChargeAbility;
        }

        // Set/active/implied
        for (int i = game::sim::FIRST_ABILITY; i <= game::sim::LAST_ABILITY; ++i) {
            game::sim::Ability a = game::sim::Ability(i);
            if (obj->hasImpliedAbility(a, m_sim->configuration(), *m_shipList, m_root->hostConfiguration())) {
                result.implied += a;
            }

            Object::AbilityInfo info = Object::getAbilityInfo(a);
            if (obj->getFlags() & info.setBit) {
                result.set += a;
            }
            if (obj->getFlags() & info.activeBit) {
                result.active += a;
            }
        }
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::getFriendlyCodeChoices(Slot_t slot, game::spec::FriendlyCodeList::Infos_t& result)
{
    using game::spec::FriendlyCode;
    const Setup& setup = getSetup();
    const Object* obj = setup.getObject(slot);

    if (obj != 0 && m_shipList.get() != 0 && m_root.get() != 0) {
        // Determine matching mode
        FriendlyCode::FlagSet_t typeFlags;
        FriendlyCode::FlagSet_t propertyFlags;
        FriendlyCode::FlagSet_t propertyMask = FriendlyCode::FlagSet_t() + FriendlyCode::CapitalShipCode + FriendlyCode::AlchemyShipCode;
        if (const Ship* sh = dynamic_cast<const Ship*>(obj)) {
            typeFlags += FriendlyCode::ShipCode;
            if (sh->getNumBays() != 0 || sh->getNumLaunchers() != 0 || sh->getNumBeams() != 0) {
                propertyFlags += FriendlyCode::CapitalShipCode;
            }
            // FIXME: alchemy (registered?)
        }
        if (const Planet* pl = dynamic_cast<const Planet*>(obj)) {
            typeFlags += FriendlyCode::PlanetCode;
            if (pl->hasBase()) {
                typeFlags += FriendlyCode::StarbaseCode;
            }
        }

        const int player = obj->getOwner();

        // Build list
        // FIXME: similar code appears in IFUIInputFCode
        const game::spec::FriendlyCodeList& originalList = m_shipList->friendlyCodes();
        game::spec::FriendlyCodeList filteredList;
        for (game::spec::FriendlyCodeList::Iterator_t gi = originalList.begin(); gi != originalList.end(); ++gi) {
            FriendlyCode::FlagSet_t fcFlags = (*gi)->getFlags();
            if (!(fcFlags & typeFlags).empty()
                && ((fcFlags & propertyMask) - propertyFlags).empty()
                // && (!fcFlags.contains(FriendlyCode::RegisteredCode) || r.registrationKey().getStatus() == game::RegistrationKey::Registered)
                && (*gi)->getRaces().contains(player))
            {
                filteredList.addCode(**gi);
            }
        }

        // Build output
        filteredList.pack(result, m_root->playerList(), m_translator);
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::getOwnerChoices(Elements_t& result)
{
    if (m_root.get() != 0) {
        const PlayerList& pl = m_root->playerList();
        for (Player* p = pl.getFirstPlayer(); p != 0; p = pl.getNextPlayer(p)) {
            result.push_back(describeOwner(p->getId(), *m_root, m_translator));
        }
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::getExperienceLevelChoices(Elements_t& result)
{
    // ex editExperienceLevel (part)
    if (m_root.get() != 0) {
        const HostConfiguration& config = m_root->hostConfiguration();
        for (int i = 0, n = config[HostConfiguration::NumExperienceLevels](); i <= n; ++i) {
            result.push_back(describeExperienceLevel(i, *m_root, m_translator));
        }
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::getHullTypeChoices(Elements_t& result)
{
    // ex WSimListWithHandler::editType (part)
    if (m_shipList.get() != 0) {
        result.push_back(describeHull(0, *m_shipList, m_translator));
        const game::spec::HullVector_t& vec = m_shipList->hulls();
        for (const game::spec::Hull* p = vec.findNext(0); p != 0; p = vec.findNext(p->getId())) {
            result.push_back(Element_t(p->getId(), p->getName(m_shipList->componentNamer())));
        }
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::getPrimaryChoices(Slot_t slot, PrimaryChoices& result)
{
    const Setup& setup = getSetup();
    if (m_shipList.get() != 0) {
        // Limit
        if (const Ship* sh = dynamic_cast<const Ship*>(setup.getObject(slot))) {
            if (const game::spec::Hull* h = m_shipList->hulls().get(sh->getHullType())) {
                result.numBeams = Range_t(0, h->getMaxBeams());
            } else {
                result.numBeams = Range_t(0, 20);
            }
        } else {
            result.numBeams = Range_t();
        }

        // Beam types
        afl::string::Translator& tx = m_translator;
        result.beamTypes.push_back(Element_t(0, tx("none")));
        const game::spec::BeamVector_t& vec = m_shipList->beams();
        for (const game::spec::Beam* p = vec.findNext(0); p != 0; p = vec.findNext(p->getId())) {
            result.beamTypes.push_back(Element_t(p->getId(),
                                                 Format(tx("%s\t(tech %d, K%d, D%d)"),
                                                        p->getName(m_shipList->componentNamer()),
                                                        p->getTechLevel(),
                                                        p->getKillPower(),
                                                        p->getDamagePower())));
        }
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::getSecondaryChoices(Slot_t slot, SecondaryChoices& result)
{
    const Setup& setup = getSetup();
    if (m_shipList.get() != 0) {
        // Limits
        if (const Ship* sh = dynamic_cast<const Ship*>(setup.getObject(slot))) {
            if (const game::spec::Hull* h = m_shipList->hulls().get(sh->getHullType())) {
                result.numLaunchers = Range_t(0, h->getMaxLaunchers());
                result.numBays      = Range_t::fromValue(h->getNumBays());
                result.ammo         = Range_t(0, h->getMaxCargo());
            } else {
                result.numLaunchers = Range_t(0, 20);
                result.numBays      = Range_t(0, 20);
                result.ammo         = Range_t(0, 10000);
            }
        } else {
            result.numLaunchers = Range_t();
            result.numBays      = Range_t();
            result.ammo         = Range_t();
        }

        // Power scaling
        const int powerScale = getTorpedoPowerScale();

        // Torpedo types
        afl::string::Translator& tx = m_translator;
        result.torpedoTypes.push_back(Element_t(0, tx("none")));
        const game::spec::TorpedoVector_t& vec = m_shipList->launchers();
        for (const game::spec::TorpedoLauncher* p = vec.findNext(0); p != 0; p = vec.findNext(p->getId())) {
            result.torpedoTypes.push_back(Element_t(p->getId(),
                                                    Format(tx("%s\t(tech %d, K%d, D%d)"),
                                                           p->getName(m_shipList->componentNamer()),
                                                           p->getTechLevel(),
                                                           powerScale * p->getKillPower(),
                                                           powerScale * p->getDamagePower())));
        }
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::getEngineTypeChoices(Elements_t& result)
{
    if (m_shipList.get() != 0) {
        const game::spec::EngineVector_t& vec = m_shipList->engines();
        for (const game::spec::Engine* p = vec.findNext(0); p != 0; p = vec.findNext(p->getId())) {
            result.push_back(Element_t(p->getId(), p->getName(m_shipList->componentNamer())));
        }
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::getAggressivenessChoices(Elements_t& result)
{
    // WSimListWithHandler::editAggressiveness (part)
    if (m_root.get() != 0) {
        afl::string::Translator& tx = m_translator;

        result.push_back(describeAggressiveness(Ship::agg_Kill, *m_root, tx));
        result.push_back(describeAggressiveness(Ship::agg_Passive, *m_root, tx));

        const PlayerList& pl = m_root->playerList();
        for (Player* p = pl.getFirstPlayer(); p != 0; p = pl.getNextPlayer(p)) {
            result.push_back(describeAggressiveness(p->getId(), *m_root, tx));
        }

        result.push_back(describeAggressiveness(Ship::agg_NoFuel, *m_root, tx));
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::getBaseBeamLevelChoices(Elements_t& result)
{
    if (m_shipList.get() != 0) {
        afl::string::Translator& tx = m_translator;
        result.push_back(Element_t(0, tx("No base")));

        const game::spec::BeamVector_t& vec = m_shipList->beams();
        for (const game::spec::Beam* p = vec.findNext(0); p != 0; p = vec.findNext(p->getId())) {
            result.push_back(Element_t(p->getId(), p->getName(m_shipList->componentNamer())));
        }
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::getBaseTorpedoLevelChoices(Elements_t& result)
{
    if (m_shipList.get() != 0) {
        const game::spec::TorpedoVector_t& vec = m_shipList->launchers();
        for (const game::spec::TorpedoLauncher* p = vec.findNext(0); p != 0; p = vec.findNext(p->getId())) {
            result.push_back(Element_t(p->getId(), p->getName(m_shipList->componentNamer())));
        }
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::getPlanetNameChoices(Elements_t& result)
{
    if (const GameInterface* gi = getGameInterface()) {
        for (Id_t i = 1, limit = gi->getMaxPlanetId(); i <= limit; ++i) {
            String_t name = gi->getPlanetName(i);
            if (!name.empty()) {
                result.push_back(Element_t(i, name));
            }
        }
    }
}

inline void
game::proxy::SimulationSetupProxy::Trampoline::getPopulationChoices(Slot_t slot, PopulationChoices& result)
{
    const Setup& setup = getSetup();
    result = PopulationChoices();

    const Planet* pl = dynamic_cast<const Planet*>(setup.getObject(slot));
    if (pl != 0) {
        result.population = getPopulationFromDefense(pl->getDefense());
        result.sampleDefense = pl->getDefense() < 10 ? 60 : pl->getDefense();
        result.samplePopulation = getPopulationFromDefense(result.sampleDefense);
        result.range = Range_t(0, 250000);
    }
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::Trampoline::getIdRange(Slot_t slot)
{
    const GameInterface* gi = getGameInterface();
    const Setup& setup = getSetup();
    if (gi != 0) {
        const Object* obj = setup.getObject(slot);
        if (dynamic_cast<const Ship*>(obj) != 0) {
            return Range_t(1, gi->getMaxShipId());
        }
        if (dynamic_cast<const Planet*>(obj) != 0) {
            return Range_t(1, gi->getMaxPlanetId());
        }
    }
    return Range_t();
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::Trampoline::getDamageRange(Slot_t slot)
{
    const Setup& setup = getSetup();
    if (m_root.get() != 0) {
        if (const Object* obj = setup.getObject(slot)) {
            return Range_t(0, getMaxDamage(*obj, m_root->hostConfiguration()));
        }
    }
    return Range_t(0, 99);
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::Trampoline::getShieldRange(Slot_t slot)
{
    const Setup& setup = getSetup();
    if (m_root.get() != 0) {
        if (const Object* obj = setup.getObject(slot)) {
            return Range_t(0, getMaxShield(*obj, m_root->hostConfiguration()));
        }
    }
    return Range_t(0, 100);
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::Trampoline::getCrewRange(Slot_t slot)
{
    const Setup& setup = getSetup();
    if (m_shipList.get() != 0) {
        if (const Ship* obj = dynamic_cast<const Ship*>(setup.getObject(slot))) {
            if (const game::spec::Hull* h = m_shipList->hulls().get(obj->getHullType())) {
                // All ships are allowed to have 10 crew because Host sometimes sets it so.
                return Range_t(1, std::max(10, h->getMaxCrew()));
            } else {
                return Range_t(1, 10000);
            }
        }
    }
    return Range_t();
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::Trampoline::getInterceptIdRange(Slot_t /*slot*/)
{
    if (const GameInterface* gi = getGameInterface()) {
        return Range_t(0, gi->getMaxShipId());
    } else {
        return Range_t();
    }
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::Trampoline::getBaseDefenseRange(Slot_t slot)
{
    const Setup& setup = getSetup();
    if (m_root.get() != 0) {
        if (const Object* obj = setup.getObject(slot)) {
            return Range_t(0, m_root->hostConfiguration()[HostConfiguration::MaximumDefenseOnBase](obj->getOwner()));
        }
    }
    return Range_t();
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::Trampoline::getNumBaseFightersRange(Slot_t slot)
{
    const Setup& setup = getSetup();
    if (m_root.get() != 0) {
        if (const Object* obj = setup.getObject(slot)) {
            return Range_t(0, m_root->hostConfiguration()[HostConfiguration::MaximumFightersOnBase](obj->getOwner()));
        }
    }
    return Range_t();
}

void
game::proxy::SimulationSetupProxy::Trampoline::getConfiguration(Configuration& config)
{
    config = m_sim->configuration();
}

void
game::proxy::SimulationSetupProxy::Trampoline::setConfiguration(const Configuration& config, Configuration::Areas_t areas)
{
    m_sim->configuration().copyFrom(config, areas);
}

inline Setup&
game::proxy::SimulationSetupProxy::Trampoline::getSetup() const
{
    return m_sim->setup();
}

inline GameInterface*
game::proxy::SimulationSetupProxy::Trampoline::getGameInterface() const
{
    return m_sim->getGameInterface();
}

inline const ShipList*
game::proxy::SimulationSetupProxy::Trampoline::getShipList() const
{
    return m_shipList.get();
}

inline int
game::proxy::SimulationSetupProxy::Trampoline::getTorpedoPowerScale() const
{
    // AC enabled?
    bool hasAlternativeCombat = m_root.get() != 0 && m_root->hostConfiguration()[HostConfiguration::AllowAlternativeCombat]() != 0;

    // AC honored by combat algo?
    bool honorsAlternativeCombat = m_sim->configuration().hasAlternativeCombat();

    return (hasAlternativeCombat && honorsAlternativeCombat) ? 1 : 2;
}

void
game::proxy::SimulationSetupProxy::Trampoline::notifyListeners(bool blockList)
{
    bool prev = m_suppressStructureChanges;
    m_suppressStructureChanges = blockList;

    getSetup().notifyListeners();
    sendListChange();

    m_suppressStructureChanges = prev;
}

void
game::proxy::SimulationSetupProxy::Trampoline::sendListChange()
{
    class Task : public Reply_t {
     public:
        Task(Trampoline& tpl)
            : m_list()
            { tpl.packList(m_list); }
        virtual void handle(SimulationSetupProxy& proxy)
            { proxy.sig_listChange.raise(m_list); }
     private:
        ListItems_t m_list;
    };

    if (m_structureChanged && !m_suppressStructureChanges) {
        m_reply.postNewRequest(new Task(*this));
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::sendObjectChange()
{
    class Task : public Reply_t {
     public:
        Task(Trampoline& tpl, Slot_t slot, const Object& obj, afl::string::Translator& tx)
            : m_slot(slot), m_info()
            { tpl.packObject(m_info, obj, tx); }
        virtual void handle(SimulationSetupProxy& proxy)
            { proxy.sig_objectChange.raise(m_slot, m_info); }
     private:
        Slot_t m_slot;
        ObjectInfo m_info;
    };

    if (const Slot_t* p = m_observedSlot.get()) {
        const Setup& setup = getSetup();
        if (const Object* obj = setup.getObject(*p)) {
            m_reply.postNewRequest(new Task(*this, *p, *obj, m_translator));
        }
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::update(Setup& setup, Object& obj, uint32_t flags)
{
    if ((flags & UpdateList) != 0) {
        m_structureChanged = true;
    }
    if ((flags & UpdateAggressiveness) != 0) {
        if (Ship* sh = dynamic_cast<Ship*>(&obj)) {
            // ex WSimListWithHandler::editAggressiveness (part)
            if (sh->getAggressiveness() == Ship::agg_Kill || sh->getAggressiveness() == Ship::agg_NoFuel) {
                sh->setFlags(sh->getFlags() & ~Ship::fl_Cloaked);
                sh->setInterceptId(0);
            }
        }
    }
    if ((flags & UpdateOwner) != 0) {
        // For starbases, the owner limits the available defense/fighters
        if (Planet* pl = dynamic_cast<Planet*>(&obj)) {
            // ex WSimPlanetEditor::editOwner (part)
            if (pl->hasBase() && m_root.get() != 0) {
                pl->setNumBaseFighters(std::min(pl->getNumBaseFighters(), m_root->hostConfiguration()[HostConfiguration::MaximumFightersOnBase](pl->getOwner())));
                pl->setBaseDefense    (std::min(pl->getBaseDefense(),     m_root->hostConfiguration()[HostConfiguration::MaximumDefenseOnBase] (pl->getOwner())));
            }
        }

        // For ships, avoid targeting ourselves and limit damage
        if (Ship* sh = dynamic_cast<Ship*>(&obj)) {
            // ex ccsim.pas:SetShipOwner
            if (sh->getAggressiveness() == sh->getOwner()) {
                sh->setAggressiveness(Ship::agg_Kill);
                sh->setFlags(sh->getFlags() & ~Ship::fl_Cloaked);
            }
            if (m_root.get() != 0) {
                obj.setDamage(std::min(obj.getDamage(), getMaxDamage(obj, m_root->hostConfiguration())));
            }
        }
    }
    if ((flags & UpdateInterceptId) != 0) {
        if (Ship* sh = dynamic_cast<Ship*>(&obj)) {
            // ex WSimListWithHandler::editInterceptId
            if (sh->getInterceptId() != 0) {
                // Cannot cloak and intercept
                sh->setFlags(sh->getFlags() & ~Ship::fl_Cloaked);
                if (const Ship* target = setup.findShipById(sh->getInterceptId())) {
                    // Pick correct PE.
                    // FIXME: if we have a standing 'enemies' order, and a nonzero PE, we don't need to change here.
                    sh->setAggressiveness(target->getOwner());
                } else {
                    const GameInterface* gi = getGameInterface();
                    const int ownerInGame = (gi != 0 ? gi->getShipOwner(sh->getInterceptId()) : 0);
                    if (ownerInGame != 0) {
                        // We're intercepting a ship which is not in the simulation, but in the game. Set correct PE in case ship is added later.
                        sh->setAggressiveness(ownerInGame);
                    }
                }
                if (sh->getAggressiveness() == Ship::agg_Kill || sh->getAggressiveness() == Ship::agg_NoFuel) {
                    // Cannot intercept and kill, cannot intercept without fuel.
                    sh->setAggressiveness(Ship::agg_Passive);
                }
            }
        }
    }
    if ((flags & UpdateRandomFriendlyCode) != 0) {
        obj.setRandomFriendlyCodeFlags();
    }
    if ((flags & UpdateDamageShield) != 0) {
        if (m_root.get() != 0) {
            obj.setShield(std::min(obj.getShield(), getMaxShield(obj, m_root->hostConfiguration())));
        }
    }
    notifyListeners(false);
}

void
game::proxy::SimulationSetupProxy::Trampoline::updatePlanetName(Setup& setup)
{
    // ex simplanetedit.cc:setPlanetName
    if (Planet* p = setup.getPlanet()) {
        String_t name;
        if (const GameInterface* gi = getGameInterface()) {
            name = gi->getPlanetName(p->getId());
        }
        if (!name.empty()) {
            p->setName(name);
        }
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::onStructureChange()
{
    m_structureChanged = true;
    sendListChange();
}

void
game::proxy::SimulationSetupProxy::Trampoline::onPlanetChange()
{
    const Slot_t* p = m_observedSlot.get();
    const Setup& setup = getSetup();
    if (p != 0 && *p == setup.getNumShips()) {
        sendObjectChange();
    }
}

void
game::proxy::SimulationSetupProxy::Trampoline::onShipChange(Slot_t slot)
{
    const Slot_t* p = m_observedSlot.get();
    if (p != 0 && *p == slot) {
        sendObjectChange();
    }
}


/*
 *  SimulationSetupProxy
 */

game::proxy::SimulationSetupProxy::SimulationSetupProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_gameSender(gameSender),
      m_reply(reply, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

void
game::proxy::SimulationSetupProxy::getList(WaitIndicator& ind, ListItems_t& out)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(ListItems_t& result)
            : m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packList(m_result); }
     private:
        ListItems_t& m_result;
    };

    Task t(out);
    ind.call(m_trampoline, t);
}

game::proxy::SimulationSetupProxy::Slot_t
game::proxy::SimulationSetupProxy::addPlanet(WaitIndicator& ind)
{
    class Task : public Trampoline::Request_t {
     public:
        Task()
            : m_result(),
              m_list()
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.addPlanet(m_list); }
        Slot_t getResult() const
            { return m_result; }
        const ListItems_t& getList() const
            { return m_list; }
     private:
        Slot_t m_result;
        ListItems_t m_list;
    };

    Task t;
    ind.call(m_trampoline, t);
    sig_listChange.raise(t.getList());
    return t.getResult();
}

game::proxy::SimulationSetupProxy::Slot_t
game::proxy::SimulationSetupProxy::addShip(WaitIndicator& ind, Slot_t slot, int count)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, int count)
            : m_result(), m_slot(slot), m_count(count), m_list()
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.addShip(m_slot, m_count, m_list); }
        Slot_t getResult() const
            { return m_result; }
        const ListItems_t& getList() const
            { return m_list; }
     private:
        Slot_t m_result;
        Slot_t m_slot;
        int m_count;
        ListItems_t m_list;
    };

    Task t(slot, count);
    ind.call(m_trampoline, t);
    sig_listChange.raise(t.getList());
    return t.getResult();
}

void
game::proxy::SimulationSetupProxy::swapShips(Slot_t a, Slot_t b)
{
    m_trampoline.postRequest(&Trampoline::swapShips, a, b);
}

void
game::proxy::SimulationSetupProxy::removeObject(Slot_t slot)
{
    m_trampoline.postRequest(&Trampoline::removeObject, slot);
}

void
game::proxy::SimulationSetupProxy::clear()
{
    m_trampoline.postRequest(&Trampoline::clear);
}

void
game::proxy::SimulationSetupProxy::sortShips(SortOrder order)
{
    m_trampoline.postRequest(&Trampoline::sortShips, order);
}

game::sim::Setup::Status
game::proxy::SimulationSetupProxy::copyToGame(WaitIndicator& ind, Slot_t from, Slot_t to)
{
    return copyGame(ind, from, to, &Trampoline::copyToGame);
}

game::sim::Setup::Status
game::proxy::SimulationSetupProxy::copyFromGame(WaitIndicator& ind, Slot_t from, Slot_t to)
{
    return copyGame(ind, from, to, &Trampoline::copyFromGame);
}

bool
game::proxy::SimulationSetupProxy::load(WaitIndicator& ind, String_t fileName, String_t& errorMessage)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(String_t fileName, String_t& errorMessage)
            : m_fileName(fileName), m_errorMessage(errorMessage), m_result(false)
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.load(m_fileName, m_errorMessage); }
        bool getResult() const
            { return m_result; }
     private:
        String_t m_fileName;
        String_t& m_errorMessage;
        bool m_result;
    };

    Task t(fileName, errorMessage);
    ind.call(m_trampoline, t);
    return t.getResult();
}

bool
game::proxy::SimulationSetupProxy::save(WaitIndicator& ind, String_t fileName, String_t& errorMessage)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(String_t fileName, String_t& errorMessage)
            : m_fileName(fileName), m_errorMessage(errorMessage), m_result(false)
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.save(m_fileName, m_errorMessage); }
        bool getResult() const
            { return m_result; }
     private:
        String_t m_fileName;
        String_t& m_errorMessage;
        bool m_result;
    };

    Task t(fileName, errorMessage);
    ind.call(m_trampoline, t);
    return t.getResult();
}

void
game::proxy::SimulationSetupProxy::setSlot(Slot_t slot)
{
    m_trampoline.postRequest(&Trampoline::setSlot, slot);
}

bool
game::proxy::SimulationSetupProxy::getObject(WaitIndicator& ind, Slot_t slot, ObjectInfo& info)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, ObjectInfo& info)
            : m_slot(slot), m_info(info), m_result(false)
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.getObject(m_slot, m_info); }
        bool getResult() const
            { return m_result; }
     private:
        Slot_t m_slot;
        ObjectInfo& m_info;
        bool m_result;
    };

    Task t(slot, info);
    ind.call(m_trampoline, t);
    return t.getResult();
}

bool
game::proxy::SimulationSetupProxy::isDuplicateId(WaitIndicator& ind, Slot_t slot, Id_t candidate)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, Id_t candidate)
            : m_slot(slot), m_candidate(candidate), m_result()
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.isDuplicateId(m_slot, m_candidate); }
        bool getResult() const
            { return m_result; }
     private:
        Slot_t m_slot;
        Id_t m_candidate;
        bool m_result;
    };

    Task t(slot, candidate);
    ind.call(m_trampoline, t);
    return t.getResult();
}

void
game::proxy::SimulationSetupProxy::getNumBaseTorpedoes(WaitIndicator& ind, Slot_t slot, Elements_t& result)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, Elements_t& result)
            : m_slot(slot), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getNumBaseTorpedoes(m_slot, m_result); }
     private:
        Slot_t m_slot;
        Elements_t& m_result;
    };

    Task t(slot, result);
    ind.call(m_trampoline, t);
}

void
game::proxy::SimulationSetupProxy::setFlags(Slot_t slot, uint32_t keep, uint32_t toggle)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, uint32_t keep, uint32_t toggle)
            : m_slot(slot), m_keep(keep), m_toggle(toggle)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.setFlags(m_slot, m_keep, m_toggle); }
     private:
        Slot_t m_slot;
        uint32_t m_keep;
        uint32_t m_toggle;
    };
    m_trampoline.postNewRequest(new Task(slot, keep, toggle));
}

void
game::proxy::SimulationSetupProxy::toggleDisabled(Slot_t slot)
{
    // WSimListWithHandler::toggleEnabled
    setFlags(slot, -1U, Object::fl_Deactivated);
}

void
game::proxy::SimulationSetupProxy::toggleCloak(Slot_t slot)
{
    // WSimListWithHandler::toggleCloak
    setFlags(slot, -1U, Object::fl_Cloaked);
}

void
game::proxy::SimulationSetupProxy::toggleRandomFriendlyCode(Slot_t slot)
{
    // WSimListWithHandler::toggleRandomFC
    setFlags(slot, -1U, Object::fl_RandomFC);
}

void
game::proxy::SimulationSetupProxy::setAbilities(Slot_t slot, const AbilityChoices& choices)
{
    uint32_t set = 0;
    uint32_t toggle = 0;
    for (int i = game::sim::FIRST_ABILITY; i <= game::sim::LAST_ABILITY; ++i) {
        game::sim::Ability a = game::sim::Ability(i);
        if (choices.available.contains(a)) {
            Object::AbilityInfo info = Object::getAbilityInfo(a);
            set += info.setBit;
            set += info.activeBit;
            if (choices.set.contains(a)) {
                toggle += info.setBit;
            }
            if (choices.active.contains(a)) {
                toggle += info.activeBit;
            }
        }
    }
    setFlags(slot, ~set, toggle);
}

void
game::proxy::SimulationSetupProxy::setSequentialFriendlyCode(Slot_t slot)
{
    m_trampoline.postRequest(&Trampoline::setSequentialFriendlyCode, slot);
}

void
game::proxy::SimulationSetupProxy::setId(Slot_t slot, int id)
{
    m_trampoline.postRequest(&Trampoline::setId, slot, id);
}

void
game::proxy::SimulationSetupProxy::setName(Slot_t slot, String_t name)
{
    setProperty(slot, &Object::setName, name, UpdateList);
}

void
game::proxy::SimulationSetupProxy::setFriendlyCode(Slot_t slot, String_t fcode)
{
    setProperty(slot, &Object::setFriendlyCode, fcode, UpdateRandomFriendlyCode);
}

void
game::proxy::SimulationSetupProxy::setDamage(Slot_t slot, int damage)
{
    setProperty(slot, &Object::setDamage, damage, UpdateDamageShield);
}

void
game::proxy::SimulationSetupProxy::setShield(Slot_t slot, int shield)
{
    setProperty(slot, &Object::setShield, shield, 0);
}

void
game::proxy::SimulationSetupProxy::setOwner(Slot_t slot, int owner)
{
    setProperty(slot, &Object::setOwner, owner, UpdateList | UpdateOwner | UpdateDamageShield);
}

void
game::proxy::SimulationSetupProxy::setExperienceLevel(Slot_t slot, int level)
{
    setProperty(slot, &Object::setExperienceLevel, level, 0);
}

void
game::proxy::SimulationSetupProxy::setFlakRatingOverride(Slot_t slot, int32_t r)
{
    setProperty(slot, &Object::setFlakRatingOverride, r, 0);
}

void
game::proxy::SimulationSetupProxy::setFlakCompensationOverride(Slot_t slot, int r)
{
    setProperty(slot, &Object::setFlakCompensationOverride, r, 0);
}

void
game::proxy::SimulationSetupProxy::setCrew(Slot_t slot, int crew)
{
    setProperty(slot, &Ship::setCrew, crew, 0);
}

void
game::proxy::SimulationSetupProxy::setHullType(Slot_t slot, int hullType, bool afterAdd)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, int hullType, bool afterAdd)
            : m_slot(slot), m_hullType(hullType), m_afterAdd(afterAdd)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.setHullType(m_slot, m_hullType, m_afterAdd); }
     private:
        Slot_t m_slot;
        int m_hullType;
        bool m_afterAdd;
    };
    m_trampoline.postNewRequest(new Task(slot, hullType, afterAdd));
}

void
game::proxy::SimulationSetupProxy::setMass(Slot_t slot, int mass)
{
    setProperty(slot, &Ship::setMass, mass, 0);
}

void
game::proxy::SimulationSetupProxy::setBeamType(Slot_t slot, int beamType)
{
    setProperty(slot, &Ship::setBeamType, beamType, 0);
}

void
game::proxy::SimulationSetupProxy::setNumBeams(Slot_t slot, int numBeams)
{
    setProperty(slot, &Ship::setNumBeams, numBeams, 0);
}

void
game::proxy::SimulationSetupProxy::setTorpedoType(Slot_t slot, int torpedoType)
{
    setProperty(slot, &Ship::setTorpedoType, torpedoType, 0);
}

void
game::proxy::SimulationSetupProxy::setNumLaunchers(Slot_t slot, int numLaunchers)
{
    setProperty(slot, &Ship::setNumLaunchers, numLaunchers, 0);
}

void
game::proxy::SimulationSetupProxy::setNumBays(Slot_t slot, int numBays)
{
    setProperty(slot, &Ship::setNumBays, numBays, 0);
}

void
game::proxy::SimulationSetupProxy::setAmmo(Slot_t slot, int ammo)
{
    setProperty(slot, &Ship::setAmmo, ammo, 0);
}

void
game::proxy::SimulationSetupProxy::setEngineType(Slot_t slot, int engineType)
{
    setProperty(slot, &Ship::setEngineType, engineType, 0);
}

void
game::proxy::SimulationSetupProxy::setAggressiveness(Slot_t slot, int aggressiveness)
{
    setProperty(slot, &Ship::setAggressiveness, aggressiveness, UpdateAggressiveness);
}

void
game::proxy::SimulationSetupProxy::setInterceptId(Slot_t slot, int id)
{
    setProperty(slot, &Ship::setInterceptId, id, UpdateInterceptId);
}

void
game::proxy::SimulationSetupProxy::setDefense(Slot_t slot, int defense)
{
    setProperty(slot, &Planet::setDefense, defense, 0);
}

void
game::proxy::SimulationSetupProxy::setPopulation(Slot_t slot, int32_t pop)
{
    setDefense(slot, getDefenseFromPopulation(pop));
}

void
game::proxy::SimulationSetupProxy::setBaseDefense(Slot_t slot, int defense)
{
    setProperty(slot, &Planet::setBaseDefense, defense, 0);
}

void
game::proxy::SimulationSetupProxy::setBaseBeamTech(Slot_t slot, int level)
{
    setProperty(slot, &Planet::setBaseBeamTech, level, UpdateList);
}

void
game::proxy::SimulationSetupProxy::setBaseTorpedoTech(Slot_t slot, int level)
{
    setProperty(slot, &Planet::setBaseTorpedoTech, level, 0);
}

void
game::proxy::SimulationSetupProxy::setNumBaseFighters(Slot_t slot, int baseFighters)
{
    setProperty(slot, &Planet::setNumBaseFighters, baseFighters, 0);
}

void
game::proxy::SimulationSetupProxy::setNumBaseTorpedoes(Slot_t slot, const Elements_t& list)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, const Elements_t& list)
            : m_slot(slot), m_list(list)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.setNumBaseTorpedoes(m_slot, m_list); }
     private:
        Slot_t m_slot;
        Elements_t m_list;
    };
    m_trampoline.postNewRequest(new Task(slot, list));
}

void
game::proxy::SimulationSetupProxy::getAbilityChoices(WaitIndicator& ind, Slot_t slot, AbilityChoices& result)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, AbilityChoices& result)
            : m_slot(slot), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getAbilityChoices(m_slot, m_result); }
     private:
        Slot_t m_slot;
        AbilityChoices& m_result;
    };

    Task t(slot, result);
    ind.call(m_trampoline, t);
}

void
game::proxy::SimulationSetupProxy::getFriendlyCodeChoices(WaitIndicator& ind, Slot_t slot, game::spec::FriendlyCodeList::Infos_t& result)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, game::spec::FriendlyCodeList::Infos_t& result)
            : m_slot(slot), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getFriendlyCodeChoices(m_slot, m_result); }
     private:
        Slot_t m_slot;
        game::spec::FriendlyCodeList::Infos_t& m_result;
    };

    Task t(slot, result);
    ind.call(m_trampoline, t);
}

void
game::proxy::SimulationSetupProxy::getOwnerChoices(WaitIndicator& ind, Elements_t& result)
{
    getChoices(ind, &Trampoline::getOwnerChoices, result);
}

void
game::proxy::SimulationSetupProxy::getExperienceLevelChoices(WaitIndicator& ind, Elements_t& result)
{
    getChoices(ind, &Trampoline::getExperienceLevelChoices, result);
}

void
game::proxy::SimulationSetupProxy::getHullTypeChoices(WaitIndicator& ind, Elements_t& result)
{
    getChoices(ind, &Trampoline::getHullTypeChoices, result);
}

void
game::proxy::SimulationSetupProxy::getPrimaryChoices(WaitIndicator& ind, Slot_t slot, PrimaryChoices& result)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, PrimaryChoices& result)
            : m_slot(slot), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getPrimaryChoices(m_slot, m_result); }
     private:
        Slot_t m_slot;
        PrimaryChoices& m_result;
    };

    Task t(slot, result);
    ind.call(m_trampoline, t);
}

void
game::proxy::SimulationSetupProxy::getSecondaryChoices(WaitIndicator& ind, Slot_t slot, SecondaryChoices& result)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, SecondaryChoices& result)
            : m_slot(slot), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getSecondaryChoices(m_slot, m_result); }
     private:
        Slot_t m_slot;
        SecondaryChoices& m_result;
    };

    Task t(slot, result);
    ind.call(m_trampoline, t);
}

void
game::proxy::SimulationSetupProxy::getEngineTypeChoices(WaitIndicator& ind, Elements_t& result)
{
    getChoices(ind, &Trampoline::getEngineTypeChoices, result);
}

void
game::proxy::SimulationSetupProxy::getAggressivenessChoices(WaitIndicator& ind, Elements_t& result)
{
    getChoices(ind, &Trampoline::getAggressivenessChoices, result);
}

void
game::proxy::SimulationSetupProxy::getBaseBeamLevelChoices(WaitIndicator& ind, Elements_t& result)
{
    getChoices(ind, &Trampoline::getBaseBeamLevelChoices, result);
}

void
game::proxy::SimulationSetupProxy::getBaseTorpedoLevelChoices(WaitIndicator& ind, Elements_t& result)
{
    getChoices(ind, &Trampoline::getBaseTorpedoLevelChoices, result);
}

void
game::proxy::SimulationSetupProxy::getPlanetNameChoices(WaitIndicator& ind, Elements_t& result)
{
    getChoices(ind, &Trampoline::getPlanetNameChoices, result);
}

void
game::proxy::SimulationSetupProxy::getPopulationChoices(WaitIndicator& ind, Slot_t slot, PopulationChoices& result)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, PopulationChoices& result)
            : m_slot(slot), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getPopulationChoices(m_slot, m_result); }
     private:
        Slot_t m_slot;
        PopulationChoices& m_result;
    };

    Task t(slot, result);
    ind.call(m_trampoline, t);
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::getIdRange(WaitIndicator& ind, Slot_t slot)
{
    return getRange(ind, &Trampoline::getIdRange, slot);
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::getDamageRange(WaitIndicator& ind, Slot_t slot)
{
    return getRange(ind, &Trampoline::getDamageRange, slot);
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::getShieldRange(WaitIndicator& ind, Slot_t slot)
{
    return getRange(ind, &Trampoline::getShieldRange, slot);
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::getCrewRange(WaitIndicator& ind, Slot_t slot)
{
    return getRange(ind, &Trampoline::getCrewRange, slot);
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::getInterceptIdRange(WaitIndicator& ind, Slot_t slot)
{
    return getRange(ind, &Trampoline::getInterceptIdRange, slot);
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::getBaseDefenseRange(WaitIndicator& ind, Slot_t slot)
{
    return getRange(ind, &Trampoline::getBaseDefenseRange, slot);
}

game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::getNumBaseFightersRange(WaitIndicator& ind, Slot_t slot)
{
    return getRange(ind, &Trampoline::getNumBaseFightersRange, slot);
}

void
game::proxy::SimulationSetupProxy::getConfiguration(WaitIndicator& ind, game::sim::Configuration& config)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Configuration& config)
            : m_config(config)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getConfiguration(m_config); }
     private:
        Configuration& m_config;
    };

    Task t(config);
    ind.call(m_trampoline, t);
}

void
game::proxy::SimulationSetupProxy::setConfiguration(const game::sim::Configuration& config, game::sim::Configuration::Areas_t areas)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(const Configuration& config, Configuration::Areas_t areas)
            : m_config(config), m_areas(areas)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.setConfiguration(m_config, m_areas); }
     private:
        Configuration m_config;
        Configuration::Areas_t m_areas;
    };

    m_trampoline.postNewRequest(new Task(config, areas));
}

util::RequestSender<game::Session>
game::proxy::SimulationSetupProxy::gameSender()
{
    return m_gameSender;
}

template<typename Object, typename Property>
void
game::proxy::SimulationSetupProxy::setProperty(Slot_t slot, void (Object::*set)(Property), Property value, uint32_t updateFlags)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Slot_t slot, void (Object::*set)(Property), Property value, uint32_t updateFlags)
            : m_slot(slot), m_set(set), m_value(value), m_updateFlags(updateFlags)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.setProperty(m_slot, m_set, m_value, m_updateFlags); }
     private:
        Slot_t m_slot;
        void (Object::*m_set)(Property);
        Property m_value;
        uint32_t m_updateFlags;
    };
    m_trampoline.postNewRequest(new Task(slot, set, value, updateFlags));
}

template<typename Object, typename Result>
void
game::proxy::SimulationSetupProxy::getChoices(WaitIndicator& ind, void (Object::*get)(Result&), Result& result)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Result& result, void (Object::*get)(Result&))
            : m_result(result), m_get(get)
            { }
        virtual void handle(Trampoline& tpl)
            { (tpl.*m_get)(m_result); }
     private:
        Result& m_result;
        void (Object::*m_get)(Result&);
    };

    Task t(result, get);
    ind.call(m_trampoline, t);
}

template<typename Object>
game::proxy::SimulationSetupProxy::Range_t
game::proxy::SimulationSetupProxy::getRange(WaitIndicator& ind, Range_t (Object::*get)(Slot_t), Slot_t slot)
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Range_t (Object::*get)(Slot_t), Slot_t slot)
            : m_get(get), m_slot(slot), m_result()
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = (tpl.*m_get)(m_slot); }
        Range_t getResult() const
            { return m_result; }
     private:
        Range_t (Object::*m_get)(Slot_t);
        Slot_t m_slot;
        Range_t m_result;
    };

    Task t(get, slot);
    ind.call(m_trampoline, t);
    return t.getResult();
}

template<typename Object>
game::sim::Setup::Status
game::proxy::SimulationSetupProxy::copyGame(WaitIndicator& ind, Slot_t from, Slot_t to, game::sim::Setup::Status (Object::*copy)(Slot_t, Slot_t))
{
    class Task : public Trampoline::Request_t {
     public:
        Task(Setup::Status (Object::*copy)(Slot_t, Slot_t), Slot_t from, Slot_t to)
            : m_copy(copy), m_from(from), m_to(to), m_result(0, 0)
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = (tpl.*m_copy)(m_from, m_to); }
        Setup::Status getResult() const
            { return m_result; }
     private:
        Setup::Status (Object::*m_copy)(Slot_t, Slot_t);
        Slot_t m_from;
        Slot_t m_to;
        Setup::Status m_result;
    };

    Task t(copy, from, to);
    ind.call(m_trampoline, t);
    return t.getResult();
}
