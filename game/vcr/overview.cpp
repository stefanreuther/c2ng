/**
  *  \file game/vcr/overview.cpp
  *  \brief Class game::vcr::Overview
  */

#include "game/vcr/overview.hpp"
#include "afl/string/format.hpp"
#include "game/vcr/object.hpp"

namespace {
    struct SortGroups {
        bool operator()(const game::vcr::Overview::Item& a, const game::vcr::Overview::Item& b)
            {
                if (a.groupId == b.groupId) {
                    return a.sequence < b.sequence;
                } else {
                    return a.groupId < b.groupId;
                }
            }
    };

    void addAppearance(game::vcr::Overview::Appearance& app, size_t index, size_t side)
    {
        app.lastIn = index;
        app.lastAs = side;
        ++app.num;
    }

    String_t getObjectName(const game::vcr::Object& obj, afl::string::Translator& tx)
    {
        // ex WCombatDiagramWidget::getObjectName
        if (obj.getName().empty()) {
            if (obj.isPlanet()) {
                return afl::string::Format(tx("Planet #%d"), obj.getId());
            } else {
                return afl::string::Format(tx("Ship #%d"), obj.getId());
            }
        } else {
            if (obj.isPlanet()) {
                return afl::string::Format(tx("%s (planet #%d)"), obj.getName(), obj.getId());
            } else {
                return afl::string::Format(tx("%s (ship #%d)"), obj.getName(), obj.getId());
            }
        }
    }
}

game::vcr::Overview::Overview(Database& battles, const game::config::HostConfiguration& config, const game::spec::ShipList& shipList)
    : m_battles(battles),
      m_config(config),
      m_shipList(shipList),
      m_units(),
      m_groupCounter(0)
{
    for (size_t i = 0, n = battles.getNumBattles(); i < n; ++i) {
        if (Battle* b = battles.getBattle(i)) {
            addBattle(*b, i);
        }
    }
    finish();
}

game::vcr::Overview::~Overview()
{ }

void
game::vcr::Overview::buildDiagram(Diagram& out, const PlayerList& players, afl::string::Translator& tx) const
{
    packUnits(out.units, tx);
    packBattles(out.battles, players, tx);
}

void
game::vcr::Overview::buildScoreSummary(ScoreSummary& out)
{
    const size_t numBattles = m_battles.getNumBattles();
    out.players.clear();
    out.scores.setAll(Score());
    out.numBattles = numBattles;
    for (size_t battleNr = 0; battleNr < numBattles; ++battleNr) {
        if (Battle* b = m_battles.getBattle(battleNr)) {
            b->prepareResult(m_config, m_shipList, Battle::NeedCompleteResult);
            for (size_t i = 0, numObjects = b->getNumObjects(); i < numObjects; ++i) {
                if (const Object* obj = b->getObject(i, false)) {
                    int playerNr = obj->getOwner();
                    if (Score* thisScore = out.scores.at(playerNr)) {
                        if (b->computeScores(*thisScore, i, m_config, m_shipList)) {
                            out.players += playerNr;
                        }
                    }
                }
            }
        }
    }
}

void
game::vcr::Overview::packUnits(std::vector<Diagram::Unit>& units, afl::string::Translator& tx) const
{
    const size_t n = m_units.size();
    units.resize(n);
    for (size_t i = 0; i < n; ++i) {
        if (const Battle* b = m_battles.getBattle(m_units[i].appears.firstIn)) {
            if (const Object* obj = b->getObject(m_units[i].appears.firstAs, false)) {
                units[i].initialOwner = obj->getOwner();
                units[i].name = getObjectName(*obj, tx);
            }
        }
    }
}

void
game::vcr::Overview::packBattles(std::vector<Diagram::Battle>& out, const PlayerList& players, afl::string::Translator& tx) const
{
    const size_t n = m_battles.getNumBattles();
    out.resize(n);
    for (size_t i = 0; i < n; ++i) {
        if (Battle* b = m_battles.getBattle(i)) {
            // Header info
            Diagram::Battle& db = out[i];
            db.name = b->getDescription(players, tx);
            db.participants.clear();

            // Participants
            bool anyKill = false;
            bool anyResult = false;
            int captor = 0;
            b->prepareResult(m_config, m_shipList, Battle::NeedQuickOutcome);
            for (size_t pi = 0, np = b->getNumObjects(); pi < np; ++pi) {
                // Track status
                const int status = b->getOutcome(m_config, m_shipList, pi);
                if (status < 0) {
                    // killed
                    anyKill = true;
                    anyResult = true;
                } else if (status > 0) {
                    // captured
                    anyResult = true;
                    if (captor == 0 || captor == status) {
                        captor = status;
                    } else {
                        captor = -1;
                    }
                } else {
                    // survived
                }

                // Register status
                std::vector<Item>::iterator it = const_cast<Overview*>(this)->findObject(*b->getObject(pi, false));
                if (it != m_units.end()) {
                    db.participants.push_back(Diagram::Participant(it - m_units.begin(), status));
                }
            }

            // Status
            if (!anyResult) {
                db.status = 0;
            } else if (!anyKill && captor > 0) {
                db.status = captor;
            } else {
                db.status = -1;
            }
        }
    }
}

void
game::vcr::Overview::addBattle(Battle& b, size_t index)
{
    // ex WCombatDiagram::init, vcrplay.pas:InitializeTacticalDiagram (approx)

    // Compute result
    b.prepareResult(m_config, m_shipList, Battle::NeedQuickOutcome);

    // Assimilate first object and obtain a group Id
    const Object* leftObject = b.getObject(0, false);
    if (leftObject == 0) {
        // Error: battle has no first object. Ignore.
        return;
    }

    std::vector<Item>::iterator lptr = findObject(*leftObject);
    Id_t groupId;
    if (lptr != m_units.end()) {
        // already known, reuse group Id
        groupId = lptr->groupId;
        addAppearance(lptr->appears, index, 0);
    } else {
        // not known, allocate new group Id
        groupId = ++m_groupCounter;
        m_units.push_back(Item(leftObject->isPlanet(), leftObject->getId(), groupId, m_units.size(), Appearance(index, 0)));
    }

    // Assimilate other objects
    for (size_t side = 1, n = b.getNumObjects(); side < n; ++side) {
        if (const Object* rightObject = b.getObject(side, false)) {
            std::vector<Item>::iterator rptr = findObject(*rightObject);
            if (rptr != m_units.end()) {
                // already known, rename into earlier group
                if (rptr->groupId < groupId) {
                    renameGroup(groupId, rptr->groupId);
                    groupId = rptr->groupId;
                } else {
                    renameGroup(rptr->groupId, groupId);
                }
                addAppearance(rptr->appears, index, side);
            } else {
                // not known, put into this group */
                m_units.push_back(Item(rightObject->isPlanet(), rightObject->getId(), groupId, m_units.size(), Appearance(index, side)));
            }
        }
    }
}

void
game::vcr::Overview::finish()
{
    // Sort
    std::sort(m_units.begin(), m_units.end(), SortGroups());
}

/** Find an object in the diagram.
    \param obj Object to find
    \return iterator pointing to this object's line, end() if none */
std::vector<game::vcr::Overview::Item>::iterator
game::vcr::Overview::findObject(const Object& obj)
{
    // ex WCombatDiagram::findObject
    std::vector<Item>::iterator i = m_units.begin();
    while (i != m_units.end() && (i->planet != obj.isPlanet() || i->id != obj.getId())) {
        ++i;
    }
    return i;
}

/** Rename a group.
    \param from Replace this group Id...
    \param to ...by this one */
void
game::vcr::Overview::renameGroup(int from, int to)
{
    // ex WCombatDiagram::renameGroup
    for (std::vector<Item>::iterator i = m_units.begin(); i != m_units.end(); ++i) {
        if (i->groupId == from) {
            i->groupId = to;
        }
    }
}
