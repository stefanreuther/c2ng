/**
  *  \file game/interface/globalactionextra.cpp
  *  \brief Class game::interface::GlobalActionExtra
  */

#include "game/interface/globalactionextra.hpp"
#include "game/interface/simpleprocedure.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"

using interpreter::CallableValue;
using interpreter::Error;

namespace {
    // Extra Identifier
    const game::ExtraIdentifier<game::Session, game::interface::GlobalActionExtra> LABEL_ID = {{}};

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

    /* @q AddGlobalAction name:Str, prepare:Func, exec:Sub, result:Sub (Global Command)
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

       @since PCC2 2.40.13 */
    void IFAddGlobalAction(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
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

        // Access extra
        game::interface::GlobalActionExtra* extra = game::interface::GlobalActionExtra::get(session);
        if (extra == 0) {
            throw Error::contextError();
        }

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
        size_t id = extra->actions().addAction(prepareCallable, execCallable, resultCallable);

        // Check for two-way castability
        int32_t treeId = static_cast<int32_t>(id + 1);
        size_t recoveredId = static_cast<size_t>(treeId - 1);
        if (treeId == 0 || recoveredId != id) {
            throw Error::rangeError();
        }

        // Add to tree
        extra->actionNames().addPath(treeId, path, util::TreeList::root);
    }
}

/*
 *  GlobalActionExtra
 */

game::interface::GlobalActionExtra::GlobalActionExtra(Session& session)
{
    session.world().setNewGlobalValue("ADDGLOBALACTION", new SimpleProcedure(session, IFAddGlobalAction));
}

game::interface::GlobalActionExtra::~GlobalActionExtra()
{ }

game::interface::GlobalActionExtra&
game::interface::GlobalActionExtra::create(Session& session)
{
    GlobalActionExtra* p = session.extra().get(LABEL_ID);
    if (p == 0) {
        p = session.extra().setNew(LABEL_ID, new GlobalActionExtra(session));
    }
    return *p;
}

game::interface::GlobalActionExtra*
game::interface::GlobalActionExtra::get(Session& session)
{
    return session.extra().get(LABEL_ID);
}

game::interface::GlobalActions&
game::interface::GlobalActionExtra::actions()
{
    return m_actions;
}

const game::interface::GlobalActions&
game::interface::GlobalActionExtra::actions() const
{
    return m_actions;
}

util::TreeList&
game::interface::GlobalActionExtra::actionNames()
{
    return m_actionNames;
}

const util::TreeList&
game::interface::GlobalActionExtra::actionNames() const
{
    return m_actionNames;
}
