/**
  *  \file game/interface/ufocontext.cpp
  */

#include "game/interface/ufocontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/typehint.hpp"
#include "game/interface/ufoproperty.hpp"
#include "interpreter/propertyacceptor.hpp"

namespace {

    enum UfoDomain { UfoPropertyDomain, UfoMethodDomain };

    const interpreter::NameTable UFO_MAPPING[] = {
        { "COLOR",          game::interface::iupColorPCC,      UfoPropertyDomain, interpreter::thInt },
        { "COLOR.EGA",      game::interface::iupColorEGA,      UfoPropertyDomain, interpreter::thInt },
        { "HEADING",        game::interface::iupHeadingName,   UfoPropertyDomain, interpreter::thString },
        { "HEADING$",       game::interface::iupHeadingInt,    UfoPropertyDomain, interpreter::thInt },
        { "ID",             game::interface::iupId,            UfoPropertyDomain, interpreter::thInt },
        { "ID2",            game::interface::iupId2,           UfoPropertyDomain, interpreter::thInt },
        { "INFO1",          game::interface::iupInfo1,         UfoPropertyDomain, interpreter::thString },
        { "INFO2",          game::interface::iupInfo2,         UfoPropertyDomain, interpreter::thString },
        { "KEEP",           game::interface::iupKeepFlag,      UfoPropertyDomain, interpreter::thBool },
        { "LASTSCAN",       game::interface::iupLastScan,      UfoPropertyDomain, interpreter::thInt },
        { "LOC.X",          game::interface::iupLocX,          UfoPropertyDomain, interpreter::thInt },
        { "LOC.Y",          game::interface::iupLocY,          UfoPropertyDomain, interpreter::thInt },
        { "MARK",           0,                                 UfoMethodDomain,   interpreter::thProcedure },
        { "MARKED",         game::interface::iupMarked,        UfoPropertyDomain, interpreter::thBool },
        { "MOVE.DX",        game::interface::iupMoveDX,        UfoPropertyDomain, interpreter::thInt },
        { "MOVE.DY",        game::interface::iupMoveDY,        UfoPropertyDomain, interpreter::thInt },
        { "NAME",           game::interface::iupName,          UfoPropertyDomain, interpreter::thString },
        { "RADIUS",         game::interface::iupRadius,        UfoPropertyDomain, interpreter::thInt },
        { "SPEED",          game::interface::iupSpeedName,     UfoPropertyDomain, interpreter::thString },
        { "SPEED$",         game::interface::iupSpeedInt,      UfoPropertyDomain, interpreter::thInt },
        { "TYPE",           game::interface::iupType,          UfoPropertyDomain, interpreter::thInt },
        { "UNMARK",         1,                                 UfoMethodDomain,   interpreter::thProcedure },
        { "VISIBLE.PLANET", game::interface::iupVisiblePlanet, UfoPropertyDomain, interpreter::thInt },
        { "VISIBLE.SHIP",   game::interface::iupVisibleShip,   UfoPropertyDomain, interpreter::thInt },
    };


// static const IntObjectProcedureValue::call_t ufo_methods[] = {
//     IFObjMark,                  // 0
//     IFObjUnmark,                // 1
// };

}


game::interface::UfoContext::UfoContext(Id_t slot, afl::base::Ref<Turn> turn, Session& session)
    : Context(),
      m_slot(slot),
      m_turn(turn),
      m_session(session)
{ }

game::interface::UfoContext::~UfoContext()
{ }

// Context:
game::interface::UfoContext*
game::interface::UfoContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntUfoContext::lookup
    return lookupName(name, UFO_MAPPING, result) ? this : 0;
}

void
game::interface::UfoContext::set(PropertyIndex_t index, afl::data::Value* value)
{
    // ex IntUfoContext::set
    if (game::map::Ufo* ufo = getObject()) {
        switch (UfoDomain(UFO_MAPPING[index].domain)) {
         case UfoPropertyDomain:
            setUfoProperty(*ufo, UfoProperty(UFO_MAPPING[index].index), value);
            break;
         case UfoMethodDomain:
            throw interpreter::Error::notAssignable();
        }
    } else {
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::UfoContext::get(PropertyIndex_t index)
{
    // ex IntUfoContext::get
    if (game::map::Ufo* ufo = getObject()) {
        switch (UfoDomain(UFO_MAPPING[index].domain)) {
         case UfoPropertyDomain:
            return getUfoProperty(*ufo, UfoProperty(UFO_MAPPING[index].index), m_session.translator(), m_session.interface());
         case UfoMethodDomain:
            // FIXME
            // return new IntObjectProcedureValue(*it, getDisplayedTurn().getCurrentUniverse(), ufo_methods[ufo_mapping[index].index]);
            return 0;
        }
        return 0;
    } else {
        return 0;
    }
}

bool
game::interface::UfoContext::next()
{
    // ex IntUfoContext::next
    if (Id_t nextSlot = m_turn->universe().ufos().findNextIndexNoWrap(m_slot, false)) {
        m_slot = nextSlot;
        return true;
    } else {
        return false;
    }
}

game::interface::UfoContext*
game::interface::UfoContext::clone() const
{
    // ex IntUfoContext::clone
    return new UfoContext(m_slot, m_turn, m_session);
}

game::map::Ufo*
game::interface::UfoContext::getObject()
{
    // ex IntUfoContext::getObject
    return m_turn->universe().ufos().getObjectByIndex(m_slot);
}

void
game::interface::UfoContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex IntUfoContext::enumProperties
    acceptor.enumTable(UFO_MAPPING);
}

// BaseValue:
String_t
game::interface::UfoContext::toString(bool /*readable*/) const
{
    // ex IntUfoContext::toString
    // FIXME?
    return "#<ufo>";
}

void
game::interface::UfoContext::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntUfoContext::store
    // FIXME?
    throw interpreter::Error::notSerializable();
}
