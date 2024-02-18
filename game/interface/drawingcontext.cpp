/**
  *  \file game/interface/drawingcontext.cpp
  *  \brief Class game::interface::DrawingContext
  */

#include "game/interface/drawingcontext.hpp"
#include "game/interface/drawingmethod.hpp"
#include "game/interface/drawingproperty.hpp"
#include "game/root.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/procedurevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "game/actions/preconditions.hpp"

namespace {
    enum DrawingDomain { DrawingPropertyDomain, DrawingMethodDomain };

    static const interpreter::NameTable drawing_mapping[] = {
        { "COLOR",           game::interface::idpColor,          DrawingPropertyDomain,    interpreter::thInt },
        { "COMMENT",         game::interface::idpComment,        DrawingPropertyDomain,    interpreter::thString },
        { "DELETE",          game::interface::idmDelete,         DrawingMethodDomain,      interpreter::thProcedure },
        { "END.X",           game::interface::idpEndX,           DrawingPropertyDomain,    interpreter::thInt },
        { "END.Y",           game::interface::idpEndY,           DrawingPropertyDomain,    interpreter::thInt },
        { "EXPIRE",          game::interface::idpExpire,         DrawingPropertyDomain,    interpreter::thInt },
        { "LOC.X",           game::interface::idpLocX,           DrawingPropertyDomain,    interpreter::thInt },
        { "LOC.Y",           game::interface::idpLocY,           DrawingPropertyDomain,    interpreter::thInt },
        { "MESSAGE.ENCODED", game::interface::idpEncodedMessage, DrawingPropertyDomain,    interpreter::thString },
        { "RADIUS",          game::interface::idpRadius,         DrawingPropertyDomain,    interpreter::thInt },
        { "SETCOLOR",        game::interface::idmSetColor,       DrawingMethodDomain,      interpreter::thProcedure },
        { "SETCOMMENT",      game::interface::idmSetComment,     DrawingMethodDomain,      interpreter::thProcedure },
        { "SHAPE",           game::interface::idpShape,          DrawingPropertyDomain,    interpreter::thInt },
        { "TAG",             game::interface::idpTag,            DrawingPropertyDomain,    interpreter::thInt },
        { "TYPE",            game::interface::idpTypeString,     DrawingPropertyDomain,    interpreter::thString },
        { "TYPE$",           game::interface::idpTypeCode,       DrawingPropertyDomain,    interpreter::thInt },
    };

    class DrawingMethodValue : public interpreter::ProcedureValue {
     public:
        DrawingMethodValue(const afl::base::Ref<game::Turn>& turn, const game::map::DrawingContainer::Iterator_t& it, game::interface::DrawingMethod method)
            : m_turn(turn),
              m_iterator(it),
              m_method(method)
            { }
        virtual void call(interpreter::Process& /*proc*/, interpreter::Arguments& args)
            {
                // ex IntDrawingMethod::call
                game::actions::mustBeLocallyEditable(*m_turn);
                callDrawingMethod(m_turn->universe().drawings(), m_iterator, m_method, args);
            }
        virtual DrawingMethodValue* clone() const
            { return new DrawingMethodValue(m_turn, m_iterator, m_method); }

     private:
        const afl::base::Ref<game::Turn> m_turn;
        const game::map::DrawingContainer::Iterator_t m_iterator;
        const game::interface::DrawingMethod m_method;
    };
}

game::interface::DrawingContext::DrawingContext(const afl::base::Ref<Turn>& turn, const afl::base::Ref<const Root>& root, const game::map::DrawingContainer::Iterator_t& it)
    : m_turn(turn),
      m_root(root),
      m_iterator(it)
{ }

game::interface::DrawingContext::~DrawingContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::DrawingContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntDrawingContext::lookup
    return lookupName(name, drawing_mapping, result) ? this : 0;
}

void
game::interface::DrawingContext::set(PropertyIndex_t index, const afl::data::Value* value)
{
    // ex IntDrawingContext::set
    if (game::map::Drawing* d = *m_iterator) {
        switch (DrawingDomain(drawing_mapping[index].domain)) {
         case DrawingPropertyDomain:
            game::actions::mustBeLocallyEditable(*m_turn);
            setDrawingProperty(*d, DrawingProperty(drawing_mapping[index].index), value);
            m_turn->universe().drawings().sig_change.raise();
            break;
         default:
            throw interpreter::Error::notAssignable();
        }
    } else {
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::DrawingContext::get(PropertyIndex_t index)
{
    // ex IntDrawingContext::get
    if (const game::map::Drawing* d = *m_iterator) {
        switch (DrawingDomain(drawing_mapping[index].domain)) {
         case DrawingPropertyDomain:
            return getDrawingProperty(*d, DrawingProperty(drawing_mapping[index].index), m_root->charset());
         case DrawingMethodDomain:
            return new DrawingMethodValue(m_turn, m_iterator, DrawingMethod(drawing_mapping[index].index));
         default:
            return 0;
        }
    } else {
        return 0;
    }
}

bool
game::interface::DrawingContext::next()
{
    // ex IntDrawingContext::next
    const game::map::DrawingContainer& container = m_turn->universe().drawings();
    if (m_iterator == container.end()) {
        return false;
    } else {
        game::map::DrawingContainer::Iterator_t i = m_iterator;
        ++i;
        if (i == container.end()) {
            return false;
        } else {
            m_iterator = i;
            return true;
        }
    }
}

game::interface::DrawingContext*
game::interface::DrawingContext::clone() const
{
    // ex IntDrawingContext::clone
    return new DrawingContext(m_turn, m_root, m_iterator);
}

afl::base::Deletable*
game::interface::DrawingContext::getObject()
{
    // ex IntDrawingContext::getObject
    return 0;
}

void
game::interface::DrawingContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    return acceptor.enumTable(drawing_mapping);
}

// BaseValue:
String_t
game::interface::DrawingContext::toString(bool /*readable*/) const
{
    // ex IntDrawingContext::toString
    return "#<Marker>";
}

void
game::interface::DrawingContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    // ex IntDrawingContext::store
    rejectStore(out, aux, ctx);
}

game::interface::DrawingContext*
game::interface::DrawingContext::create(Session& session, const afl::base::Ref<Turn>& turn)
{
    // ex values.pas:CreateMarkerContext
    const Root* root = session.getRoot().get();
    if (root == 0) {
        return 0;
    }

    game::map::DrawingContainer& d = turn->universe().drawings();
    game::map::DrawingContainer::Iterator_t it = d.begin();
    if (it == d.end()) {
        return 0;
    }

    return new DrawingContext(turn, *root, it);
}
