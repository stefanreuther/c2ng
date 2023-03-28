/**
  *  \file game/vcr/test/battle.cpp
  *  \brief Class game::vcr::test::Battle
  */

#include "game/vcr/test/battle.hpp"
#include "game/vcr/object.hpp"

struct game::vcr::test::Battle::Info {
    Object before;
    Object after;
    int outcome;

    Info(const Object& obj, int outcome)
        : before(obj), after(obj), outcome(outcome)
        { }
};

game::vcr::test::Battle::Battle()
    : m_infos(), m_groups(), m_playability(IsPlayable), m_algorithmName("Test"), m_esbActive(), m_position(), m_auxInfo()
{ }

game::vcr::test::Battle::~Battle()
{ }

void
game::vcr::test::Battle::addObject(const Object& obj, int outcome)
{
    m_infos.pushBackNew(new Info(obj, outcome));
}

game::vcr::Object*
game::vcr::test::Battle::getObject(size_t slot, bool after)
{
    if (slot < m_infos.size()) {
        if (after) {
            return &m_infos[slot]->after;
        } else {
            return &m_infos[slot]->before;
        }
    } else {
        return 0;
    }
}

void
game::vcr::test::Battle::addGroup(const GroupInfo& info)
{
    m_groups.push_back(info);
}

void
game::vcr::test::Battle::setPlayability(Playability p)
{
    m_playability = p;
}

void
game::vcr::test::Battle::setAlgorithmName(const String_t& name)
{
    m_algorithmName = name;
}

void
game::vcr::test::Battle::setIsESBActive(bool flag)
{
    m_esbActive = flag;
}

void
game::vcr::test::Battle::setPosition(game::map::Point pos)
{
    m_position = pos;
}

void
game::vcr::test::Battle::setAuxiliaryInformation(AuxInfo info, int32_t value)
{
    m_auxInfo.set(info, value);
}

size_t
game::vcr::test::Battle::getNumObjects() const
{
    return m_infos.size();
}

const game::vcr::Object*
game::vcr::test::Battle::getObject(size_t slot, bool after) const
{
    return const_cast<Battle&>(*this).getObject(slot, after);
}

size_t
game::vcr::test::Battle::getNumGroups() const
{
    if (m_groups.empty()) {
        return m_infos.size();
    } else {
        return m_groups.size();
    }
}

game::vcr::GroupInfo
game::vcr::test::Battle::getGroupInfo(size_t groupNr, const game::config::HostConfiguration& /*config*/) const
{
    if (m_groups.empty()) {
        if (groupNr < m_infos.size()) {
            return GroupInfo(groupNr, 1, static_cast<int>(1000 + 100*groupNr), 0, m_infos[groupNr]->before.getOwner(), 10);
        } else {
            return GroupInfo();
        }
    } else {
        if (groupNr < m_groups.size()) {
            return m_groups[groupNr];
        } else {
            return GroupInfo();
        }
    }
}

int
game::vcr::test::Battle::getOutcome(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/, size_t slot)
{
    if (slot < m_infos.size()) {
        return m_infos[slot]->outcome;
    } else {
        return 0;
    }
}

game::vcr::Battle::Playability
game::vcr::test::Battle::getPlayability(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/)
{
    return m_playability;
}

void
game::vcr::test::Battle::prepareResult(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/, int /*resultLevel*/)
{ }

String_t
game::vcr::test::Battle::getAlgorithmName(afl::string::Translator& /*tx*/) const
{
    return m_algorithmName;
}

bool
game::vcr::test::Battle::isESBActive(const game::config::HostConfiguration& /*config*/) const
{
    return m_esbActive;
}

afl::base::Optional<game::map::Point>
game::vcr::test::Battle::getPosition() const
{
    return m_position;
}

afl::base::Optional<int32_t>
game::vcr::test::Battle::getAuxiliaryInformation(AuxInfo info) const
{
    return m_auxInfo.get(info);
}

String_t
game::vcr::test::Battle::getResultSummary(int /*viewpointPlayer*/, const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/, util::NumberFormatter /*fmt*/, afl::string::Translator& /*tx*/) const
{
    return String_t();
}

bool
game::vcr::test::Battle::computeScores(Score& /*score*/, size_t /*slot*/, const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/) const
{
    return false;
}
