/**
  *  \file game/interface/referencecontext.cpp
  *  \brief Class game::interface::ReferenceContext
  */

#include "game/interface/referencecontext.hpp"
#include "afl/base/countof.hpp"
#include "game/interface/beamcontext.hpp"
#include "game/interface/enginecontext.hpp"
#include "game/interface/hullcontext.hpp"
#include "game/interface/ionstormcontext.hpp"
#include "game/interface/minefieldcontext.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/interface/playercontext.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/interface/torpedocontext.hpp"
#include "game/interface/ufocontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/values.hpp"
#include "interpreter/basevalue.hpp"

using game::Reference;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using interpreter::makeIntegerValue;
using interpreter::makeOptionalStringValue;
using interpreter::makeStringValue;

namespace {
    /* Reference property name table. */
    const interpreter::NameTable REFERENCE_MAP[] = {
        { "ID",        game::interface::irpId,            0, interpreter::thInt    },
        { "KIND",      game::interface::irpKind,          0, interpreter::thString },
        { "LOC.X",     game::interface::irpLocX,          0, interpreter::thInt    },
        { "LOC.Y",     game::interface::irpLocY,          0, interpreter::thInt    },
        { "NAME",      game::interface::irpPlainName,     0, interpreter::thString },
        { "NAME$",     game::interface::irpReferenceName, 0, interpreter::thString },
        { "NAME.FULL", game::interface::irpDetailedName,  0, interpreter::thString },
        { "OBJECT",    game::interface::irpObject,        0, interpreter::thNone   },
    };

    /* Reference type name table. */
    const struct TypeMap {
        const char* name;
        Reference::Type type;
    } TYPE_MAP[] = {
        // Do NOT mention 'Location' because that cannot be constructed from type+id
        { "b",         Reference::Starbase  },
        { "base",      Reference::Starbase  },
        { "beam",      Reference::Beam      },
        { "e",         Reference::Engine    },
        { "engine",    Reference::Engine    },
        { "h",         Reference::Hull      },
        { "hull",      Reference::Hull      },
        { "i",         Reference::IonStorm  },
        { "m",         Reference::Minefield },
        { "minefield", Reference::Minefield },
        { "p",         Reference::Planet    },
        { "planet",    Reference::Planet    },
        { "player",    Reference::Player    },
        { "s",         Reference::Ship      },
        { "ship",      Reference::Ship      },
        { "special",   Reference::Special   },
        { "storm",     Reference::IonStorm  },
        { "t",         Reference::Torpedo   },
        { "torpedo",   Reference::Torpedo   },
        { "u",         Reference::Ufo       },
        { "ufo",       Reference::Ufo       },
        { "w",         Reference::Beam      },
        { "y",         Reference::Player    },
    };
}


game::interface::ReferenceContext::ReferenceContext(Reference ref, Session& session)
    : SingleContext(),
      m_ref(ref),
      m_session(session)
{ }

game::interface::ReferenceContext::~ReferenceContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::ReferenceContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, REFERENCE_MAP, result) ? this : 0;
}

afl::data::Value*
game::interface::ReferenceContext::get(PropertyIndex_t index)
{
    return getReferenceProperty(m_ref, ReferenceProperty(REFERENCE_MAP[index].index), m_session);
}

game::interface::ReferenceContext*
game::interface::ReferenceContext::clone() const
{
    return new ReferenceContext(m_ref, m_session);
}

afl::base::Deletable*
game::interface::ReferenceContext::getObject()
{
    return 0;
}

void
game::interface::ReferenceContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(REFERENCE_MAP);
}

// BaseValue:
String_t
game::interface::ReferenceContext::toString(bool /*readable*/) const
{
    return "#<reference>";
}

void
game::interface::ReferenceContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

game::Reference
game::interface::ReferenceContext::getReference() const
{
    return m_ref;
}


afl::data::Value*
game::interface::getReferenceProperty(Reference ref, ReferenceProperty prop, Session& session)
{
    switch (prop) {
     case irpLocX: {
        /* @q Loc.X:Int (Reference Property)
           If this is a reference to a map location, returns the X coordinate.
           @since PCC2 2.40.7 */
        game::map::Point pt;
        if (ref.getPosition().get(pt)) {
            return makeIntegerValue(pt.getX());
        } else {
            return 0;
        }
     }

     case irpLocY: {
        /* @q Loc.Y:Int (Reference Property)
           If this is a reference to a map location, returns the Y coordinate.
           @since PCC2 2.40.7 */
        game::map::Point pt;
        if (ref.getPosition().get(pt)) {
            return makeIntegerValue(pt.getY());
        } else {
            return 0;
        }
     }

     case irpId:
        /* @q Id:Int (Reference Property)
           Returns the Id of the ship/planet/hull/player/... this reference points at.
           @since PCC2 2.40.7 */
        return makeIntegerValue(ref.getId());

     case irpReferenceName:
        /* @q Name$:Str (Reference Property)
           Returns the internal name of the reference, for example, "Ship #13", "Engine #5".
           @since PCC2 2.40.7 */
        return makeStringValue(ref.toString(session.translator()));

     case irpPlainName:
        /* @q Name:Str (Reference Property)
           Returns the user-friendly name of the reference, for example, the ship or planet name.
           @since PCC2 2.40.7 */
        return makeOptionalStringValue(session.getReferenceName(ref, PlainName));

     case irpDetailedName:
        /* @q Name.Full:Str (Reference Property)
           Returns the full user-friendly name of the reference, for example, "Ship #13: NSEA Protector".
           @since PCC2 2.40.7 */
        return makeOptionalStringValue(session.getReferenceName(ref, DetailedName));

     case irpKind:
        /* @q Kind:Str (Reference Property)
           Returns the kind of the reference, one of:
           - "player"
           - "location"
           - "ship"
           - "planet"
           - "base"
           - "storm"
           - "minefield"
           - "ufo"
           - "hull"
           - "engine"
           - "beam"
           - "torpedo"
           - "special"
           @since PCC2 2.40.7 */
        if (const char* p = getReferenceTypeName(ref.getType())) {
            return makeStringValue(p);
        } else {
            return 0;
        }

     case irpObject:
        /* @q Object:Any (Reference Property)
           If the reference refers to a game object, returns the appropriate object.
           For example, if this is the reference to a planet, returns the equivalent of {Planet()|Planet(Id)};
           if this is a hull, returns the equivalent to {Hull()|Hull(Id)}.
           @since PCC2 2.40.7 */
        return makeObjectValue(ref, session);
    }
    return 0;
}

interpreter::Context*
game::interface::makeObjectValue(Reference ref, Session& session)
{
    switch (ref.getType()) {
     case Reference::Null:
     case Reference::Special:
        return 0;

     case Reference::Player:
        return PlayerContext::create(ref.getId(), session);

     case Reference::MapLocation:
        return 0;

     case Reference::Ship:
        if (Game* g = session.getGame().get()) {
            return ShipContext::create(ref.getId(), session, *g, g->viewpointTurn());
        } else {
            return 0;
        }

     case Reference::Planet:
     case Reference::Starbase:
        if (Game* g = session.getGame().get()) {
            return PlanetContext::create(ref.getId(), session, *g, g->viewpointTurn());
        } else {
            return 0;
        }

     case Reference::IonStorm:
        if (Game* g = session.getGame().get()) {
            return IonStormContext::create(ref.getId(), session, g->viewpointTurn());
        } else {
            return 0;
        }

     case Reference::Minefield:
        if (Game* g = session.getGame().get()) {
            return MinefieldContext::create(ref.getId(), session, *g, g->viewpointTurn(), false);
        } else {
            return 0;
        }

     case Reference::Ufo:
        if (Game* g = session.getGame().get()) {
            Turn& t = g->viewpointTurn();
            game::map::UfoType& ty = t.universe().ufos();
            Id_t slot = ty.findIndexForId(ref.getId());
            if (ty.getObjectByIndex(slot) != 0) {
                return new game::interface::UfoContext(slot, t, session.translator());
            }
        }
        return 0;

     case Reference::Hull:
        return HullContext::create(ref.getId(), session);

     case Reference::Engine:
        return EngineContext::create(ref.getId(), session);

     case Reference::Beam:
        return BeamContext::create(ref.getId(), session);

     case Reference::Torpedo:
        return TorpedoContext::create(true, ref.getId(), session);
    }
    return 0;
}

const char*
game::interface::getReferenceTypeName(Reference::Type t)
{
    switch (t) {
     case Reference::Null:         return 0;
     case Reference::Special:      return "special";
     case Reference::Player:       return "player";
     case Reference::MapLocation:  return "location";
     case Reference::Ship:         return "ship";
     case Reference::Planet:       return "planet";
     case Reference::Starbase:     return "base";
     case Reference::IonStorm:     return "storm";
     case Reference::Minefield:    return "minefield";
     case Reference::Ufo:          return "ufo";
     case Reference::Hull:         return "hull";
     case Reference::Engine:       return "engine";
     case Reference::Beam:         return "beam";
     case Reference::Torpedo:      return "torpedo";
    }
    return 0;
}

bool
game::interface::parseReferenceTypeName(const String_t& str, Reference::Type& t)
{
    for (size_t i = 0; i < countof(TYPE_MAP); ++i) {
        if (afl::string::strCaseCompare(str, TYPE_MAP[i].name) == 0) {
            t = TYPE_MAP[i].type;
            return true;
        }
    }
    return false;
}

/* @q Reference(kind:Str, id:Int):Reference (Function)
   Produces a reference to an object (unit, component, etc.).
   The %kind parameter determines the object type:
   - "b", "base": starbase (see {Planet()})
   - "e", "engine": engine (see {Engine()})
   - "h", "hull": hull (see {Hull()})
   - "i", "storm": ion storm (see {Storm()})
   - "m", "minefield": minefield (see {Minefield()})
   - "p", "planet": planet (see {Planet()})
   - "s", "ship": ship (see {Ship()})
   - "t", "torpedo": torpedo (see {Launcher()})
   - "w", "beam": beam weapon (see {Beam()})
   - "y", "player": player (see {Player()})

   Experimental and subject to change:
   - "u", "ufo": Ufo (see {Ufo()})
   - "special": special entry.

   The referenced object need not exist.

   @since PCC2 2.40.7 */
afl::data::Value*
game::interface::IFReference(game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(2);

    String_t typeStr;
    int32_t id;
    if (!checkStringArg(typeStr, args.getNext())
        || !checkIntegerArg(id, args.getNext(), 0, MAX_REFERENCE_ID))
    {
        return 0;
    }

    Reference::Type type;
    if (!parseReferenceTypeName(typeStr, type)) {
        throw interpreter::Error::rangeError();
    }

    return new ReferenceContext(Reference(type, id), session);
}

/* @q LocationReference(x:Int, y:Int):Reference (Function)
   Produces a reference to a location in space.
   @since PCC2 2.40.7 */
afl::data::Value*
game::interface::IFLocationReference(game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(2);

    int32_t x, y;
    if (!checkIntegerArg(x, args.getNext(), 0, MAX_NUMBER)
        || !checkIntegerArg(y, args.getNext(), 0, MAX_NUMBER))
    {
        return 0;
    }

    return new ReferenceContext(game::map::Point(x, y), session);
}

bool
game::interface::checkReferenceArg(Reference& out, const afl::data::Value* p)
{
    // Null?
    if (p == 0) {
        return false;
    }

    // Needs to be a context.
    const interpreter::Context* cv = dynamic_cast<const interpreter::Context*>(p);
    if (cv == 0) {
        throw interpreter::Error::typeError();
    }

    // Resolving 'ID' needs to end up at a ReferenceContext.
    // We use this two-step process instead of directly casting p -> ReferenceContext
    // to allow wrappers/forwarders of the ReferenceContext, namely: ReferenceListContext::IterableReferenceContext.
    interpreter::Context::PropertyIndex_t pi;
    ReferenceContext* rcv = dynamic_cast<ReferenceContext*>(const_cast<interpreter::Context*>(cv)->lookup("ID", pi));
    if (rcv == 0) {
        throw interpreter::Error::typeError();
    }

    out = rcv->getReference();
    return true;
}
