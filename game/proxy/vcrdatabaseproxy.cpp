/**
  *  \file game/proxy/vcrdatabaseproxy.cpp
  *  \brief Class game::proxy::VcrDatabaseProxy
  */

#include "game/proxy/vcrdatabaseproxy.hpp"
#include "afl/string/format.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/sim/transfer.hpp"
#include "game/vcr/battle.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/flak/database.hpp"
#include "game/vcr/object.hpp"

using afl::string::Format;

/*
 *  Trampoline
 *
 *  As of 20210315, this is not strictly required (doesn't keep any game-side state),
 *  but let's do it by the book for now.
 */

class game::proxy::VcrDatabaseProxy::Trampoline {
 public:
    Trampoline(VcrDatabaseAdaptor& adaptor, util::RequestSender<VcrDatabaseProxy> reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer);

    size_t getNumBattles();
    void requestData(size_t index);
    void requestSideInfo(size_t index, size_t side, bool setHull);
    void requestHullInfo(size_t index, size_t side, int hullType);

    AddResult addToSimulation(size_t index, size_t side, int hullType, bool after);

    void packStatus(Status& st);
    void packPlayerNames(PlayerArray<String_t>& result, Player::Name which);
    void copyTeamSettings(TeamSettings& result);

 private:
    VcrDatabaseAdaptor& m_adaptor;
    util::RequestSender<VcrDatabaseProxy> m_reply;
    std::auto_ptr<game::spec::info::PictureNamer> m_picNamer;
};

game::proxy::VcrDatabaseProxy::Trampoline::Trampoline(VcrDatabaseAdaptor& adaptor, util::RequestSender<VcrDatabaseProxy> reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
    : m_adaptor(adaptor),
      m_reply(reply),
      m_picNamer(picNamer)
{ }

inline size_t
game::proxy::VcrDatabaseProxy::Trampoline::getNumBattles()
{
    return m_adaptor.battles().getNumBattles();
}

void
game::proxy::VcrDatabaseProxy::Trampoline::requestData(size_t index)
{
    game::vcr::BattleInfo d;
    if (game::vcr::Battle* b = m_adaptor.battles().getBattle(index)) {
        const Root& root = m_adaptor.root();
        const game::spec::ShipList& shipList = m_adaptor.shipList();
        b->prepareResult(root.hostConfiguration(), shipList, game::vcr::Battle::NeedQuickOutcome);
        b->getBattleInfo(d, m_adaptor.getTeamSettings(), shipList, root, m_adaptor.translator());
    }

    m_reply.postRequest(&VcrDatabaseProxy::updateCurrentBattle, index, getNumBattles(), d);
    m_adaptor.setCurrentBattle(index);
}

void
game::proxy::VcrDatabaseProxy::Trampoline::requestSideInfo(size_t index, size_t side, bool setHull)
{
    // Environment
    const Root& root = m_adaptor.root();
    const game::spec::ShipList& shipList = m_adaptor.shipList();
    afl::string::Translator& tx = m_adaptor.translator();

    // Produce output
    SideInfo info;
    int firstHull = 0;
    if (game::vcr::Battle* b = m_adaptor.battles().getBattle(index)) {
        if (const game::vcr::Object* obj = b->getObject(side, false)) {
            // Name and header information
            info.name = obj->getName();
            info.subtitle = obj->getSubtitle(m_adaptor.getTeamSettings(), root, shipList, tx);
            info.isPlanet = obj->isPlanet();
            if (m_adaptor.isGameObject(*obj)) {
                info.reference = Reference(obj->isPlanet() ? Reference::Planet : Reference::Ship,
                                           obj->getId());
            }

            // Type choices
            if (obj->isPlanet()) {
                info.typeChoices.add(0, tx("Planet"));
            } else {
                const game::spec::HullVector_t& hulls = shipList.hulls();
                for (game::spec::Hull* p = hulls.findNext(0); p != 0; p = hulls.findNext(p->getId())) {
                    int id = p->getId();
                    if (obj->canBeHull(hulls, id)) {
                        info.typeChoices.add(id, p->getName(shipList.componentNamer()));
                        if (firstHull == 0) {
                            firstHull = id;
                        }
                    }
                }
                if (info.typeChoices.empty()) {
                    info.typeChoices.add(0, tx("Unknown ship type"));
                }
            }
        }
    }

    // Send data
    m_reply.postRequest(&VcrDatabaseProxy::updateSideInfo, info);
    if (setHull) {
        requestHullInfo(index, side, firstHull);
    }
}

void
game::proxy::VcrDatabaseProxy::Trampoline::requestHullInfo(size_t index, size_t side, int hullType)
{
    // Environment
    const Root& root = m_adaptor.root();
    const game::spec::ShipList& shipList = m_adaptor.shipList();
    afl::string::Translator& tx = m_adaptor.translator();

    // Produce output
    HullInfo info;
    if (game::vcr::Battle* b = m_adaptor.battles().getBattle(index)) {
        if (const game::vcr::Object* obj = b->getObject(side, false)) {
            // Image
            const game::spec::Hull* pHull = shipList.hulls().get(hullType);
            info.imageName = (m_picNamer.get() == 0
                              ? String_t()
                              : pHull != 0
                              ? m_picNamer->getHullPicture(*pHull)
                              : m_picNamer->getVcrObjectPicture(obj->isPlanet(), obj->getPicture()));

            // Description
            if (obj->isPlanet()) {
                info.planetInfo = game::vcr::PlanetInfo();
                describePlanet(*info.planetInfo.get(), *obj, root.hostConfiguration());
            } else {
                info.shipInfo = game::vcr::ShipInfo();
                describeShip(*info.shipInfo.get(), *obj, shipList, pHull,
                             b->isESBActive(root.hostConfiguration()), root.hostConfiguration(), tx,
                             root.userConfiguration().getNumberFormatter());

                if (pHull != 0) {
                    ShipQuery q;
                    q.setHullType(hullType);
                    q.setOwner(obj->getOwner());
                    q.setPlayerDisplaySet(PlayerSet_t(obj->getOwner()));
                    q.setLevelDisplaySet(ExperienceLevelSet_t(obj->getExperienceLevel()));
                    q.setCombatMass(obj->getMass(), 0);
                    q.setCrew(obj->getCrew());
                    q.setDamage(obj->getDamage());
                    info.shipQuery = q;
                }
            }
        }
    }

    // Send data
    m_reply.postRequest(&VcrDatabaseProxy::updateHullInfo, info);
}

game::proxy::VcrDatabaseProxy::AddResult
game::proxy::VcrDatabaseProxy::Trampoline::addToSimulation(size_t index, size_t side, int hullType, bool after)
{
    // ex WVcrInfoMain::addToSim
    const game::config::HostConfiguration& config = m_adaptor.root().hostConfiguration();
    const game::spec::ShipList& shipList = m_adaptor.shipList();

    // Obtain battle
    game::vcr::Battle* b = m_adaptor.battles().getBattle(index);
    if (b == 0) {
        return Error;
    }

    // Prepare result if needed
    if (after) {
        b->prepareResult(config, shipList, game::vcr::Battle::NeedQuickOutcome | game::vcr::Battle::NeedCompleteResult);
        if (b->getPlayability(config, shipList) != game::vcr::Battle::IsPlayable) {
            return NotPlayable;
        }
        if (b->getOutcome(config, shipList, side) < 0) {
            return UnitDied;
        }
    }

    // Fetch object
    const game::vcr::Object* obj = b->getObject(side, after);
    if (obj == 0) {
        return Error;
    }

    // Fetch simulation object
    game::sim::Setup* setup = m_adaptor.getSimulationSetup();
    if (setup == 0) {
        return Error;
    }

    // Perform transfer
    game::sim::BaseTransfer transfer(shipList, config, m_adaptor.translator());
    bool ok = false;
    if (obj->isPlanet()) {
        game::sim::Planet* pl = setup->addPlanet();
        if (pl != 0) {
            ok = transfer.copyPlanetFromBattle(*pl, *obj);
        }
    } else {
        game::sim::Ship* sh = setup->findShipById(obj->getId());
        if (sh == 0) {
            sh = setup->addShip();
        }
        if (sh != 0) {
            ok = transfer.copyShipFromBattle(*sh, *obj, hullType, b->isESBActive(config));
        }
    }
    setup->notifyListeners();
    return ok ? Success : NotParseable;
}

void
game::proxy::VcrDatabaseProxy::Trampoline::packStatus(Status& st)
{
    st.numBattles = getNumBattles();
    st.currentBattle = m_adaptor.getCurrentBattle();

    const game::vcr::Database& db = m_adaptor.battles();
    if (dynamic_cast<const game::vcr::classic::Database*>(&db) != 0) {
        st.kind = ClassicCombat;
    } else if (dynamic_cast<const game::vcr::flak::Database*>(&db) != 0) {
        st.kind = FlakCombat;
    } else {
        st.kind = UnknownCombat;
    }
}

inline void
game::proxy::VcrDatabaseProxy::Trampoline::packPlayerNames(PlayerArray<String_t>& result, Player::Name which)
{
    result = m_adaptor.root().playerList().getPlayerNames(which, m_adaptor.translator());
}

inline void
game::proxy::VcrDatabaseProxy::Trampoline::copyTeamSettings(TeamSettings& result)
{
    if (const TeamSettings* teams = m_adaptor.getTeamSettings()) {
        result.copyFrom(*teams);
    }
}

/*
 *  TrampolineFromAdaptor
 */

class game::proxy::VcrDatabaseProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(VcrDatabaseAdaptor&)> {
 public:
    TrampolineFromAdaptor(const util::RequestSender<VcrDatabaseProxy>& reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
        : m_reply(reply), m_picNamer(picNamer)
        { }
    virtual Trampoline* call(VcrDatabaseAdaptor& adaptor)
        { return new Trampoline(adaptor, m_reply, m_picNamer); }
 private:
    util::RequestSender<VcrDatabaseProxy> m_reply;
    std::auto_ptr<game::spec::info::PictureNamer> m_picNamer;
};


game::proxy::VcrDatabaseProxy::VcrDatabaseProxy(util::RequestSender<VcrDatabaseAdaptor> sender, util::RequestDispatcher& recv, afl::string::Translator& tx, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
    : m_reply(recv, *this),
      m_request(sender.makeTemporary(new TrampolineFromAdaptor(m_reply.getSender(), picNamer))),
      m_translator(tx),
      m_isActiveQuery(false),
      m_currentIndex(0),
      m_currentSide(0),
      m_numBattles()
{ }

game::proxy::VcrDatabaseProxy::~VcrDatabaseProxy()
{ }

void
game::proxy::VcrDatabaseProxy::getStatus(WaitIndicator& ind, Status& status)
{
    // Query status from game side
    class Task : public util::Request<Trampoline> {
     public:
        Task(Status& status)
            : m_status(status)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packStatus(m_status); }
     private:
        Status& m_status;
    };
    Task t(status);
    ind.call(m_request, t);

    // Save m_numBattles for UI-side rendering
    m_numBattles = status.numBattles;
}

void
game::proxy::VcrDatabaseProxy::getTeamSettings(WaitIndicator& ind, TeamSettings& teams)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(TeamSettings& result)
            : m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.copyTeamSettings(m_result); }
     private:
        TeamSettings& m_result;
    };

    // Two-step copy (game > tmp > teams) so if teams has listeners attached,
    // those are called in the right context.
    TeamSettings tmp;
    Task t(tmp);
    ind.call(m_request, t);
    teams.copyFrom(tmp);
}

game::PlayerArray<String_t>
game::proxy::VcrDatabaseProxy::getPlayerNames(WaitIndicator& ind, Player::Name which)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(PlayerArray<String_t>& result, Player::Name which)
            : m_result(result), m_which(which)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packPlayerNames(m_result, m_which); }
     private:
        PlayerArray<String_t>& m_result;
        Player::Name m_which;
    };

    PlayerArray<String_t> result;
    Task t(result, which);
    ind.call(m_request, t);
    return result;
}

void
game::proxy::VcrDatabaseProxy::setCurrentBattle(size_t index)
{
    m_currentIndex = index;
    if (!m_isActiveQuery) {
        m_isActiveQuery = true;
        m_request.postRequest(&Trampoline::requestData, index);
    } else {
        // Send updateTemporaryState() only when there is an active request.
        // This means we don't update with temporary data when game side answers fast enough (=almost always).
        // Essentially, this is el-cheapo flicker prevention.
        updateTemporaryState();
    }
}

void
game::proxy::VcrDatabaseProxy::setSide(size_t side, bool setHull)
{
    m_currentSide = side;
    m_request.postRequest(&Trampoline::requestSideInfo, m_currentIndex, side, setHull);
}

void
game::proxy::VcrDatabaseProxy::setHullType(int hullType)
{
    m_request.postRequest(&Trampoline::requestHullInfo, m_currentIndex, m_currentSide, hullType);
}

game::proxy::VcrDatabaseProxy::AddResult
game::proxy::VcrDatabaseProxy::addToSimulation(WaitIndicator& ind, int hullType, bool after)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(size_t index, size_t side, int hullType, bool after)
            : m_index(index), m_side(side), m_hullType(hullType), m_after(after), m_result(NotPlayable)
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.addToSimulation(m_index, m_side, m_hullType, m_after); }
        AddResult getResult() const
            { return m_result; }
     private:
        size_t m_index;
        size_t m_side;
        int m_hullType;
        bool m_after;
        AddResult m_result;
    };
    Task t(m_currentIndex, m_currentSide, hullType, after);
    ind.call(m_request, t);
    return t.getResult();
}

void
game::proxy::VcrDatabaseProxy::updateCurrentBattle(size_t index, size_t numBattles, game::vcr::BattleInfo data)
{
    if (index == m_currentIndex) {
        m_isActiveQuery = false;
        m_numBattles = numBattles;
        renderHeading(data, numBattles);
        sig_update.raise(index, data);
    } else {
        m_request.postRequest(&Trampoline::requestData, m_currentIndex);
    }
}

void
game::proxy::VcrDatabaseProxy::updateSideInfo(SideInfo info)
{
    sig_sideUpdate.raise(info);
}

void
game::proxy::VcrDatabaseProxy::updateHullInfo(HullInfo info)
{
    sig_hullUpdate.raise(info);
}

void
game::proxy::VcrDatabaseProxy::updateTemporaryState()
{
    game::vcr::BattleInfo d;
    if (const size_t* p = m_numBattles.get()) {
        renderHeading(d, *p);
    }
    sig_update.raise(m_currentIndex, d);
}

void
game::proxy::VcrDatabaseProxy::renderHeading(game::vcr::BattleInfo& data, size_t numBattles)
{
    data.heading = Format(m_translator("Battle %d of %d"), m_currentIndex+1, numBattles);
}

