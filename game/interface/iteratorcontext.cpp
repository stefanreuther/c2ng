/**
  *  \file game/interface/iteratorcontext.cpp
  */

#include <climits>
#include "game/interface/iteratorcontext.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "game/map/objectcursor.hpp"
#include "game/map/objecttype.hpp"
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
        iitObject,
        iitPrevious
    };

    const interpreter::NameTable ITERATOR_MAP[] = {
        { "COUNT",            iitCount,    0, interpreter::thInt },
        { "CURRENTINDEX",     iitCurrent,  0, interpreter::thInt },
        { "ID",               iitId,       0, interpreter::thArray },
        { "INDEX",            iitIndex,    0, interpreter::thArray },
        { "NEXTINDEX",        iitNext,     0, interpreter::thArray },
        { "OBJECT",           iitObject,   0, interpreter::thArray },
        { "PREVIOUSINDEX",    iitPrevious, 0, interpreter::thArray },
    };

    class IteratorFunction : public interpreter::IndexableValue {
     public:
        IteratorFunction(afl::base::Ptr<game::interface::IteratorProvider> provider, IteratorProperty p)
            : m_provider(provider),
              m_property(p)
            { }
        ~IteratorFunction()
            { }

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args)
            {
                // ex IntIteratorFunction::get
                int32_t i;
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
                                return makeIntegerValue(type->findPreviousIndexWrap(i, fl & 1));
                            }
                        }
                    }
                    return 0;

                 case iitObject:
                    // FIXME: implement this (missing in PCC2)

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
        virtual int32_t getDimension(int32_t /*which*/)
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

        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext* /*ctx*/) const
            {
                // ex IntIteratorFunction::store
                throw interpreter::Error::notSerializable();
            }

     private:
        afl::base::Ptr<game::interface::IteratorProvider> m_provider;
        IteratorProperty m_property;
    };
}



game::interface::IteratorContext::IteratorContext(afl::base::Ptr<IteratorProvider> provider)
    : SingleContext(),
      m_provider(provider)
{
    // ex IntIteratorContext::IntIteratorContext
}

game::interface::IteratorContext::~IteratorContext()
{ }

// Context:
bool
game::interface::IteratorContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntIteratorContext::lookup
    return lookupName(name, ITERATOR_MAP, result);
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
     case iitObject:
     case iitPrevious:
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
game::interface::IteratorContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext* /*ctx*/) const
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
//     - Iterator(n).NextIndex(x,fl): get next index after x. 0 if none found. Flags are "w" to permit wrap, "m" to accept only marked.
//     - Iterator(n).PreviousIndex(x,fl): same like NextIndex, but other direction
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

    return makeIteratorValue(session.getGame(), v);
}

afl::data::Value*
game::interface::makeIteratorValue(afl::base::Ptr<Game> game, int nr)
{
    class NumberedIteratorProvider : public IteratorProvider {
     public:
        NumberedIteratorProvider(afl::base::Ptr<Game> game, int nr)
            : m_game(game),
              m_number(nr)
            { }
        virtual game::map::ObjectCursor* getCursor()
            {
                if (m_game.get() != 0) {
                    return m_game->cursors().getCursorByNumber(m_number);
                } else {
                    return 0;
                }
            }
        virtual game::map::ObjectType* getType()
            {
                if (m_game.get() != 0) {
                    return m_game->cursors().getTypeByNumber(m_number);
                } else {
                    return 0;
                }
            }
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
        afl::base::Ptr<game::Game> m_game;
        int m_number;
    };

    if (game.get() == 0) {
        return 0;
    } else if (game->cursors().getTypeByNumber(nr) == 0) {
        throw interpreter::Error::rangeError();
    } else {
        return new IteratorContext(new NumberedIteratorProvider(game, nr));
    }
}
