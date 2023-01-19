/**
  *  \file game/interface/referencelistcontext.cpp
  */

#include "game/interface/referencelistcontext.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/turn.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/procedurevalue.hpp"
#include "interpreter/propertyacceptor.hpp"

using game::ref::List;
using interpreter::checkFlagArg;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;

namespace {
    enum ReferenceListProperty {
        ilpAdd,
        ilpAddObjects,
        ilpAddObjectsAt,
        ilpObjects
    };

    const interpreter::NameTable REFLIST_MAP[] = {
        { "ADD",          ilpAdd,          0, interpreter::thProcedure },
        { "ADDOBJECTS",   ilpAddObjects,   0, interpreter::thProcedure },
        { "ADDOBJECTSAT", ilpAddObjectsAt, 0, interpreter::thProcedure },
        { "OBJECTS",      ilpObjects,      0, interpreter::thFunction },
    };

    /* Fetch index argument for accessing a reference list.
       Validates type and range.
       \param[out] out Index
       \param[in] args Argument list
       \param[in] list reference List
       \retval true valid value obtained
       \retval false value was null
       \throw Error value vas invalid */
    bool fetchIndex(size_t& out, interpreter::Arguments& args, List& list)
    {
        args.checkArgumentCount(1);

        int32_t index;
        if (!checkIntegerArg(index, args.getNext())) {
            return false;
        }

        if (index < 0 || index >= int32_t(list.size())) {
            throw interpreter::Error::rangeError();
        }

        out = int32_t(index);
        return true;
    }
}

/*
 *  ReferenceListContext::ProcedureValue
 */

class game::interface::ReferenceListContext::ProcedureValue : public interpreter::ProcedureValue {
 public:
    typedef void Procedure_t(game::ref::List& list, Session& session, interpreter::Arguments& args);

    ProcedureValue(afl::base::Ref<Data> list, Session& session, Procedure_t& proc)
        : m_list(list), m_session(session), m_procedure(proc)
        { }

    virtual void call(interpreter::Process& /*proc*/, interpreter::Arguments& args)
        { m_procedure(m_list->list, m_session, args); }
    virtual ProcedureValue* clone() const
        { return new ProcedureValue(m_list, m_session, m_procedure); }
 private:
    afl::base::Ref<Data> m_list;
    Session& m_session;
    Procedure_t& m_procedure;
};

/*
 *  ReferenceListContext::IterableReferenceContext
 *
 *  ReferenceContext only takes a single reference and cannot iterate.
 *  IterableReferenceContext wraps the ReferenceContext to provide iteration.
 */

class game::interface::ReferenceListContext::IterableReferenceContext : public interpreter::SimpleContext {
 public:
    /* Regular constructor. */
    IterableReferenceContext(afl::base::Ref<Data> list, Session& session, size_t index)
        : SimpleContext(),
          m_list(list), m_session(session), m_index(index),
          m_child(new ReferenceContext(m_list->list[index], session))
        { }

    /* Copy constructor.
       We need a separate copy constructor to make exact copies.
       Using the regular constructor would rebuild m_child from list/index,
       which might produce a different result if list has already changed in the meantime. */
    IterableReferenceContext(const IterableReferenceContext& other)
        : SimpleContext(),
          m_list(other.m_list), m_session(other.m_session), m_index(other.m_index),
          m_child(other.m_child->clone())
        { }

    /* Forward most functions to m_child */
    virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
        { return m_child->lookup(name, result); }
    virtual IterableReferenceContext* clone() const
        { return new IterableReferenceContext(*this); }
    virtual game::map::Object* getObject()
        { return m_child->getObject(); }
    virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const
        { return m_child->enumProperties(acceptor); }
    virtual String_t toString(bool readable) const
        { return m_child->toString(readable); }
    virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
        { return m_child->store(out, aux, ctx); }

    /* Implementation of next() to iterate through the ref::List */
    virtual bool next()
        {
            size_t newIndex = m_index + 1;
            if (newIndex < m_list->list.size()) {
                m_index = newIndex;
                m_child.reset(new ReferenceContext(m_list->list[newIndex], m_session));
                return true;
            } else {
                return false;
            }
        }

 private:
    afl::base::Ref<Data> m_list;
    Session& m_session;
    size_t m_index;
    std::auto_ptr<ReferenceContext> m_child;
};

/*
 *  ReferenceListContext::ObjectArrayValue
 *
 *  Implementation of ReferenceList().Objects(...)
 */

/* @q Objects(index:Int):Reference (Reference List Operation)
   Access objects in the reference list.
   The index starts at 0.

   Given a reference list RL, you can use
   |  ForEach RL->Objects Do ...
   to iterate the references in the reference list, or
   |  n := Dim(RL->Objects)
   |  For i:=0 To n-1 Do ...
   to explicitly access the number of references on the list and iterate them.

   @since PCC2 2.40.7 */
class game::interface::ReferenceListContext::ObjectArrayValue : public interpreter::IndexableValue {
 public:
    ObjectArrayValue(afl::base::Ref<Data> list, Session& session)
        : m_list(list), m_session(session)
        { }

    // IndexableValue:
    virtual afl::data::Value* get(interpreter::Arguments& args)
        {
            size_t index;
            if (fetchIndex(index, args, m_list->list)) {
                return new ReferenceContext(m_list->list[index], m_session);
            } else {
                return 0;
            }
        }

    virtual void set(interpreter::Arguments& args, afl::data::Value* value)
        {
            size_t index;
            if (!fetchIndex(index, args, m_list->list)) {
                throw interpreter::Error::typeError(interpreter::Error::ExpectInteger);
            }

            Reference ref;
            if (!checkReferenceArg(ref, value)) {
                throw interpreter::Error::typeError();
            }

            m_list->list.set(index, ref);
        }

    // CallableValue:
    virtual int32_t getDimension(int32_t which) const
        { return (which == 0 ? 1 : int32_t(m_list->list.size())); }
    virtual Context* makeFirstContext()
        { return m_list->list.size() != 0 ? new IterableReferenceContext(m_list, m_session, 0) : 0; }
    virtual CallableValue* clone() const
        { return new ObjectArrayValue(m_list, m_session); }

    // BaseValue:
    virtual String_t toString(bool /*readable*/) const
        { return "#<array>"; }
    virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
        { rejectStore(out, aux, ctx); }

 private:
    afl::base::Ref<Data> m_list;
    Session& m_session;
};

/*
 *  ReferenceListContext
 */

game::interface::ReferenceListContext::ReferenceListContext(afl::base::Ref<Data> list, Session& session)
    : SingleContext(),
      m_list(list),
      m_session(session)
{
}

game::interface::ReferenceListContext::~ReferenceListContext()
{ }

interpreter::Context::PropertyAccessor*
game::interface::ReferenceListContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, REFLIST_MAP, result) ? this : 0;
}

afl::data::Value*
game::interface::ReferenceListContext::get(PropertyIndex_t index)
{
    switch (ReferenceListProperty(REFLIST_MAP[index].index)) {
     case ilpAdd:
        return new ProcedureValue(m_list, m_session, IFReferenceList_Add);
     case ilpAddObjects:
        return new ProcedureValue(m_list, m_session, IFReferenceList_AddObjects);
     case ilpAddObjectsAt:
        return new ProcedureValue(m_list, m_session, IFReferenceList_AddObjectsAt);
     case ilpObjects:
        return new ObjectArrayValue(m_list, m_session);
    }
    return 0;
}

game::interface::ReferenceListContext*
game::interface::ReferenceListContext::clone() const
{
    return new ReferenceListContext(m_list, m_session);
}

game::map::Object*
game::interface::ReferenceListContext::getObject()
{
    return 0;
}

void
game::interface::ReferenceListContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(REFLIST_MAP);
}

String_t
game::interface::ReferenceListContext::toString(bool /*readable*/) const
{
    return "#<reference-list>";
}

void
game::interface::ReferenceListContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

const game::ref::List&
game::interface::ReferenceListContext::getList() const
{
    return m_list->list;
}

/*
 *  Interface Functions
 */

/* @q Add ref:Reference, ... (Reference List Operation)
   Add one or more references to the reference list.
   @see Reference(), LocationReference()
   @since PCC2 2.40.7 */
void
game::interface::IFReferenceList_Add(game::ref::List& list, Session& /*session*/, interpreter::Arguments& args)
{
    args.checkArgumentCountAtLeast(1);
    while (args.getNumArgs() > 0) {
        Reference r;
        if (checkReferenceArg(r, args.getNext())) {
            list.add(r);
        }
    }
}

/* @q AddObjects kind:Str, id:Int, ... (Reference List Operation)
   Add one or more object references to the reference list.
   The %kind parameter specifies the object kinds (see {Reference()}), followed by a set of Ids.
   @since PCC2 2.40.7 */
void
game::interface::IFReferenceList_AddObjects(game::ref::List& list, Session& /*session*/, interpreter::Arguments& args)
{
    args.checkArgumentCountAtLeast(2);
    String_t typeStr;
    if (!checkStringArg(typeStr, args.getNext())) {
        return;
    }

    Reference::Type type;
    if (!parseReferenceTypeName(typeStr, type)) {
        throw interpreter::Error::rangeError();
    }

    while (args.getNumArgs() > 0) {
        int32_t id;
        if (checkIntegerArg(id, args.getNext(), 0, MAX_REFERENCE_ID)) {
            list.add(Reference(type, id));
        }
    }
}

/* @q AddObjectsAt x:Int, y:Int, Optional flags:Str (Reference List Operation)
   Add all ships and planets at a given location to the reference list.

   Flags can be a combination of:
   - "f" (include foreign ships; default is own ships only)
   - "p" (include the planet; default is ships only)
   - "s" (include safe ships only; default is also include guessed ships)
   - a ship Id (exclude that ship)
   @since PCC2 2.40.7 */
void
game::interface::IFReferenceList_AddObjectsAt(game::ref::List& list, Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(2, 3);

    // Location
    int32_t x, y;
    if (!checkIntegerArg(x, args.getNext(), 0, MAX_NUMBER) || !checkIntegerArg(y, args.getNext(), 0, MAX_NUMBER)) {
        return;
    }

    // Flags
    int32_t flags = 0;
    int32_t excludeShipId = 0;
    checkFlagArg(flags, &excludeShipId, args.getNext(), "FPS");

    List::Options_t opts;
    if (flags & 1) {
        opts += List::IncludeForeignShips;
    }
    if (flags & 2) {
        opts += List::IncludePlanet;
    }
    if (flags & 4) {
        opts += List::SafeShipsOnly;
    }

    // Environment
    Game& g = game::actions::mustHaveGame(session);
    if (Turn* t = g.getViewpointTurn().get()) {
        // getViewpointTurn()==0 cannot happen normally, so no need to generate an error for now
        list.addObjectsAt(t->universe(), g.mapConfiguration().getCanonicalLocation(game::map::Point(x, y)), opts, excludeShipId);
    }
}

/* @q ReferenceList():Obj (Function)
   Create a reference list.
   The reference list is initially empty.
   You can add objects to and iterate it using {@group Reference List Operation|reference list functions}. */
afl::data::Value*
game::interface::IFReferenceList(game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);

    afl::base::Ref<ReferenceListContext::Data> list(*new ReferenceListContext::Data);
    return new ReferenceListContext(list, session);
}
