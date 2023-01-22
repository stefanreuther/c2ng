/**
  *  \file game/interface/friendlycodecontext.cpp
  */

#include "game/interface/friendlycodecontext.hpp"
#include "interpreter/nametable.hpp"
#include "game/interface/friendlycodeproperty.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "afl/string/format.hpp"
#include "interpreter/values.hpp"

namespace {
    enum FriendlyCodeDomain {
        FriendlyCodePropertyDomain
    };

    const interpreter::NameTable FC_MAPPING[] = {
        { "DESCRIPTION", game::interface::ifpDescription, FriendlyCodePropertyDomain, interpreter::thString },
        { "FLAGS",       game::interface::ifpFlags,       FriendlyCodePropertyDomain, interpreter::thString },
        { "NAME",        game::interface::ifpName,        FriendlyCodePropertyDomain, interpreter::thString },
        { "RACES$",      game::interface::ifpRaces,       FriendlyCodePropertyDomain, interpreter::thInt },
    };
}

game::interface::FriendlyCodeContext::FriendlyCodeContext(size_t slot,
                                                          afl::base::Ref<Root> root,
                                                          afl::base::Ref<game::spec::ShipList> shipList,
                                                          afl::string::Translator& tx)
    : SimpleContext(),
      m_slot(slot),
      m_root(root),
      m_shipList(shipList),
      m_translator(tx)
{ }

game::interface::FriendlyCodeContext::~FriendlyCodeContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::FriendlyCodeContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, FC_MAPPING, result) ? this : 0;
}

afl::data::Value*
game::interface::FriendlyCodeContext::get(PropertyIndex_t index)
{
    const game::spec::FriendlyCode* fc = m_shipList->friendlyCodes().at(m_slot);
    if (fc != 0) {
        return getFriendlyCodeProperty(*fc, FriendlyCodeProperty(FC_MAPPING[index].index), m_root->playerList(), m_translator);
    } else {
        return 0;
    }
}

bool
game::interface::FriendlyCodeContext::next()
{
    size_t n = m_slot+1;
    if (m_shipList->friendlyCodes().at(n) != 0) {
        m_slot = n;
        return true;
    } else {
        return false;
    }
}

game::interface::FriendlyCodeContext*
game::interface::FriendlyCodeContext::clone() const
{
    return new FriendlyCodeContext(m_slot, m_root, m_shipList, m_translator);
}

afl::base::Deletable*
game::interface::FriendlyCodeContext::getObject()
{
    return 0;
}

void
game::interface::FriendlyCodeContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(FC_MAPPING);
}

// BaseValue:
String_t
game::interface::FriendlyCodeContext::toString(bool readable) const
{
    if (readable) {
        if (const game::spec::FriendlyCode* fc = m_shipList->friendlyCodes().at(m_slot)) {
            return afl::string::Format("FCode(%s)", interpreter::quoteString(fc->getCode()));
        }
    }
    return "#<fcode>";
}

void
game::interface::FriendlyCodeContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
