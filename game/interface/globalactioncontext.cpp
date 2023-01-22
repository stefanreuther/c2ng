/**
  *  \file game/interface/globalactioncontext.cpp
  *  \brief Class game::interface::GlobalActionContext
  */

#include "game/interface/globalactioncontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/typehint.hpp"

using interpreter::CallableValue;
using interpreter::Error;

namespace {
    // Helper to check type of a CallableValue parameter
    const CallableValue& requireCallable(const afl::data::Value* value)
    {
        const CallableValue* cv = dynamic_cast<const CallableValue*>(value);
        if (cv == 0) {
            throw Error::typeError(Error::ExpectCallable);
        }
        return *cv;
    }

    // Add component to a string list
    void addComponent(std::vector<String_t>& out, const String_t& in)
    {
        String_t value = afl::string::strTrim(in);
        if (!value.empty()) {
            out.push_back(value);
        }
    }

    /* @q Add name:Str, prepare:Func, exec:Sub, result:Sub (Global Action Context)
       Add a Global Action.

       The name is a string containing the name of the action.
       Actions are presented as a tree; use "|" to separate components.
       For example, "Friendly Codes | Randomize" will create an action "Randomize" below a "Friendly Codes" node.

       The other three parameters are names of functions/subroutines defined as follows:
       | Function prepare()
       | Sub exec(obj,state)
       | Sub result(state,globalstate)
       (Just pass the names of the functions, do not pass the function names as strings!)

       When the user chooses to execute this action, the prepare function is called to set up.
       It shall ask the user for parameters, and return a state value.
       If it returns EMPTY, the action is aborted.

       Then, the exec function is called for each object, passing it the object and the state value.

       Finally, the result function is called with the state value, and a globalstate object generated internally.
       The globalstate contains these attributes:
       - NumSuccess: integer, number of objects successfully processed
       - NumErrors: integer, number of objects where exec threw an error
       - Error: if exec threw an error, last error message

       @since PCC2 2.41
       @see GlobalActions (Hook) */
    void IFGlobalAction_Add(afl::base::Ref<game::interface::GlobalActionContext::Data> state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        args.checkArgumentCount(4);

        // Check name
        String_t name;
        if (!interpreter::checkStringArg(name, args.getNext())) {
            return;
        }

        // Check other args
        const afl::data::Value* prepareValue = args.getNext();
        const afl::data::Value* execValue    = args.getNext();
        const afl::data::Value* resultValue  = args.getNext();
        if (!prepareValue || !execValue || !resultValue) {
            return;
        }

        const CallableValue& prepareCallable = requireCallable(prepareValue);
        const CallableValue& execCallable    = requireCallable(execValue);
        const CallableValue& resultCallable  = requireCallable(resultValue);

        // Parse the name into a path
        std::vector<String_t> path;
        size_t i = 0, n;
        while ((n = name.find('|', i)) != String_t::npos) {
            addComponent(path, name.substr(i, n-i));
            i = n+1;
        }
        addComponent(path, name.substr(i));
        if (path.empty()) {
            throw Error("Action name cannot be empty");
        }

        // Create
        size_t id = state->actions.addAction(prepareCallable, execCallable, resultCallable);

        // Check for two-way castability
        int32_t treeId = static_cast<int32_t>(id + 1);
        size_t recoveredId = static_cast<size_t>(treeId - 1);
        if (treeId == 0 || recoveredId != id) {
            throw Error::rangeError();
        }

        // Add to tree
        state->actionNames.addPath(treeId, path, util::TreeList::root);
    }

    /*
     *  Property Mapping
     */
    enum {
        piAdd
    };

    const interpreter::NameTable TABLE[] = {
        { "ADD", piAdd, 0, interpreter::thProcedure },
    };
}


/*
 *  GlobalActionContext
 */

game::interface::GlobalActionContext::GlobalActionContext()
    : SingleContext(),
      m_data(*new Data())
{ }

game::interface::GlobalActionContext::~GlobalActionContext()
{ }

// ReadOnlyAccessor:
afl::data::Value*
game::interface::GlobalActionContext::get(PropertyIndex_t index)
{
    switch (TABLE[index].index) {
     case piAdd:
        return new interpreter::SimpleProcedure<afl::base::Ref<Data> >(m_data, IFGlobalAction_Add);
    }
    return 0;
}

// Context:
interpreter::Context::PropertyAccessor*
game::interface::GlobalActionContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (interpreter::lookupName(name, TABLE, result)) {
        return this;
    } else {
        return 0;
    }
}

game::interface::GlobalActionContext*
game::interface::GlobalActionContext::clone() const
{
    return new GlobalActionContext(*this);
}

afl::base::Deletable*
game::interface::GlobalActionContext::getObject()
{
    return 0;
}

void
game::interface::GlobalActionContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(TABLE);
}

// BaseValue:
String_t
game::interface::GlobalActionContext::toString(bool /*readable*/) const
{
    return "#<GlobalActions>";
}

void
game::interface::GlobalActionContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

/* @q GlobalActionContext():Obj (Function)
   Create a Global Action Context.

   @see Add (Global Action Context), int:index:group:globalactioncontext|Global Action Context, GlobalActions (Hook)
   @since PCC2 2.41 */
afl::data::Value*
game::interface::IFGlobalActionContext(interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    return new GlobalActionContext();
}
