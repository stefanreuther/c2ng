/**
  *  \file game/interface/minefieldcontext.cpp
  */

#include "game/interface/minefieldcontext.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "game/interface/playerproperty.hpp"
#include "game/interface/minefieldproperty.hpp"
#include "interpreter/error.hpp"
#include "game/turn.hpp"
#include "afl/string/format.hpp"

namespace {
    enum MinefieldDomain { MinefieldPropertyDomain, MinefieldMethodDomain, OwnerPropertyDomain };
   
    static const interpreter::NameTable minefield_mapping[] = {
        // { "DELETE",     2,                             MinefieldMethodDomain,   interpreter::thProcedure },
        { "ID",         game::interface::impId,        MinefieldPropertyDomain, interpreter::thInt },
        { "LASTSCAN",   game::interface::impLastScan,  MinefieldPropertyDomain, interpreter::thInt },
        { "LOC.X",      game::interface::impLocX,      MinefieldPropertyDomain, interpreter::thInt },
        { "LOC.Y",      game::interface::impLocY,      MinefieldPropertyDomain, interpreter::thInt },
        // { "MARK",       0,                             MinefieldMethodDomain,   interpreter::thProcedure },
        { "MARKED",     game::interface::impMarked,    MinefieldPropertyDomain, interpreter::thBool },
        { "OWNER",      game::interface::iplShortName, OwnerPropertyDomain,     interpreter::thString },
        { "OWNER$",     game::interface::iplId,        OwnerPropertyDomain,     interpreter::thInt },
        { "OWNER.ADJ",  game::interface::iplAdjName,   OwnerPropertyDomain,     interpreter::thString },
        { "RADIUS",     game::interface::impRadius,    MinefieldPropertyDomain, interpreter::thInt },
        { "SCANNED",    game::interface::impScanType,  MinefieldPropertyDomain, interpreter::thInt },
        { "TYPE",       game::interface::impTypeStr,   MinefieldPropertyDomain, interpreter::thString },
        { "TYPE$",      game::interface::impTypeCode,  MinefieldPropertyDomain, interpreter::thInt },
        { "UNITS",      game::interface::impUnits,     MinefieldPropertyDomain, interpreter::thInt },
        // { "UNMARK",     1,                             MinefieldMethodDomain,   interpreter::thProcedure },
    };
}

game::interface::MinefieldContext::MinefieldContext(int id, afl::base::Ptr<Root> root, afl::base::Ptr<Game> game)
    : m_id(id),
      m_root(root),
      m_game(game)
{
    // ex IntMinefieldContext::IntMinefieldContext
}

game::interface::MinefieldContext::~MinefieldContext()
{ }

// Context:
bool
game::interface::MinefieldContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntMinefieldContext::lookup
    return lookupName(name, minefield_mapping, result);
}

void
game::interface::MinefieldContext::set(PropertyIndex_t index, afl::data::Value* value)
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
            if (mf->getOwner(n)) {
                return getPlayerProperty(n, PlayerProperty(minefield_mapping[index].index), m_root->playerList(), *m_game, m_root->hostConfiguration());
            } else {
                return 0;
            }

         case MinefieldMethodDomain:
         // FIXME: port this (MinefieldMethod)
         //    return new IntObjectProcedureValue(*mf, *getCurrentUniverse(), minefield_methods[minefield_mapping[index].index]);
            ;
        }
    }
    return 0;
}

bool
game::interface::MinefieldContext::next()
{
    if (int id = m_game->currentTurn().universe().minefields().findNextIndex(m_id)) {
        m_id = id;
        return true;
    } else {
        return false;
    }
}

game::interface::MinefieldContext*
game::interface::MinefieldContext::clone() const
{
    return new MinefieldContext(m_id, m_root, m_game);
}

game::map::Minefield*
game::interface::MinefieldContext::getObject()
{
    return m_game->currentTurn().universe().minefields().get(m_id);
}

void
game::interface::MinefieldContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
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
game::interface::MinefieldContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext* /*ctx*/) const
{
    // ex IntMinefieldContext::store
    out.tag   = out.Tag_Minefield;
    out.value = m_id;
}
