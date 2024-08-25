/**
  *  \file game/interface/missionlistcontext.cpp
  *  \brief Class game::interface::MissionListContext
  */

#include "game/interface/missionlistcontext.hpp"
#include "afl/io/constmemorystream.hpp"
#include "game/interface/missioncontext.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/process.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/world.hpp"

using afl::base::Ref;
using game::interface::MissionContext;
using game::spec::Mission;
using game::spec::MissionList;

namespace {
    enum MissionListProperty {
        impAddMissionCommand,
        impMissionFunction
    };

    const interpreter::NameTable TABLE[] = {
        { "ADDMISSION", impAddMissionCommand, 0, interpreter::thProcedure },
        { "MISSION",    impMissionFunction,   0, interpreter::thFunction },
    };


    /*
     *  Implementation of the "Mission()" function.
     */

    /* @q Mission(index:Int):Obj (MissionList Operation)
       Access ship mission properties.
       Use as
       | ForEach list->Mission Do ...
       or
       | With list->Mission(index) Do ...

       Note that while the {Mission (Function)|global Mission() function} accesses missions by number
       (e.g. 10 = Cloak mission), the MissionList operation accesses missions by list position
       (e.g. 0 = first).

       @since PCC2 2.41.2 */
    class ListFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit ListFunction(const Ref<MissionList>& data)
            : m_data(data)
            { }

        // IndexableValue:
        virtual MissionContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual size_t getDimension(size_t which) const;
        virtual MissionContext* makeFirstContext();
        virtual ListFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Ref<MissionList> m_data;
    };


    /*
     *  Implementation of the "AddMission" command
     */

    /* @q AddMission msn:Any (MissionList Operation)
       Add a new mission definition.
       The parameter can be:
       - a mission, i.e. the result of {Mission (Function)|Mission() function}
         or {Mission (MissionList Operation)|Mission() of a MissionList}.
       - a mission definition as a string.
         The string can contain one or more mission definitions in the same format as in file mission.cc.
         For example, <tt>AddMission "9,+2,Hiss\nc=Beam"</tt> defines the Hiss mission.

       @since PCC2 2.41.2 */
    void IFMissionList_AddMission(const Ref<MissionList>& state, interpreter::Process& proc, interpreter::Arguments& args)
    {
        args.checkArgumentCount(1);

        afl::data::Value* p = args.getNext();
        if (p == 0) {
            // ok, ignore
        } else if (MissionContext* ctx = dynamic_cast<MissionContext*>(p)) {
            // It's a mission
            if (const Mission* msn = ctx->getMission()) {
                state->addMission(*msn);
            }
        } else {
            // Expect string; parse as file
            String_t str;
            interpreter::checkStringArg(str, p);
            afl::io::ConstMemoryStream ms(afl::string::toBytes(str));
            state->loadFromFile(ms, proc.world().logListener(), proc.world().translator());
        }
    }
}

game::interface::MissionContext*
ListFunction::get(interpreter::Arguments& args)
{
    args.checkArgumentCount(1);

    size_t index;
    if (!interpreter::checkIndexArg(index, args.getNext(), 0, m_data->size())) {
        return 0;
    }

    return new MissionContext(index, m_data);
}

void
ListFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

// CallableValue:
size_t
ListFunction::getDimension(size_t which) const
{
    if (which == 0) {
        return 1;
    } else {
        return m_data->size();
    }
}

game::interface::MissionContext*
ListFunction::makeFirstContext()
{
    if (m_data->at(0) != 0) {
        return new MissionContext(0, m_data);
    } else {
        return 0;
    }
}

ListFunction*
ListFunction::clone() const
{
    return new ListFunction(m_data);
}

// BaseValue:
String_t
ListFunction::toString(bool /*readable*/) const
{
    return "#<array:Mission>";
}

void
ListFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}



/*
 *  MissionListContext
 */

game::interface::MissionListContext::MissionListContext(const afl::base::Ref<game::spec::MissionList>& data)
    : m_data(data)
{ }

game::interface::MissionListContext::~MissionListContext()
{ }

// ReadOnlyAccessor:
afl::data::Value*
game::interface::MissionListContext::get(PropertyIndex_t index)
{
    switch (MissionListProperty(TABLE[index].index)) {
     case impAddMissionCommand:
        return new interpreter::SimpleProcedure<Ref<MissionList>, const Ref<MissionList>&>(m_data, IFMissionList_AddMission);
     case impMissionFunction:
        return new ListFunction(m_data);
    }
    return 0;
}

// Context:
interpreter::Context::PropertyAccessor*
game::interface::MissionListContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, TABLE, result) ? this : 0;
}

game::interface::MissionListContext*
game::interface::MissionListContext::clone() const
{
    return new MissionListContext(m_data);
}

afl::base::Deletable*
game::interface::MissionListContext::getObject()
{
    return 0;
}

void
game::interface::MissionListContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(TABLE);
}

// BaseValue:
String_t
game::interface::MissionListContext::toString(bool /*readable*/) const
{
    return "#<MissionList>";
}

void
game::interface::MissionListContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}


/* @q MissionList():MissionList (Function)
   Create an empty mission list.

   @since PCC2 2.41.2 */
afl::data::Value*
game::interface::IFMissionList(interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    return new MissionListContext(game::spec::MissionList::create());
}
