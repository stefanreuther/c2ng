/**
  *  \file game/interface/configurationeditorcontext.cpp
  *  \brief Class game::interface::ConfigurationEditorContext
  */

#include "game/interface/configurationeditorcontext.hpp"
#include "game/root.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplefunction.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/values.hpp"

/*
 *  Imports
 */

using game::interface::ConfigurationEditorContext;
using interpreter::BytecodeObject;
using interpreter::CallableValue;
using interpreter::Error;
using interpreter::Opcode;
using interpreter::SimpleFunction;
using interpreter::SimpleProcedure;

const int game::interface::ConfigurationEditorContext::ScriptEditor;

namespace {
    /*
     *  Class Declarations
     */

    /** Implementation of ConfigurationEditor::Node for scripting.
        Among others, stores a subroutine to modify the value using arbitrary user interaction,
        and a function to retrieve the current value.

        We cannot invoke scripts directly.
        Thus, the value reported in getValue() is cached and not computed on demand.

        Driver code must make sure to call the functions at appropriate times. */
    class ScriptedNode : public game::config::ConfigurationEditor::Node {
     public:
        /** Constructor.
            @param level          Level of node
            @param name           Name of node
            @param editSub        Subroutine to edit the value; will be cloned
            @param valueFunction  Function to get current value; will be cloned */
        ScriptedNode(int level, const String_t& name, const CallableValue& editSub, const CallableValue& valueFunction);

        // Node:
        virtual int getType();
        virtual String_t getValue(const game::config::Configuration& config, afl::string::Translator& tx);
        virtual void enumOptions(game::config::Configuration& config, afl::base::Closure<void(game::config::ConfigurationOption&)>& fcn);

        /** Add affected option (LinkPref command).
            @param optionName Name */
        void addOptionName(const String_t& optionName);

        /** Set cached value.
            @param cachedValue New value */
        void setCachedValue(const String_t& cachedValue);

        /** Set extra value.
            @param extraValue New value, ScriptedNode will assume ownership. Can be null */
        void setNewExtraValue(afl::data::Value* extraValue);

        /** Get name of first option.
            Essentially, performs the same operation as getFirstOption(),
            but more efficient and without needing a Configuration.
            @return name; can be null. May become invalid on next modification. */
        const String_t* getFirstOptionName() const;

        /** Get extra value.
            @return value, owned by ScriptedNode */
        afl::data::Value* getExtraValue() const;

        /** Get edit subroutine.
            @return subroutine */
        CallableValue* getEditSub() const;

        /** Get value function.
            @return function */
        CallableValue* getValueFunction() const;

     private:
        std::auto_ptr<CallableValue> m_editSub;
        std::auto_ptr<CallableValue> m_valueFunction;
        std::vector<String_t> m_optionNames;
        String_t m_cachedValue;
        std::auto_ptr<afl::data::Value> m_extraValue;
    };

    /** Symbolic reference to a ScriptedNode. */
    class NodeRef {
     public:
        /** Constructor.
            @param d Reference to ConfigurationEditor::Data object
            @param i Index into Data::editor */
        NodeRef(const ConfigurationEditorContext::DataRef& d, size_t i)
            : m_data(d), m_index(i)
            { }

        /** Get ScriptedNode instance.
            @return ScriptedNode instance; can be null */
        ScriptedNode* get() const
            { return dynamic_cast<ScriptedNode*>(m_data.ref->editor.getNodeByIndex(m_index)); }
     private:
        ConfigurationEditorContext::DataRef m_data;
        size_t m_index;
    };

    /** Context for a single node (configuration option).
        The editSub and valueFunction are evaluated in an instance of this context. */
    class NodeContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param ref Node reference */
        explicit NodeContext(const NodeRef& ref);

        // PropertyAccessor:
        virtual afl::data::Value* get(PropertyIndex_t index);

        // Context:
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual NodeContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        NodeRef m_ref;
    };

    /*
     *  Helpers
     */

    // Helper to check type of a CallableValue parameter [FIXME: dup from GlobalActionContext]
    const CallableValue& requireCallable(const afl::data::Value* value)
    {
        const CallableValue* cv = dynamic_cast<const CallableValue*>(value);
        if (cv == 0) {
            throw Error::typeError(Error::ExpectCallable);
        }
        return *cv;
    }

    // Add component to a string list [FIXME: dup from GlobalActionContext]
    void addComponent(std::vector<String_t>& out, const String_t& in)
    {
        String_t value = afl::string::strTrim(in);
        if (!value.empty()) {
            out.push_back(value);
        }
    }

    // Parse a name into a path [FIXME: dup from GlobalActionContext]
    std::vector<String_t> parsePath(const String_t& name)
    {
        std::vector<String_t> path;
        size_t i = 0, n;
        while ((n = name.find('|', i)) != String_t::npos) {
            addComponent(path, name.substr(i, n-i));
            i = n+1;
        }
        addComponent(path, name.substr(i));
        if (path.empty()) {
            throw Error("Option name cannot be empty");
        }
        return path;
    }


    typedef SimpleProcedure<ConfigurationEditorContext::DataRef, const ConfigurationEditorContext::DataRef&> DataProcedure_t;
    typedef SimpleFunction<ConfigurationEditorContext::DataRef, const ConfigurationEditorContext::DataRef&> DataFunction_t;

    void IFConfigurationEditor_Notify(const ConfigurationEditorContext::DataRef& state, interpreter::Process& proc, interpreter::Arguments& args);
    void IFConfigurationEditorNode_SetValue(const NodeRef& ref, interpreter::Process& proc, interpreter::Arguments& args);

    // Generate code for updating a single node
    void compileUpdater(BytecodeObject& bco, const ConfigurationEditorContext::DataRef& d, size_t index)
    {
        // Generated code as pseudo-code:
        //   Try ctx->SetValue (With ctx Do updateFcn())
        NodeRef ref(d, index);
        if (ScriptedNode* n = ref.get()) {
            NodeContext ctx(ref);
            SimpleProcedure<NodeRef, const NodeRef&> update(ref, IFConfigurationEditorNode_SetValue);
            BytecodeObject::Label_t lcatch = bco.makeLabel();
            BytecodeObject::Label_t lend   = bco.makeLabel();

            bco.addJump(Opcode::jCatch, lcatch);
            bco.addPushLiteral(&ctx);
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
            bco.addPushLiteral(n->getValueFunction());
            bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 0);
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
            bco.addPushLiteral(&update);
            bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 1);
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
            bco.addJump(Opcode::jAlways, lend);
            bco.addLabel(lcatch);
            bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            bco.addLabel(lend);
        }
    }

    // Generate code for forwarding all updates to the ConfigurationEditor
    void compileForwarder(BytecodeObject& bco, const ConfigurationEditorContext::DataRef& d)
    {
        // Just call the Notify function.
        DataProcedure_t update(d, IFConfigurationEditor_Notify);
        bco.addPushLiteral(&update);
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0);
    }

    /*
     *  Script Interface Functions
     */

    /* @q Add name:Str, edit:Sub, value:Func (Configuration Editor Command)
       Add an editable configuration option.

       The name is a string containing the name of the option.
       Options are presented as a tree; use "|" to separate components.

       The other two parameters are names of functions/subroutines defined as follows:
       | Sub edit()
       | Function value()
       (Just pass the names of the functions, do not pass the function names as strings!)

       The value function is called to determine the current value of the option as a string.
       The edit subroutine is called when the user chooses to edit the option.
       It shall update the option, and may use any user interaction it requires.

       @since PCC2 2.41 */
    void IFConfigurationEditor_Add(const ConfigurationEditorContext::DataRef& state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        // Add "tree position", EditSub, ValueFunction
        args.checkArgumentCount(3);

        // Check name
        String_t name;
        if (!interpreter::checkStringArg(name, args.getNext())) {
            return;
        }

        // Check other args
        const afl::data::Value* editValue  = args.getNext();
        const afl::data::Value* valueValue = args.getNext();
        if (!editValue || !valueValue) {
            return;
        }

        const CallableValue& editCallable  = requireCallable(editValue);
        const CallableValue& valueCallable = requireCallable(valueValue);

        // Parse the name into a path [FIXME: dup from GlobalActionContext]
        std::vector<String_t> path = parsePath(name);

        // Create
        const int32_t treeId = ConfigurationEditorContext::getTreeIdFromEditorIndex(state.ref->editor.getNumNodes());
        state.ref->editor.addNewNode(new ScriptedNode(0, "", editCallable, valueCallable));
        state.ref->optionNames.addPath(treeId, path, state.root);
    }

    /* @q Subtree(name:Str):Obj (Configuration Editor Command)
       Create a new Configuration Editor Context for a subtree of the option tree.
       All additions to the created context will add to the subtree, not to the root.
       For example, the following two commands are identical:
       | Add "Options | Plugin | Option", My.Edit, My.Value
       | With Subtree("Options") Do Add "Plugin | Option", My.Edit, My.Value

       @since PCC2 2.41 */
    afl::data::Value* IFConfigurationEditor_Subtree(const ConfigurationEditorContext::DataRef& state, interpreter::Arguments& args)
    {
        args.checkArgumentCount(1);
        String_t name;
        if (!interpreter::checkStringArg(name, args.getNext())) {
            return 0;
        }

        std::vector<String_t> path = parsePath(name);
        size_t nodeId = state.ref->optionNames.addPath(0, path, state.root);
        return new ConfigurationEditorContext(state.ref, nodeId);
    }

    /* @q LinkPref name:Str... (Configuration Editor Command)
       Link preference entry with most-recently-added option.

       When this command is called after {Add (Configuration Editor Command)|Add},
       the given preference entries (pcc2.ini values) are associated with the option.
       This will cause the configuration editor to offer changing the storage location
       for this option (user or game-specific pcc2.ini file).

       For convenience, the first added preference name is also available as %Option
       during the edit/value callbacks.
       @since PCC2 2.41 */
    void IFConfigurationEditor_LinkPref(const ConfigurationEditorContext::DataRef& state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        // LinkPref "option", "option..."
        args.checkArgumentCountAtLeast(1);

        ScriptedNode* node = dynamic_cast<ScriptedNode*>(state.ref->editor.getNodeByIndex(state.ref->editor.getNumNodes() - 1));
        if (node == 0) {
            throw Error::contextError();
        }

        while (args.getNumArgs() > 0) {
            String_t optionName;
            if (interpreter::checkStringArg(optionName, args.getNext())) {
                node->addOptionName(optionName);
            }
        }
    }

    /* @q LinkExtra value:Any (Configuration Editor Command)
       Link an extra value to the most-recently-added option.

       When this command is called after {Add (Configuration Editor Command)|Add},
       it associates an arbitrary value with the option.

       This value is available as %Extra during the edit/value callbacks.

       You can use values of any type.
       Thus, if you need multiple values, use a structure or array.

       @since PCC2 2.41 */
    void IFConfigurationEditor_LinkExtra(const ConfigurationEditorContext::DataRef& state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        // LinkExtra value
        args.checkArgumentCount(1);

        ScriptedNode* node = dynamic_cast<ScriptedNode*>(state.ref->editor.getNodeByIndex(state.ref->editor.getNumNodes() - 1));
        if (node == 0) {
            throw Error::contextError();
        }

        node->setNewExtraValue(afl::data::Value::cloneOf(args.getNext()));
    }

    /* @q UpdateAll (Configuration Editor Command)
       Update all option values.

       This calls all options' value function and publishes the values to the user interface.
       You normally do not have to call this function in your own callbacks.

       @since PCC2 2.41 */
    void IFConfigurationEditor_UpdateAll(const ConfigurationEditorContext::DataRef& state, interpreter::Process& proc, interpreter::Arguments& args)
    {
        // UpdateAll
        args.checkArgumentCount(0);

        // Compiler update for all values
        interpreter::BCORef_t bco = BytecodeObject::create(true);
        for (size_t i = 0, n = state.ref->editor.getNumNodes(); i < n; ++i) {
            compileUpdater(*bco, state, i);
        }
        compileForwarder(*bco, state);

        // Inject into running process
        proc.pushFrame(bco, false);
    }

    // Notify (internal, not published): forward all accumulated changes
    // @since PCC2 2.41
    void IFConfigurationEditor_Notify(const ConfigurationEditorContext::DataRef& state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        // Nodify (internal, not published)
        args.checkArgumentCount(0);
        if (game::Root* r = state.ref->session.getRoot().get()) {
            state.ref->editor.updateValues(r->userConfiguration(), state.ref->session.translator());
        }
    }

    // SetValue value:Str (internal, not published): update a single value
    // @since PCC2 2.41
    void IFConfigurationEditorNode_SetValue(const NodeRef& ref, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        args.checkArgumentCount(1);

        String_t theValue;
        if (interpreter::checkStringArg(theValue, args.getNext())) {
            if (ScriptedNode* n = ref.get()) {
                n->setCachedValue(theValue);
            }
        }
    }


    /*
     *  Property Tables
     */

    enum ConfigurationEditorProperty {
        cepAdd,
        cepLinkExtra,
        cepLinkPref,
        cepSubtree,
        cepUpdateAll
    };

    const interpreter::NameTable CE_TABLE[] = {
        { "ADD",       cepAdd,       0, interpreter::thProcedure },
        { "LINKEXTRA", cepLinkExtra, 0, interpreter::thProcedure },
        { "LINKPREF",  cepLinkPref,  0, interpreter::thProcedure },
        { "SUBTREE",   cepSubtree,   0, interpreter::thFunction },
        { "UPDATEALL", cepUpdateAll, 0, interpreter::thProcedure },
    };

    enum NodeProperty {
        npExtra,
        npOption
    };

    const interpreter::NameTable NP_TABLE[] = {
        { "EXTRA",  npExtra,  0, interpreter::thNone },
        { "OPTION", npOption, 0, interpreter::thString },
    };
}


/*
 *  ScriptedNode
 */

ScriptedNode::ScriptedNode(int level, const String_t& name, const CallableValue& editSub, const CallableValue& valueFunction)
    : Node(level, name),
      m_editSub(editSub.clone()),
      m_valueFunction(valueFunction.clone()),
      m_optionNames(),
      m_cachedValue(),
      m_extraValue()
{ }

int
ScriptedNode::getType()
{
    return ConfigurationEditorContext::ScriptEditor;
}

String_t
ScriptedNode::getValue(const game::config::Configuration& /*config*/, afl::string::Translator& /*tx*/)
{
    return m_cachedValue;
}

void
ScriptedNode::enumOptions(game::config::Configuration& config, afl::base::Closure<void(game::config::ConfigurationOption&)>& fcn)
{
    for (size_t i = 0, n = m_optionNames.size(); i < n; ++i) {
        if (game::config::ConfigurationOption* p = config.getOptionByName(m_optionNames[i])) {
            fcn.call(*p);
        }
    }
}

void
ScriptedNode::addOptionName(const String_t& optionName)
{
    m_optionNames.push_back(optionName);
}

void
ScriptedNode::setCachedValue(const String_t& cachedValue)
{
    m_cachedValue = cachedValue;
}

void
ScriptedNode::setNewExtraValue(afl::data::Value* extraValue)
{
    m_extraValue.reset(extraValue);
}

const String_t*
ScriptedNode::getFirstOptionName() const
{
    return m_optionNames.empty() ? 0 : &m_optionNames.front();
}

afl::data::Value*
ScriptedNode::getExtraValue() const
{
    return m_extraValue.get();
}

CallableValue*
ScriptedNode::getEditSub() const
{
    return m_editSub.get();
}

CallableValue*
ScriptedNode::getValueFunction() const
{
    return m_valueFunction.get();
}


/*
 *  NodeContext
 */

NodeContext::NodeContext(const NodeRef& ref)
    : m_ref(ref)
{ }

afl::data::Value*
NodeContext::get(PropertyIndex_t index)
{
    if (ScriptedNode* n = m_ref.get()) {
        switch (NodeProperty(NP_TABLE[index].index)) {
         case npExtra:
            return afl::data::Value::cloneOf(n->getExtraValue());

         case npOption:
            if (const String_t* p = n->getFirstOptionName()) {
                return interpreter::makeStringValue(*p);
            } else {
                return 0;
            }
        }
    }
    return 0;
}

interpreter::Context::PropertyAccessor*
NodeContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (interpreter::lookupName(name, NP_TABLE, result)) {
        return this;
    } else {
        return 0;
    }
}

NodeContext*
NodeContext::clone() const
{
    return new NodeContext(m_ref);
}

afl::base::Deletable*
NodeContext::getObject()
{
    return 0;
}

void
NodeContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(NP_TABLE);
}

String_t
NodeContext::toString(bool /*readable*/) const
{
    return "#<ConfigurationEditorContext.Node>";
}

void
NodeContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

/*
 *  ConfigurationEditorContext
 */

game::interface::ConfigurationEditorContext::ConfigurationEditorContext(Session& session)
    : m_data(*new Data(session), util::TreeList::root)
{ }

game::interface::ConfigurationEditorContext::ConfigurationEditorContext(const afl::base::Ref<Data>& ref, size_t root)
    : m_data(ref, root)
{ }

game::interface::ConfigurationEditorContext::~ConfigurationEditorContext()
{ }

// ReadOnlyAccessor:
afl::data::Value*
game::interface::ConfigurationEditorContext::get(PropertyIndex_t index)
{
    switch (ConfigurationEditorProperty(CE_TABLE[index].index)) {
     case cepAdd:       return new DataProcedure_t(m_data, IFConfigurationEditor_Add);
     case cepLinkPref:  return new DataProcedure_t(m_data, IFConfigurationEditor_LinkPref);
     case cepLinkExtra: return new DataProcedure_t(m_data, IFConfigurationEditor_LinkExtra);
     case cepSubtree:   return new DataFunction_t (m_data, IFConfigurationEditor_Subtree);
     case cepUpdateAll: return new DataProcedure_t(m_data, IFConfigurationEditor_UpdateAll);
    }
    return 0;
}

// Context:
interpreter::Context::PropertyAccessor*
game::interface::ConfigurationEditorContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (interpreter::lookupName(name, CE_TABLE, result)) {
        return this;
    } else {
        return 0;
    }
}

game::interface::ConfigurationEditorContext*
game::interface::ConfigurationEditorContext::clone() const
{
    return new ConfigurationEditorContext(*this);
}

afl::base::Deletable*
game::interface::ConfigurationEditorContext::getObject()
{
    return 0;
}

void
game::interface::ConfigurationEditorContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(CE_TABLE);
}

// BaseValue:
String_t
game::interface::ConfigurationEditorContext::toString(bool /*readable*/) const
{
    return "#<ConfigurationEditor>";
}

void
game::interface::ConfigurationEditorContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

void
game::interface::ConfigurationEditorContext::compileEditor(interpreter::BytecodeObject& bco, size_t index) const
{
    // Edit: Try With ctx Do editSub
    NodeRef ref(m_data, index);
    if (ScriptedNode* n = ref.get()) {
        NodeContext ctx(ref);
        BytecodeObject::Label_t lcatch = bco.makeLabel();
        BytecodeObject::Label_t lend   = bco.makeLabel();

        bco.addJump(Opcode::jCatch, lcatch);
        bco.addPushLiteral(&ctx);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        bco.addPushLiteral(n->getEditSub());
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
        bco.addJump(Opcode::jAlways, lend);
        bco.addLabel(lcatch);
        bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
        bco.addLabel(lend);
    }

    // Update the changed value
    compileUpdater(bco, m_data, index);

    // Forward changed value(s) to observers
    compileForwarder(bco, m_data);
}

size_t
game::interface::ConfigurationEditorContext::getEditorIndexFromTreeId(int32_t id)
{
    return static_cast<size_t>(id-1);
}

int32_t
game::interface::ConfigurationEditorContext::getTreeIdFromEditorIndex(size_t index)
{
    int32_t treeId = static_cast<int32_t>(index + 1);
    size_t recoveredIndex = static_cast<size_t>(treeId - 1);
    if (treeId == 0 || recoveredIndex != index) {
        throw Error::rangeError();
    }

    return treeId;
}


/*
 *  Main Entry Point
 */

/* @q ConfigurationEditorContext():Obj (Function)
   Create a Configuration Editor Context.
   @see Add (Configuration Editor Command), int:index:group:configurationeditorcommand|Configuration Editor Commands
   @since PCC2 2.41 */
afl::data::Value*
game::interface::IFConfigurationEditorContext(Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    return new ConfigurationEditorContext(session);
}
