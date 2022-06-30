/**
  *  \file game/interface/costsummarycontext.cpp
  *  \brief Class game::interface::CostSummaryContext
  */

#include "game/interface/costsummarycontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/values.hpp"

using game::spec::Cost;
using game::spec::CostSummary;

namespace {
    /* Property indexes */
    enum CostSummaryPropertyIndex {
        bpName,
        bpCount,
        bpT,
        bpD,
        bpM,
        bpCash,
        bpMoney,                // Not in PCC2
        bpSupplies              // Not in PCC2
    };

    /* Property name lookup table */
    const interpreter::NameTable BILL_MAPPING[] = {
        { "CASH",     bpCash,     0,  interpreter::thInt },
        { "COUNT",    bpCount,    0,  interpreter::thInt },
        { "D",        bpD,        0,  interpreter::thInt },
        { "M",        bpM,        0,  interpreter::thInt },
        { "MONEY",    bpMoney,    0,  interpreter::thInt },
        { "NAME",     bpName,     0,  interpreter::thString },
        { "SUPPLIES", bpSupplies, 0,  interpreter::thInt },
        { "T",        bpT,        0,  interpreter::thInt },
    };
}


game::interface::CostSummaryContext*
game::interface::CostSummaryContext::create(afl::base::Ptr<game::spec::CostSummary> cs)
{
    if (cs.get() != 0 && cs->getNumItems() > 0) {
        return new CostSummaryContext(cs, 0);
    } else {
        return 0;
    }
}

game::interface::CostSummaryContext::~CostSummaryContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::CostSummaryContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex BillContext::lookup
    return interpreter::lookupName(name, BILL_MAPPING, result) ? this : 0;
}

afl::data::Value*
game::interface::CostSummaryContext::get(PropertyIndex_t index)
{
    if (const CostSummary::Item* it = m_costSummary->get(m_index)) {
        switch (CostSummaryPropertyIndex(BILL_MAPPING[index].index)) {
         case bpName:     return interpreter::makeStringValue(it->name);
         case bpCount:    return interpreter::makeIntegerValue(it->multiplier);
         case bpT:        return interpreter::makeIntegerValue(it->cost.get(Cost::Tritanium));
         case bpD:        return interpreter::makeIntegerValue(it->cost.get(Cost::Duranium));;
         case bpM:        return interpreter::makeIntegerValue(it->cost.get(Cost::Molybdenum));
         case bpCash:     return interpreter::makeIntegerValue(it->cost.get(Cost::Money) + it->cost.get(Cost::Supplies));
         case bpMoney:    return interpreter::makeIntegerValue(it->cost.get(Cost::Money));
         case bpSupplies: return interpreter::makeIntegerValue(it->cost.get(Cost::Supplies));
        }
    }
    return 0;
}

bool
game::interface::CostSummaryContext::next()
{
    // ex BillContext::next
    if (m_index+1 < m_costSummary->getNumItems()) {
        ++m_index;
        return true;
    } else {
        return false;
    }
}

game::interface::CostSummaryContext*
game::interface::CostSummaryContext::clone() const
{
    return new CostSummaryContext(m_costSummary, m_index);
}

game::map::Object*
game::interface::CostSummaryContext::getObject()
{
    return 0;
}

void
game::interface::CostSummaryContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex BillContext::enumProperties
    acceptor.enumTable(BILL_MAPPING);
}

// BaseValue:
String_t
game::interface::CostSummaryContext::toString(bool /*readable*/) const
{
    return "#<CostSummary>";
}

void
game::interface::CostSummaryContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
