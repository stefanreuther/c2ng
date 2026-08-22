/**
  *  \file game/interface/simunitresultcontext.cpp
  *  \brief Class game::interface::SimUnitResultContext
  */

#include "game/interface/simunitresultcontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/values.hpp"

using interpreter::makeFloatValue;
using interpreter::makeIntegerValue;

namespace {
    typedef game::sim::ResultList::UnitInfo UnitInfo_t;

    // Individual scalar properties
    enum SimUnitScalarProperty {
        isuCaptures,
        isuFights,
        isuFightsWon
    };

    // Domain
    enum SimUnitResultDomain {
        SimUnitScalar,                   // Index is SimUnitScalarProperty
        SimUnitItemAverage,              // Index is UnitInfo::Type, return Item::averaga
        SimUnitItemMin,                  // Index is UnitInfo::Type, return Item::min
        SimUnitItemMax                   // Index is UnitInfo::Type, return Item::max
    };

    static const interpreter::NameTable UNIT_MAPPING[] = {
        /* @q Fights:Int (Simulation Unit Result Property)
           Number of fights this unit has been in.
           @since PCC2 2.41.5 */
        { "FIGHTS",                       isuFights,                         SimUnitScalar,      interpreter::thInt },
        /* @q Fights.Captured:Int (Simulation Unit Result Property)
           Number of fights in which this unit was captured.
           @since PCC2 2.41.5 */
        { "FIGHTS.CAPTURED",              isuCaptures,                       SimUnitScalar,      interpreter::thInt },
        /* @q Fights.Won:Int (Simulation Unit Result Property)
           Number of fights in which this unit survived.
           @since PCC2 2.41.5 */
        { "FIGHTS.WON",                   isuFightsWon,                      SimUnitScalar,      interpreter::thInt },
        /* @q Result.BaseFighters.Lost.Max:Int (Simulation Unit Result Property)
           Maximum of starbase fighters lost by this unit.
           @since PCC2 2.41.5 */
        { "RESULT.BASEFIGHTERS.LOST.MAX", UnitInfo_t::NumBaseFightersLost,   SimUnitItemMax,     interpreter::thInt },
        /* @q Result.BaseFighters.Lost.Min:Int (Simulation Unit Result Property)
           Minimum number of starbase fighters lost by this unit.
           @since PCC2 2.41.5 */
        { "RESULT.BASEFIGHTERS.LOST.MIN", UnitInfo_t::NumBaseFightersLost,   SimUnitItemMin,     interpreter::thInt },
        /* @q Result.Crew:Num (Simulation Unit Result Property)
           Average crew remaining on this unit.
           @since PCC2 2.41.5 */
        { "RESULT.CREW",                  UnitInfo_t::Crew,                  SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.Crew.Max:Int (Simulation Unit Result Property)
           Maximum crew remaining on this unit.
           @since PCC2 2.41.5 */
        { "RESULT.CREW.MAX",              UnitInfo_t::Crew,                  SimUnitItemMax,     interpreter::thInt },
        /* @q Result.Crew.Min:Int (Simulation Unit Result Property)
           Minimum crew remaining on this unit.
           @since PCC2 2.41.5 */
        { "RESULT.CREW.MIN",              UnitInfo_t::Crew,                  SimUnitItemMin,     interpreter::thInt },
        /* @q Result.Damage:Num (Simulation Unit Result Property)
           Average damage of this unit.
           @since PCC2 2.41.5 */
        { "RESULT.DAMAGE",                UnitInfo_t::Damage,                SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.Damage.Max:Int (Simulation Unit Result Property)
           Maximum damage of this unit.
           @since PCC2 2.41.5 */
        { "RESULT.DAMAGE.MAX",            UnitInfo_t::Damage,                SimUnitItemMax,     interpreter::thInt },
        /* @q Result.Damage.Min:Int (Simulation Unit Result Property)
           Minimum damage of this unit.
           @since PCC2 2.41.5 */
        { "RESULT.DAMAGE.MIN",            UnitInfo_t::Damage,                SimUnitItemMin,     interpreter::thInt },
        /* @q Result.Defense.Lost:Num (Simulation Unit Result Property)
           Average number of defense posts lost by this unit.
           @since PCC2 2.41.5 */
        { "RESULT.DEFENSE.LOST",          UnitInfo_t::DefenseLost,           SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.Defense.Lost.Max:Int (Simulation Unit Result Property)
           Maximum number of defense posts lost by this unit.
           @since PCC2 2.41.5 */
        { "RESULT.DEFENSE.LOST.MAX",      UnitInfo_t::DefenseLost,           SimUnitItemMax,     interpreter::thInt },
        /* @q Result.Defense.Lost.Min:Int (Simulation Unit Result Property)
           Minimum number of defense posts lost by this unit.
           @since PCC2 2.41.5 */
        { "RESULT.DEFENSE.LOST.MIN",      UnitInfo_t::DefenseLost,           SimUnitItemMin,     interpreter::thInt },
        /* @q Result.Fighters.Left:Num (Simulation Unit Result Property)
           Average number of fighters left on this unit.
           @since PCC2 2.41.5 */
        { "RESULT.FIGHTERS.LEFT",         UnitInfo_t::NumFightersRemaining,  SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.Fighters.Left.Max:Int (Simulation Unit Result Property)
           Maximum number of fighters left on this unit.
           @since PCC2 2.41.5 */
        { "RESULT.FIGHTERS.LEFT.MAX",     UnitInfo_t::NumFightersRemaining,  SimUnitItemMax,     interpreter::thInt },
        /* @q Result.Fighters.Left.Min:Int (Simulation Unit Result Property)
           Minimum number of fighters left on this unit.
           @since PCC2 2.41.5 */
        { "RESULT.FIGHTERS.LEFT.MIN",     UnitInfo_t::NumFightersRemaining,  SimUnitItemMin,     interpreter::thInt },
        /* @q Result.Fighters.Lost:Num (Simulation Unit Result Property)
           Average number of fighters lost by this unit.
           @since PCC2 2.41.5 */
        { "RESULT.FIGHTERS.LOST",         UnitInfo_t::NumFightersLost,       SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.Fighters.Lost.Max:Int (Simulation Unit Result Property)
           Maximum number of fighters lost by this unit.
           @since PCC2 2.41.5 */
        { "RESULT.FIGHTERS.LOST.MAX",     UnitInfo_t::NumFightersLost,       SimUnitItemMax,     interpreter::thInt },
        /* @q Result.Fighters.Lost.Min:Int (Simulation Unit Result Property)
           Minimum number of fighters lost by this unit.
           @since PCC2 2.41.5 */
        { "RESULT.FIGHTERS.LOST.MIN",     UnitInfo_t::NumFightersLost,       SimUnitItemMin,     interpreter::thInt },
        /* @q Result.MinFighters:Num (Simulation Unit Result Property)
           Average number of unused fighters aboard this unit.
           @since PCC2 2.41.5 */
        { "RESULT.MINFIGHTERS",           UnitInfo_t::MinFightersAboard,     SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.MinFighters.Max:Int (Simulation Unit Result Property)
           Maximum number of unused fighters aboard this unit.
           @since PCC2 2.41.5 */
        { "RESULT.MINFIGHTERS.MAX",       UnitInfo_t::MinFightersAboard,     SimUnitItemMax,     interpreter::thInt },
        /* @q Result.MinFighters.Min:Int (Simulation Unit Result Property)
           Minimum number of unused fighters aboard this unit.
           @since PCC2 2.41.5 */
        { "RESULT.MINFIGHTERS.MIN",       UnitInfo_t::MinFightersAboard,     SimUnitItemMin,     interpreter::thInt },
        /* @q Result.Shield:Num (Simulation Unit Result Property)
           Average shield level after fight.
           @since PCC2 2.41.5 */
        { "RESULT.SHIELD",                UnitInfo_t::Shield,                SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.Shield.Max:Int (Simulation Unit Result Property)
           Maximum shield level after fight.
           @since PCC2 2.41.5 */
        { "RESULT.SHIELD.MAX",            UnitInfo_t::Shield,                SimUnitItemMax,     interpreter::thInt },
        /* @q Result.Shield.Min:Int (Simulation Unit Result Property)
           Minimum shield level after fight.
           @since PCC2 2.41.5 */
        { "RESULT.SHIELD.MIN",            UnitInfo_t::Shield,                SimUnitItemMin,     interpreter::thInt },
        /* @q Result.Torps.Fired:Num (Simulation Unit Result Property)
           Average number of torpedoes fired.
           @since PCC2 2.41.5 */
        { "RESULT.TORPS.FIRED",           UnitInfo_t::NumTorpedoesFired,     SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.Torps.Fired.Max:Int (Simulation Unit Result Property)
           Maximum number of torpedoes fired.
           @since PCC2 2.41.5 */
        { "RESULT.TORPS.FIRED.MAX",       UnitInfo_t::NumTorpedoesFired,     SimUnitItemMax,     interpreter::thInt },
        /* @q Result.Torps.Fired.Min:Int (Simulation Unit Result Property)
           Minimum number of torpedoes fired.
           @since PCC2 2.41.5 */
        { "RESULT.TORPS.FIRED.MIN",       UnitInfo_t::NumTorpedoesFired,     SimUnitItemMin,     interpreter::thInt },
        /* @q Result.Torps.Hit:Num (Simulation Unit Result Property)
           Average number of torpedoes that hit their target.
           @since PCC2 2.41.5 */
        { "RESULT.TORPS.HIT",             UnitInfo_t::NumTorpedoHits,        SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.Torps.Hit.Max:Int (Simulation Unit Result Property)
           Maximum number of torpedoes that hit their target.
           @since PCC2 2.41.5 */
        { "RESULT.TORPS.HIT.MAX",         UnitInfo_t::NumTorpedoHits,        SimUnitItemMax,     interpreter::thInt },
        /* @q Result.Torps.Hit.Min:Int (Simulation Unit Result Property)
           Minimum number of torpedoes that hit their target.
           @since PCC2 2.41.5 */
        { "RESULT.TORPS.HIT.MIN",         UnitInfo_t::NumTorpedoHits,        SimUnitItemMin,     interpreter::thInt },
        /* @q Result.Torps.Left:Num (Simulation Unit Result Property)
           Average number of torpedoes remaining after fight.
           @since PCC2 2.41.5 */
        { "RESULT.TORPS.LEFT",            UnitInfo_t::NumTorpedoesRemaining, SimUnitItemAverage, interpreter::thFloat },
        /* @q Result.Torps.Left.Max:Int (Simulation Unit Result Property)
           Maximum number of torpedoes remaining after fight.
           @since PCC2 2.41.5 */
        { "RESULT.TORPS.LEFT.MAX",        UnitInfo_t::NumTorpedoesRemaining, SimUnitItemMax,     interpreter::thInt },
        /* @q Result.Torps.Left.Min:Int (Simulation Unit Result Property)
           Minimum number of torpedoes remaining after fight.
           @since PCC2 2.41.5 */
        { "RESULT.TORPS.LEFT.MIN",        UnitInfo_t::NumTorpedoesRemaining, SimUnitItemMin,     interpreter::thInt },
    };
}


/*
 *  SimUnitResultContext
 */

game::interface::SimUnitResultContext*
game::interface::SimUnitResultContext::create(const game::sim::ResultList& result,
                                              const afl::base::Ref<game::sim::Session>& session,
                                              const afl::base::Ref<const Root>& root,
                                              const afl::base::Ref<const game::spec::ShipList>& shipList,
                                              afl::string::Translator& tx)
{
    if (session->setup().getNumObjects() == 0 || result.getNumUnitResults() == 0) {
        return 0;
    } else {
        afl::base::Ptr<Infos_t> infos = new Infos_t();
        for (size_t i = 0, n = result.getNumUnitResults(); i < n; ++i) {
            infos->push_back(result.describeUnitResult(i, session->setup()));
        }
        return new SimUnitResultContext(infos, session, root, shipList, 0, tx);
    }
}

game::interface::SimUnitResultContext::SimUnitResultContext(afl::base::Ptr<Infos_t> infos,
                                                            const afl::base::Ref<game::sim::Session>& session,
                                                            const afl::base::Ref<const Root>& root,
                                                            const afl::base::Ref<const game::spec::ShipList>& shipList,
                                                            game::sim::Setup::Slot_t slot,
                                                            afl::string::Translator& tx)
    : m_delegate(session, root, shipList, slot, tx),
      m_infos(infos)
{ }

game::interface::SimUnitResultContext::~SimUnitResultContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::SimUnitResultContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (interpreter::lookupName(name, UNIT_MAPPING, result)) {
        return this;
    } else {
        return m_delegate.lookup(name, result);
    }
}

bool
game::interface::SimUnitResultContext::next()
{
    return m_delegate.next();
}

game::interface::SimUnitResultContext*
game::interface::SimUnitResultContext::clone() const
{
    return new SimUnitResultContext(*this);
}

afl::base::Deletable*
game::interface::SimUnitResultContext::getObject()
{
    return 0;
}

void
game::interface::SimUnitResultContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(UNIT_MAPPING);
    m_delegate.enumProperties(acceptor);
}

// BaseValue:
String_t
game::interface::SimUnitResultContext::toString(bool /*readable*/) const
{
    return "#<sim-unit-result>";
}

void
game::interface::SimUnitResultContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

// ReadOnlyAccessor:
afl::data::Value*
game::interface::SimUnitResultContext::get(PropertyIndex_t index)
{
    switch (SimUnitResultDomain(UNIT_MAPPING[index].domain)) {
     case SimUnitScalar:
        if (const UnitInfo_t* p = getUnitInfo()) {
            switch (SimUnitScalarProperty(UNIT_MAPPING[index].index)) {
             case isuCaptures:
                return makeIntegerValue(p->numCaptures);
             case isuFights:
                return makeIntegerValue(p->numFights);
             case isuFightsWon:
                return makeIntegerValue(p->numFightsWon);
            }
        }
        break;
     case SimUnitItemAverage:
        if (const UnitInfo_t::Item* p = getItem(UnitInfo_t::Type(UNIT_MAPPING[index].index))) {
            return makeFloatValue(p->average);
        }
        break;
     case SimUnitItemMin:
        if (const UnitInfo_t::Item* p = getItem(UnitInfo_t::Type(UNIT_MAPPING[index].index))) {
            return makeIntegerValue(p->min);
        }
        break;
     case SimUnitItemMax:
        if (const UnitInfo_t::Item* p = getItem(UnitInfo_t::Type(UNIT_MAPPING[index].index))) {
            return makeIntegerValue(p->max);
        }
        break;
    }
    return 0;
}

const game::sim::ResultList::UnitInfo*
game::interface::SimUnitResultContext::getUnitInfo()
{
    size_t slot = m_delegate.getSlotNumber();
    if (slot < m_infos->size()) {
        return &(*m_infos)[slot];
    } else {
        return 0;
    }
}

const game::sim::ResultList::UnitInfo::Item*
game::interface::SimUnitResultContext::getItem(game::sim::ResultList::UnitInfo::Type type)
{
    const UnitInfo_t* p = getUnitInfo();
    if (p != 0) {
        for (size_t i = 0, n = p->info.size(); i < n; ++i) {
            // ResultList distinguishes between NumBaseFightersLost (planets) and NumFightersLost (ships);
            // we want to publish that under a single name.
            if (p->info[i].type == type
                || (p->info[i].type == UnitInfo_t::NumBaseFightersLost && type == UnitInfo_t::NumFightersLost))
            {
                return &p->info[i];
            }
        }
    }
    return 0;
}
