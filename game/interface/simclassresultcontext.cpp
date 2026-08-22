/**
  *  \file game/interface/simclassresultcontext.cpp
  *  \brief Class game::interface::SimClassResultContext
  */

#include "game/interface/simclassresultcontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/values.hpp"

using interpreter::makeFloatValue;
using interpreter::makeIntegerValue;
using interpreter::makeStringValue;

namespace {
    // Individual scalar properties
    enum SimClassResultProperty {
        icrLabel,               // ClassInfo::label
        icrCount,               // ClassInfo::weight
        icrRatio                // ClassInfo::weight / cumulativeWeight
    };

    // Domain
    enum SimClassResultDomain {
        SimClassProperty,       // Index is SimClassResultProperty
        SimClassPlayer          // Index is player number, index into ownedUnits
    };

    static const interpreter::NameTable CLASS_MAPPING[] = {
        /* @q Count:Int (Simulation Class Result Property)
           Number of occurrences of this class result.
           @since PCC2 2.41.5 */
        { "COUNT",   icrCount, SimClassProperty, interpreter::thInt },
        /* @q Label:Str (Simulation Class Result Property)
           Name of this class result.
           A string such as "20x (40%)", or "35%", as shown on the
           <a href="pcc2:simresult">simulation result screen</a>.
           @since PCC2 2.41.5 */
        { "LABEL",   icrLabel, SimClassProperty, interpreter::thString },
        /* @q Player1:Int (Simulation Class Result Property)
           @q Player2:Int (Simulation Class Result Property)
           @q Player3:Int (Simulation Class Result Property)
           @q Player4:Int (Simulation Class Result Property)
           @q Player5:Int (Simulation Class Result Property)
           @q Player6:Int (Simulation Class Result Property)
           @q Player7:Int (Simulation Class Result Property)
           @q Player8:Int (Simulation Class Result Property)
           @q Player9:Int (Simulation Class Result Property)
           @q Player10:Int (Simulation Class Result Property)
           @q Player11:Int (Simulation Class Result Property)
           @q Player12:Int (Simulation Class Result Property)
           Number of this player's units surviving in this class result.
           @since PCC2 2.41.5 */
        { "PLAYER1",        1, SimClassPlayer,   interpreter::thInt },
        { "PLAYER10",      10, SimClassPlayer,   interpreter::thInt },
        { "PLAYER11",      11, SimClassPlayer,   interpreter::thInt },
        { "PLAYER12",      12, SimClassPlayer,   interpreter::thInt },
        { "PLAYER2",        2, SimClassPlayer,   interpreter::thInt },
        { "PLAYER3",        3, SimClassPlayer,   interpreter::thInt },
        { "PLAYER4",        4, SimClassPlayer,   interpreter::thInt },
        { "PLAYER5",        5, SimClassPlayer,   interpreter::thInt },
        { "PLAYER6",        6, SimClassPlayer,   interpreter::thInt },
        { "PLAYER7",        7, SimClassPlayer,   interpreter::thInt },
        { "PLAYER8",        8, SimClassPlayer,   interpreter::thInt },
        { "PLAYER9",        9, SimClassPlayer,   interpreter::thInt },
        /* @q Ratio:Num (Simulation Class Result Property)
           Relative frequency of this class result's occurrence,
           as a percentage (number between 0 and 100).
           @since PCC2 2.41.5 */
        { "RATIO",   icrRatio, SimClassProperty, interpreter::thFloat },
    };
}

const int game::interface::SimClassResultContext::MAX_PLAYERS;

game::interface::SimClassResultContext*
game::interface::SimClassResultContext::create(const game::sim::ResultList& result, const util::NumberFormatter& fmt)
{
    if (result.getNumClassResults() == 0) {
        return 0;
    } else {
        afl::base::Ptr<Infos_t> infos = new Infos_t();
        for (size_t i = 0, n = result.getNumClassResults(); i < n; ++i) {
            infos->push_back(result.describeClassResult(i, fmt));
        }
        return new SimClassResultContext(infos, result.getCumulativeWeight(), 0);
    }
}

game::interface::SimClassResultContext::SimClassResultContext(afl::base::Ptr<Infos_t> infos, int32_t cumulativeWeight, size_t slot)
    : m_infos(infos), m_cumulativeWeight(cumulativeWeight), m_slot(slot)
{ }

game::interface::SimClassResultContext::~SimClassResultContext()
{ }

interpreter::Context::PropertyAccessor*
game::interface::SimClassResultContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (interpreter::lookupName(name, CLASS_MAPPING, result)) {
        return this;
    } else {
        return 0;
    }
}

bool
game::interface::SimClassResultContext::next()
{
    if (m_slot+1 < m_infos->size()) {
        ++m_slot;
        return true;
    } else {
        return false;
    }
}

game::interface::SimClassResultContext*
game::interface::SimClassResultContext::clone() const
{
    return new SimClassResultContext(m_infos, m_cumulativeWeight, m_slot);
}

afl::base::Deletable*
game::interface::SimClassResultContext::getObject()
{
    return 0;
}

void
game::interface::SimClassResultContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(CLASS_MAPPING);
}

String_t
game::interface::SimClassResultContext::toString(bool /*readable*/) const
{
    return "#<sim-class-result>";
}

void
game::interface::SimClassResultContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

afl::data::Value*
game::interface::SimClassResultContext::get(PropertyIndex_t index)
{
    if (const game::sim::ResultList::ClassInfo* ci = getClassInfo()) {
        switch (SimClassResultDomain(CLASS_MAPPING[index].domain)) {
         case SimClassProperty:
            switch (SimClassResultProperty(CLASS_MAPPING[index].index)) {
             case icrLabel:
                return makeStringValue(ci->label);
             case icrCount:
                return makeIntegerValue(ci->weight);
             case icrRatio:
                if (m_cumulativeWeight <= 1) {
                    return makeIntegerValue(ci->weight);
                } else {
                    return makeFloatValue(100.0 * double(ci->weight) / m_cumulativeWeight);
                }
            }
            break;

         case SimClassPlayer:
            return makeIntegerValue(ci->ownedUnits.get(CLASS_MAPPING[index].index));
        }
    }
    return 0;
}

const game::sim::ResultList::ClassInfo*
game::interface::SimClassResultContext::getClassInfo()
{
    if (m_slot < m_infos->size()) {
        return &(*m_infos)[m_slot];
    } else {
        return 0;
    }
}
