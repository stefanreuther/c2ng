/**
  *  \file game/interface/iteratorcontext.cpp
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
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/values.hpp"

using interpreter::makeIntegerValue;
using interpreter::checkIntegerArg;
using interpreter::checkFlagArg;

namespace {
    enum IteratorProperty {
        iitCount,
        iitCurrent,
        iitId,
        iitIndex,
        iitNext,
        iitNextAt,
        iitObject,
        iitPrevious,
        iitPreviousAt
    };

    const interpreter::NameTable ITERATOR_MAP[] = {
        { "COUNT",            iitCount,      0, interpreter::thInt },
        { "CURRENTINDEX",     iitCurrent,    0, interpreter::thInt },
        { "ID",               iitId,         0, interpreter::thArray },
        { "INDEX",            iitIndex,      0, interpreter::thArray },
        { "NEXTINDEX",        iitNext,       0, interpreter::thArray },
        { "NEXTINDEXAT",      iitNextAt,     0, interpreter::thArray },
        { "OBJECT",           iitObject,     0, interpreter::thArray },
        { "PREVIOUSINDEX",    iitPreviousAt, 0, interpreter::thArray },
        { "PREVIOUSINDEXAT",  iitPreviousAt, 0, interpreter::thArray },
    };

    // FIXME: move into ObjectType, with a 'filter' argument that filters for marked, point, etc.
    game::Id_t findNextIndexAt(game::map::ObjectType& type, game::Id_t index, const game::map::Point& pt, bool marked)
    {
        // ex client/widgets/objcontrol.cc:findSameLocation, sort-of
        while ((index = type.findNextIndexNoWrap(index, marked)) != 0) {
            if (const game::map::Object* obj = type.getObjectByIndex(index)) {
                game::map::Point objPos;
                if (obj->getPosition(objPos) && objPos == pt) {
                    break;
                }
            }
        }
        return index;
    }

    game::Id_t findPreviousIndexAt(game::map::ObjectType& type, game::Id_t index, const game::map::Point& pt, bool marked)
    {
        while ((index = type.findPreviousIndexNoWrap(index, marked)) != 0) {
            if (const game::map::Object* obj = type.getObjectByIndex(index)) {
                game::map::Point objPos;
                if (obj->getPosition(objPos) && objPos == pt) {
                    break;
                }
            }
        }
        return index;
    }


    class IteratorFunction : public interpreter::IndexableValue {
     public:
        IteratorFunction(afl::base::Ref<game::interface::IteratorProvider> provider, IteratorProperty p)
            : m_provider(provider),
              m_property(p)
            { }
        ~IteratorFunction()
            { }

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args)
            {
                // ex IntIteratorFunction::get
                int32_t i, x, y;
                switch (m_property) {
                 case iitId:
                    // "Id(index)" => get Id of object
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
                    // "Index(id)" => find index for Id
                    args.checkArgumentCount(1);
                    if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)) {
                        if (game::map::ObjectType* type = m_provider->getType()) {
                            if (game::Id_t id = type->findIndexForId(i)) {
                                return makeIntegerValue(id);
                            }
                        }
                    }
                    return 0;

                 case iitNext:
                    // "Next(index[,flags])" => find next index from index
                    args.checkArgumentCount(1, 2);
                    if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)) {
                        int32_t fl = 0;
                        checkFlagArg(fl, 0, args.getNext(), "MW");
                        if (game::map::ObjectType* type = m_provider->getType()) {
                            if (fl & 2) {
                                return makeIntegerValue(type->findNextIndexWrap(i, fl & 1));
                            } else {
                                return makeIntegerValue(type->findNextIndexNoWrap(i, fl & 1));
                            }
                        }
                    }
                    return 0;

                 case iitPrevious:
                    // "Previous(index[,flags])" => find previous index from index
                    args.checkArgumentCount(1, 2);
                    if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)) {
                        int32_t fl = 0;
                        checkFlagArg(fl, 0, args.getNext(), "MW");
                        if (game::map::ObjectType* type = m_provider->getType()) {
                            if (fl & 2) {
                                return makeIntegerValue(type->findPreviousIndexWrap(i, fl & 1));
                            } else {
                                return makeIntegerValue(type->findPreviousIndexNoWrap(i, fl & 1));
                            }
                        }
                    }
                    return 0;

                 case iitNextAt:
                    // "NextIndexAt(index,x,y[,flags])" => find previous index from index
                    // @since PCC2 2.40.9
                    args.checkArgumentCount(3, 4);
                    if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)
                        && checkIntegerArg(x, args.getNext(), 0, game::MAX_NUMBER)
                        && checkIntegerArg(y, args.getNext(), 0, game::MAX_NUMBER))
                    {
                        int32_t fl = 0;
                        checkFlagArg(fl, 0, args.getNext(), "MW");
                        if (game::map::ObjectType* type = m_provider->getType()) {
                            game::Id_t id = findNextIndexAt(*type, i, game::map::Point(x, y), (fl & 1) != 0);
                            if (id == 0 && (fl & 2) != 0) {
                                id = findNextIndexAt(*type, 0, game::map::Point(x, y), (fl & 1) != 0);
                            }
                            return makeIntegerValue(id);
                        }
                    }
                    return 0;

                 case iitPreviousAt:
                    // "PreviousIndexAt(index,x,y[,flags])" => find previous index from index
                    // @since PCC2 2.40.9
                    args.checkArgumentCount(3, 4);
                    if (checkIntegerArg(i, args.getNext(), 0, INT_MAX)
                        && checkIntegerArg(x, args.getNext(), 0, game::MAX_NUMBER)
                        && checkIntegerArg(y, args.getNext(), 0, game::MAX_NUMBER))
                    {
                        int32_t fl = 0;
                        checkFlagArg(fl, 0, args.getNext(), "MW");
                        if (game::map::ObjectType* type = m_provider->getType()) {
                            game::Id_t id = findPreviousIndexAt(*type, i, game::map::Point(x, y), (fl & 1) != 0);
                            if (id == 0 && (fl & 2) != 0) {
                                id = findPreviousIndexAt(*type, 0, game::map::Point(x, y), (fl & 1) != 0);
                            }
                            return makeIntegerValue(id);
                        }
                    }
                    return 0;

                 case iitObject:
                    // "Object(id)" => object
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
                    // Scalars, implemented outside
                    break;
                }
                return 0;
            }
        virtual void set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
            {
                // ex IntIteratorFunction::set
                throw interpreter::Error::notAssignable();
            }

        // CallableValue:
        virtual int32_t getDimension(int32_t /*which*/) const
            {
                // ex IntIteratorFunction::getDimension
                return 0;
            }

        virtual interpreter::Context* makeFirstContext()
            {
                // ex IntIteratorFunction::makeFirstContext
                throw interpreter::Error::typeError(interpreter::Error::ExpectIterable);
            }

        virtual IteratorFunction* clone() const
            {
                // ex IntIteratorFunction::clone
                return new IteratorFunction(m_provider, m_property);
            }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            {
                // ex IntIteratorFunction::toString
                return "#<array>";
            }

        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            {
                // ex IntIteratorFunction::store
                throw interpreter::Error::notSerializable();
            }

     private:
        afl::base::Ref<game::interface::IteratorProvider> m_provider;
        const IteratorProperty m_property;
    };
}



game::interface::IteratorContext::IteratorContext(afl::base::Ref<IteratorProvider> provider)
    : SingleContext(),
      m_provider(provider)
{
    // ex IntIteratorContext::IntIteratorContext
}

game::interface::IteratorContext::~IteratorContext()
{ }

// Context:
game::interface::IteratorContext*
game::interface::IteratorContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntIteratorContext::lookup
    return lookupName(name, ITERATOR_MAP, result) ? this : 0;
}

void
game::interface::IteratorContext::set(PropertyIndex_t index, afl::data::Value* value)
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
        if (game::map::ObjectType* type = m_provider->getType()) {
            return makeIntegerValue(type->countObjects());
        } else {
            return 0;
        }

     case iitCurrent:
        if (game::map::ObjectCursor* cursor = m_provider->getCursor()) {
            return makeIntegerValue(cursor->getCurrentIndex());
        } else {
            return 0;
        }
     case iitId:
     case iitIndex:
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
game::interface::IteratorContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntIteratorContext::store
    m_provider->store(out);
}


// /** Implementation of the "Iterator" function. Iterators implement
//     access to a GObjectSelection. Scripts can use this to access a selection's
//     current object, and to iterate through the objects. This interface
//     is still preliminary.
//     - Iterator(n).CurrentIndex: currently active index
//     - Iterator(n).Count: number of objects
//     - Iterator(n).Id(x): given an index, return that object's Id
//     - Iterator(n).Index(id): given an Id, return that object's index; null if none
//     - Iterator(n).NextIndex(i,fl): get next index after i. 0 if none found. Flags are "w" to permit wrap, "m" to accept only marked.
//     - Iterator(n).NextIndexAt(i,x,y,fl): same, but filter for XY as well
//     - Iterator(n).PreviousIndex(i,fl): same like NextIndex, but other direction
//     - Iterator(n).PreviousIndexAt(i,x,y,fl): same but filter for XY as well
//     - Iterator(n).Object(x): object from index. Still undecided.

//     n is:
//     - 1,2,3: played ships, planets, bases
//     - 10: fleets
//     - 21, 22: all ships, planets
//     - 30: ufos
//     - 31: ion storms
//     - 32: minefields */
afl::data::Value*
game::interface::IFIterator(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/iterif.h:IFIteratorGet
    args.checkArgumentCount(1);
    int32_t v;
    if (!checkIntegerArg(v, args.getNext())) {
        return 0;
    }

    return makeIteratorValue(session, v, true);
}

interpreter::Context*
game::interface::makeIteratorValue(Session& session, int nr, bool reportRangeError)
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
        if (reportRangeError) {
            throw interpreter::Error::rangeError();
        } else {
            return 0;
        }
    } else {
        return new IteratorContext(*new NumberedIteratorProvider(session, nr));
    }
}

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
        // FIXME? other types
        return 0;
    }
}
