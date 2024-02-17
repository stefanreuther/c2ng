/**
  *  \file game/interface/minefieldcontext.cpp
  *  \brief Class game::interface::MinefieldContext
  */

#include "game/interface/minefieldcontext.hpp"
#include "afl/string/format.hpp"
#include "game/interface/minefieldmethod.hpp"
#include "game/interface/minefieldproperty.hpp"
#include "game/interface/playerproperty.hpp"
#include "game/turn.hpp"
#include "interpreter/error.hpp"
#include "interpreter/procedurevalue.hpp"
#include "interpreter/propertyacceptor.hpp"

namespace {
    enum MinefieldDomain { MinefieldPropertyDomain, MinefieldMethodDomain, OwnerPropertyDomain };

    static const interpreter::NameTable minefield_mapping[] = {
        { "DELETE",          game::interface::immDelete,         MinefieldMethodDomain,   interpreter::thProcedure },
        { "ID",              game::interface::impId,             MinefieldPropertyDomain, interpreter::thInt },
        { "LASTSCAN",        game::interface::impLastScan,       MinefieldPropertyDomain, interpreter::thInt },
        { "LOC.X",           game::interface::impLocX,           MinefieldPropertyDomain, interpreter::thInt },
        { "LOC.Y",           game::interface::impLocY,           MinefieldPropertyDomain, interpreter::thInt },
        { "MARK",            game::interface::immMark,           MinefieldMethodDomain,   interpreter::thProcedure },
        { "MARKED",          game::interface::impMarked,         MinefieldPropertyDomain, interpreter::thBool },
        { "MESSAGE.ENCODED", game::interface::impEncodedMessage, MinefieldPropertyDomain, interpreter::thString },
        { "OWNER",           game::interface::iplShortName,      OwnerPropertyDomain,     interpreter::thString },
        { "OWNER$",          game::interface::iplId,             OwnerPropertyDomain,     interpreter::thInt },
        { "OWNER.ADJ",       game::interface::iplAdjName,        OwnerPropertyDomain,     interpreter::thString },
        { "RADIUS",          game::interface::impRadius,         MinefieldPropertyDomain, interpreter::thInt },
        { "SCANNED",         game::interface::impScanType,       MinefieldPropertyDomain, interpreter::thInt },
        { "TYPE",            game::interface::impTypeStr,        MinefieldPropertyDomain, interpreter::thString },
        { "TYPE$",           game::interface::impTypeCode,       MinefieldPropertyDomain, interpreter::thBool },
        { "UNITS",           game::interface::impUnits,          MinefieldPropertyDomain, interpreter::thInt },
        { "UNMARK",          game::interface::immUnmark,         MinefieldMethodDomain,   interpreter::thProcedure },
    };

    class MinefieldMethodValue : public interpreter::ProcedureValue {
     public:
        MinefieldMethodValue(int id,
                             game::interface::MinefieldMethod ism,
                             afl::base::Ref<game::Turn> turn)
            : m_id(id),
              m_method(ism),
              m_turn(turn)
            { }

        // ProcedureValue:
        virtual void call(interpreter::Process& /*proc*/, interpreter::Arguments& a)
            {
                if (game::map::Minefield* mf = m_turn->universe().minefields().get(m_id)) {
                    game::interface::callMinefieldMethod(*mf, m_method, a, m_turn->universe());
                }
            }

        virtual MinefieldMethodValue* clone() const
            { return new MinefieldMethodValue(m_id, m_method, m_turn); }

     private:
        const game::Id_t m_id;
        const game::interface::MinefieldMethod m_method;
        const afl::base::Ref<game::Turn> m_turn;
    };
}

game::interface::MinefieldContext::MinefieldContext(Id_t id, const afl::base::Ref<const Root>& root, const afl::base::Ref<Game>& game, const afl::base::Ref<Turn>& turn, afl::string::Translator& tx)
    : m_id(id),
      m_root(root),
      m_game(game),
      m_turn(turn),
      m_translator(tx)
{
    // ex IntMinefieldContext::IntMinefieldContext
}

game::interface::MinefieldContext::~MinefieldContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::MinefieldContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntMinefieldContext::lookup
    return lookupName(name, minefield_mapping, result) ? this : 0;
}

void
game::interface::MinefieldContext::set(PropertyIndex_t index, const afl::data::Value* value)
{
    // ex IntMinefieldContext::set
    if (game::map::Minefield* mf = getObject()) {
        switch (MinefieldDomain(minefield_mapping[index].domain)) {
         case MinefieldPropertyDomain:
            setMinefieldProperty(*mf, MinefieldProperty(minefield_mapping[index].index), value);
            break;

         default:
            throw interpreter::Error::notAssignable();
        }
    } else {
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::MinefieldContext::get(PropertyIndex_t index)
{
    // ex IntMinefieldContext::get
    if (game::map::Minefield* mf = getObject()) {
        int n;
        switch (MinefieldDomain(minefield_mapping[index].domain)) {
         case MinefieldPropertyDomain:
            return getMinefieldProperty(*mf, MinefieldProperty(minefield_mapping[index].index));

         case OwnerPropertyDomain:
            if (mf->getOwner().get(n)) {
                return getPlayerProperty(n, PlayerProperty(minefield_mapping[index].index), m_root->playerList(), *m_game, m_root->hostConfiguration(), m_translator);
            } else {
                return 0;
            }

         case MinefieldMethodDomain:
            return new MinefieldMethodValue(mf->getId(), MinefieldMethod(minefield_mapping[index].index), m_turn);
        }
    }
    return 0;
}

bool
game::interface::MinefieldContext::next()
{
    // ex values.pas:CMineContext.Next
    if (int id = m_turn->universe().minefields().findNextIndex(m_id)) {
        m_id = id;
        return true;
    } else {
        return false;
    }
}

game::interface::MinefieldContext*
game::interface::MinefieldContext::clone() const
{
    return new MinefieldContext(m_id, m_root, m_game, m_turn, m_translator);
}

game::map::Minefield*
game::interface::MinefieldContext::getObject()
{
    return m_turn->universe().minefields().get(m_id);
}

void
game::interface::MinefieldContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    // ex IntMinefieldContext::enumProperties
    acceptor.enumTable(minefield_mapping);
}

// BaseValue:
String_t
game::interface::MinefieldContext::toString(bool /*readable*/) const
{
    // ex IntMinefieldContext::toString
    return afl::string::Format("Minefield(%d)", m_id);
}

void
game::interface::MinefieldContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    // ex IntMinefieldContext::store
    if (&*m_turn == &m_game->currentTurn()) {
        out.tag   = out.Tag_Minefield;
        out.value = m_id;
    } else {
        rejectStore(out, aux, ctx);
    }
}

game::interface::MinefieldContext*
game::interface::MinefieldContext::create(int id, Session& session, const afl::base::Ref<Game>& g, const afl::base::Ref<Turn>& t, bool force)
{
    // ex values.pas:CreateMinefieldContext
    const Root* r = session.getRoot().get();
    if (r != 0 && (force || t->universe().minefields().get(id) != 0)) {
        return new MinefieldContext(id, *r, g, t, session.translator());
    } else {
        return 0;
    }
}
