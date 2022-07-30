/**
  *  \file game/interface/iteratorcontext.cpp
  *  \brief Class game::interface::IteratorContext
  */

#include <climits>
#include "game/interface/iteratorcontext.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "game/interface/ionstormcontext.hpp"
#include "game/interface/minefieldcontext.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/limits.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/minefield.hpp"
#include "game/map/objectcursor.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/functionvalue.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/values.hpp"

using interpreter::checkFlagArg;
using interpreter::checkIntegerArg;
using interpreter::makeIntegerValue;

namespace {
    /*
     *  Iterator property indexes
     */
    enum IteratorProperty {
        iitCount,
        iitCurrent,
        iitId,
        iitIndex,
        iitNearestIndex,
        iitNext,
        iitNextAt,
        iitObject,
        iitPrevious,
        iitPreviousAt,
        iitScreen
    };

    const interpreter::NameTable ITERATOR_MAP[] = {
        { "COUNT",            iitCount,      0, interpreter::thInt },
        { "CURRENTINDEX",     iitCurrent,    0, interpreter::thInt },
        { "ID",               iitId,         0, interpreter::thArray },
        { "INDEX",            iitIndex,      0, interpreter::thArray },
        { "NEARESTINDEX",     iitNearestIndex, 0, interpreter::thArray },
        { "NEXTINDEX",        iitNext,       0, interpreter::thArray },
        { "NEXTINDEXAT",      iitNextAt,     0, interpreter::thArray },
        { "OBJECT",           iitObject,     0, interpreter::thArray },
        { "PREVIOUSINDEX",    iitPreviousAt, 0, interpreter::thArray },
        { "PREVIOUSINDEXAT",  iitPreviousAt, 0, interpreter::thArray },
        { "SCREEN",           iitScreen,     0, interpreter::thInt },
    };

    /*
     *  Common options for NextIndex(), PreviousIndex(), etc.
     */

    const char*const BROWSE_OPTIONS = "MW";

    const int Browse_Marked = 1;
    const int Browse_Wrap = 2;

    /*
     *  IteratorFunction: implementation of all function propertis
     */

    class IteratorFunction : public interpreter::FunctionValue {
     public:
        IteratorFunction(afl::base::Ref<game::interface::IteratorProvider> provider, IteratorProperty p)
            : m_provider(provider),
              m_property(p)
            { }
        ~IteratorFunction()
            { }

        game::map::ObjectType* getFilteredType(afl::base::Deleter& del, int flags);
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual IteratorFunction* clone() const;

     private:
        afl::base::Ref<game::interface::IteratorProvider> m_provider;
        const IteratorProperty m_property;
    };
}

game::map::ObjectType*
IteratorFunction::getFilteredType(afl::base::Deleter& del, int flags)
{
    game::map::ObjectType* type = m_provider->getType();
    if (type != 0) {
        if ((flags & Browse_Marked) != 0) {
            type = &type->filterMarked(del, true);
        }
    }
    return type;
}

afl::data::Value*
IteratorFunction::get(interpreter::Arguments& args)
{
    // ex IntIteratorFunction::get
    int32_t i, x, y;
    switch (m_property) {
     case iitId:
        /* @q Id(index:Int):Int (Iterator Property)
           Find Id for a given index.
           EMPTY if the index does not correspond to a valid object.
           @since PCC2 2.40 */
        args.checkArgumentCount(1);
        if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)) {
            if (game::map::ObjectType* type = m_provider->getType()) {
                if (game::map::Object* obj = type->getObjectByIndex(i)) {
                    return makeIntegerValue(obj->getId());
                }
            }
        }
        return 0;

     case iitIndex:
        /* @q Index(id:Int):Int (Iterator Property)
           Find index for a given Id.
           EMPTY if there is no object with the given Id.
           @since PCC2 2.40 */
        args.checkArgumentCount(1);
        if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)) {
            if (game::map::ObjectType* type = m_provider->getType()) {
                if (game::Id_t index = type->findIndexForId(i)) {
                    return makeIntegerValue(index);
                }
            }
        }
        return 0;

     case iitNearestIndex:
        /* @q NearestIndex(x:Int, y:Int):Int (Iterator Property)
           Find index of object nearest to the given position.
           EMPTY if no object found.
           @since PCC2 2.40.10 */
        args.checkArgumentCount(2);
        if (checkIntegerArg(x, args.getNext(), 0, game::MAX_NUMBER) && checkIntegerArg(y, args.getNext(), 0, game::MAX_NUMBER)) {
            if (game::map::ObjectType* type = m_provider->getType()) {
                if (game::Game* g = m_provider->getSession().getGame().get()) {
                    if (game::Id_t index = type->findNearestIndex(game::map::Point(x, y), g->mapConfiguration())) {
                        return makeIntegerValue(index);
                    }
                }
            }
        }
        return 0;

     case iitNext:
        /* @q NextIndex(index:Int, Optional flags:Str):Int (Iterator Property)
           Find next index (browse forward).
           Pass index=0 to find the first possible index.

           Flags can be a combination of:
           - "M": only accept marked objects
           - "W": wraparound; after last object, select first one

           Returns the index of a found object, 0 if no applicable object exists.
           @since PCC2 2.40 */
        args.checkArgumentCount(1, 2);
        if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)) {
            int32_t fl = 0;
            checkFlagArg(fl, 0, args.getNext(), BROWSE_OPTIONS);
            afl::base::Deleter del;
            if (game::map::ObjectType* type = getFilteredType(del, fl)) {
                if ((fl & Browse_Wrap) != 0) {
                    return makeIntegerValue(type->findNextIndexWrap(i, false));
                } else {
                    return makeIntegerValue(type->findNextIndexNoWrap(i, false));
                }
            }
        }
        return 0;

     case iitPrevious:
        /* @q PreviousIndex(index:Int, Optional flags:Str):Int (Iterator Property)
           Find previous index (browse backward).
           Pass index=0 to find the last possible index.

           Flags can be a combination of:
           - "M": only accept marked objects
           - "W": wraparound; after first object, select last one

           Returns the index of a found object, 0 if no applicable object exists.
           @since PCC2 2.40 */
        args.checkArgumentCount(1, 2);
        if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)) {
            int32_t fl = 0;
            checkFlagArg(fl, 0, args.getNext(), BROWSE_OPTIONS);
            afl::base::Deleter del;
            if (game::map::ObjectType* type = getFilteredType(del, fl)) {
                if ((fl & Browse_Wrap) != 0) {
                    return makeIntegerValue(type->findPreviousIndexWrap(i, false));
                } else {
                    return makeIntegerValue(type->findPreviousIndexNoWrap(i, false));
                }
            }
        }
        return 0;

     case iitNextAt:
        /* @q NextIndexAt(index:Int, x:Int, y:Int, Optional flags:Str):Int (Iterator Property)
           Find next index at a given position.
           Pass index=0 to find the first possible index.

           Flags can be a combination of:
           - "M": only accept marked objects
           - "W": wraparound; after last object, select first one

           Returns the index of a found object, 0 if no applicable object exists.
           @since PCC2 2.40.9 */
        args.checkArgumentCount(3, 4);
        if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)
            && checkIntegerArg(x, args.getNext(), 0, game::MAX_NUMBER)
            && checkIntegerArg(y, args.getNext(), 0, game::MAX_NUMBER))
        {
            int32_t fl = 0;
            checkFlagArg(fl, 0, args.getNext(), BROWSE_OPTIONS);
            afl::base::Deleter del;
            if (game::map::ObjectType* type = getFilteredType(del, fl)) {
                if ((fl & Browse_Wrap) != 0) {
                    return makeIntegerValue(type->findNextObjectAtWrap(game::map::Point(x, y), i, false));
                } else {
                    return makeIntegerValue(type->findNextObjectAt(game::map::Point(x, y), i, false));
                }
            }
        }
        return 0;

     case iitPreviousAt:
        /* @q PreviousIndexAt(index:Int, x:Int, y:Int, Optional flags:Str):Int (Iterator Property)
           Find previous index at a given position.
           Pass index=0 to find the last possible index.

           Flags can be a combination of:
           - "M": only accept marked objects
           - "W": wraparound; after first object, select last one

           Returns the index of a found object, 0 if no applicable object exists.
           @since PCC2 2.40.9 */
        args.checkArgumentCount(3, 4);
        if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)
            && checkIntegerArg(x, args.getNext(), 0, game::MAX_NUMBER)
            && checkIntegerArg(y, args.getNext(), 0, game::MAX_NUMBER))
        {
            int32_t fl = 0;
            checkFlagArg(fl, 0, args.getNext(), BROWSE_OPTIONS);
            afl::base::Deleter del;
            if (game::map::ObjectType* type = getFilteredType(del, fl)) {
                if ((fl & Browse_Wrap) != 0) {
                    return makeIntegerValue(type->findPreviousObjectAtWrap(game::map::Point(x, y), i, false));
                } else {
                    return makeIntegerValue(type->findPreviousObjectAt(game::map::Point(x, y), i, false));
                }
            }
        }
        return 0;

     case iitObject:
        /* @q Object(index:Int):Obj (Iterator Property)
           Access object by index.
           For example, if this iterator iterates through planets,
           this function will return a planet as if by use of the {Planet()} function.

           @since PCC2 2.40 */
        args.checkArgumentCount(1);
        if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)) {
            if (game::map::ObjectType* type = m_provider->getType()) {
                if (game::map::Object* obj = type->getObjectByIndex(i)) {
                    return game::interface::createObjectContext(obj, m_provider->getSession());
                }
            }
        }
        return 0;

     case iitCurrent:
     case iitCount:
     case iitScreen:
        // Scalars, implemented outside
        break;
    }
    return 0;
}

IteratorFunction*
IteratorFunction::clone() const
{
    // ex IntIteratorFunction::clone
    return new IteratorFunction(m_provider, m_property);
}


/*
 *  IteratorContext
 */

game::interface::IteratorContext::IteratorContext(afl::base::Ref<IteratorProvider> provider)
    : SingleContext(),
      m_provider(provider)
{
    // ex IntIteratorContext::IntIteratorContext
}

game::interface::IteratorContext::~IteratorContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::IteratorContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntIteratorContext::lookup
    return lookupName(name, ITERATOR_MAP, result) ? this : 0;
}

void
game::interface::IteratorContext::set(PropertyIndex_t index, const afl::data::Value* value)
{
    // ex IntIteratorContext::set
    int32_t v;
    switch (IteratorProperty(index)) {
     case iitCurrent:
        if (checkIntegerArg(v, value, 0, INT_MAX)) {
            if (game::map::ObjectCursor* cursor = m_provider->getCursor()) {
                game::map::ObjectType* type = cursor->getObjectType();
                if (type != 0 && type->getObjectByIndex(v) != 0) {
                    cursor->setCurrentIndex(v);
                } else {
                    throw interpreter::Error::rangeError();
                }
            } else {
                throw interpreter::Error::notAssignable();
            }
        }
        break;
     default:
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::IteratorContext::get(PropertyIndex_t index)
{
    // ex IntIteratorContext::get
    switch (IteratorProperty(index)) {
     case iitCount:
        /* @q Count:Int (Iterator Property)
           Number of objects in this set (e.g. number of ships).
           @since PCC2 2.40 */
        if (game::map::ObjectType* type = m_provider->getType()) {
            return makeIntegerValue(type->countObjects());
        } else {
            return 0;
        }

     case iitCurrent:
        /* @q CurrentIndex:Int (Iterator Property)
           Index of currently-selected object.
           EMPTY if this iterator has no underlying cursor.
           @assignable
           @since PCC2 2.40 */
        if (game::map::ObjectCursor* cursor = m_provider->getCursor()) {
            return makeIntegerValue(cursor->getCurrentIndex());
        } else {
            return 0;
        }

     case iitScreen:
        /* @q Screen:Int (Iterator Property)
           Associated screen/iterator number.
           In particular, if this iterator was created using {Iterator()|Iterator(n)}, returns n.
           If this iterator matches the object set for a control screen,
           this is the correct value to use for {UI.GotoScreen}.

           For example, if this iterator iterates through own planets, this property has value 2.

           EMPTY if there is no associated screen number.

           @since PCC2 2.40.13 */
        if (int n = m_provider->getCursorNumber()) {
            return makeIntegerValue(n);
        } else {
            return 0;
        }

     case iitId:
     case iitIndex:
     case iitNearestIndex:
     case iitNext:
     case iitNextAt:
     case iitObject:
     case iitPrevious:
     case iitPreviousAt:
        return new IteratorFunction(m_provider, IteratorProperty(index));
    }
    return 0;
}

game::interface::IteratorContext*
game::interface::IteratorContext::clone() const
{
    // ex IntIteratorContext::clone
    return new IteratorContext(m_provider);
}

game::map::Object*
game::interface::IteratorContext::getObject()
{
    // ex IntIteratorContext::getObject
    return 0;
}

void
game::interface::IteratorContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex IntIteratorContext::enumProperties
    acceptor.enumTable(ITERATOR_MAP);
}

// BaseValue:
String_t
game::interface::IteratorContext::toString(bool readable) const
{
    // ex IntIteratorContext::toString
    if (readable) {
        return m_provider->toString();
    } else {
        return "#<iterator>";
    }
}

void
game::interface::IteratorContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntIteratorContext::store
    m_provider->store(out);
}


/* @q Iterator(n:Int):Iterator (Function)
   Access to a a set of objects.
   This function accesses the well-known global object sets.

   Parameter n selects the set. The values are chosen similar to {UI.GotoScreen} or {UI.ChooseObject}.

   <table>
    <tr><td width="4">1</td> <td width="10">Own starships</td></tr>
    <tr><td width="4">2</td> <td width="10">Own planets</td></tr>
    <tr><td width="4">3</td> <td width="10">Own starbases</td></tr>
    <tr><td width="4">10</td><td width="10">Fleets</td></tr>
    <tr><td width="4">21</td><td width="10">All ships</td></tr>
    <tr><td width="4">22</td><td width="10">All planets</td></tr>
    <tr><td width="4">30</td><td width="10">Ufos</td></tr>
    <tr><td width="4">31</td><td width="10">Ion storms</td></tr>
    <tr><td width="4">32</td><td width="10">Minefields</td></tr>
   </table>

   It is important that PCC2 distinguishes between <b>Index</b> and <b>Id</b>.
   The Id is the regular object Id, whereas the Index is an opaque value referring to that object.
   For ships, planets, starbases, and fleets, these values are always identical.
   For other object types, they may differ.
   Most functions in an iterator work on Indexes.
   Convert between index and Id using the Index() and Id() functions.

   @see int:index:group:iteratorproperty|Iterator Properties

   @since PCC2 2.40 */
afl::data::Value*
game::interface::IFIterator(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/iterif.h:IFIteratorGet
    args.checkArgumentCount(1);
    int32_t v;
    if (!checkIntegerArg(v, args.getNext())) {
        return 0;
    }

    afl::data::Value* result = makeIteratorValue(session, v);
    if (result == 0) {
        throw interpreter::Error::rangeError();
    }

    return result;
}

// Make iterator for a screen number.
interpreter::Context*
game::interface::makeIteratorValue(Session& session, int nr)
{
    class NumberedIteratorProvider : public IteratorProvider {
     public:
        NumberedIteratorProvider(Session& session, int nr)
            : m_session(session),
              m_number(nr)
            { }
        virtual game::map::ObjectCursor* getCursor()
            {
                if (Game* g = m_session.getGame().get()) {
                    return g->cursors().getCursorByNumber(m_number);
                } else {
                    return 0;
                }
            }
        virtual game::map::ObjectType* getType()
            {
                if (Game* g = m_session.getGame().get()) {
                    return g->cursors().getTypeByNumber(m_number);
                } else {
                    return 0;
                }
            }
        virtual int getCursorNumber()
            { return m_number; }
        virtual Session& getSession()
            { return m_session; }
        virtual void store(interpreter::TagNode& out)
            {
                out.tag = out.Tag_Iterator;
                out.value = m_number;
            }
        virtual String_t toString()
            {
                return afl::string::Format("Iterator(%d)", m_number);
            }
     private:
        Session& m_session;
        int m_number;
    };

    Game* g = session.getGame().get();
    if (g == 0) {
        return 0;
    } else if (g->cursors().getTypeByNumber(nr) == 0) {
        return 0;
    } else {
        return new IteratorContext(*new NumberedIteratorProvider(session, nr));
    }
}

// Create object context, given an object.
interpreter::Context*
game::interface::createObjectContext(game::map::Object* obj, Session& session)
{
    if (dynamic_cast<game::map::Ship*>(obj) != 0) {
        return ShipContext::create(obj->getId(), session);
    } else if (dynamic_cast<game::map::Planet*>(obj) != 0) {
        return PlanetContext::create(obj->getId(), session);
    } else if (dynamic_cast<game::map::IonStorm*>(obj) != 0) {
        return IonStormContext::create(obj->getId(), session);
    } else if (dynamic_cast<game::map::Minefield*>(obj) != 0) {
        return MinefieldContext::create(obj->getId(), session, false);
    } else {
        return 0;
    }
}
