/**
  *  \file game/interface/configurationcontext.cpp
  *  \brief Class game::interface::ConfigurationContext
  */

#include "game/interface/configurationcontext.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/format.hpp"
#include "game/config/booleanvalueparser.hpp"
#include "game/config/configurationparser.hpp"
#include "game/config/costarrayoption.hpp"
#include "game/config/genericintegerarrayoption.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/stringarrayoption.hpp"
#include "game/config/stringoption.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplefunction.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/values.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::string::Format;
using game::config::Configuration;
using game::config::ConfigurationOption;
using game::config::HostConfiguration;
using game::config::UserConfiguration;
using interpreter::Error;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using interpreter::getBooleanValue;
using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeStringValue;

namespace {
    /*
     *  Properties for ConfigurationContext
     */
    enum ConfigurationProperty {
        icpAdd,
        icpCreate,
        icpEntry,
        icpGet,
        icpLoad,
        icpMerge,
        icpSubtract
    };

    const interpreter::NameTable CONFIG_TABLE[] = {
        { "ADD",      icpAdd,      0, interpreter::thProcedure },
        { "CREATE",   icpCreate,   0, interpreter::thProcedure },
        { "ENTRY",    icpEntry,    0, interpreter::thFunction  },
        { "GET",      icpGet,      0, interpreter::thFunction  },
        { "LOAD",     icpLoad,     0, interpreter::thProcedure },
        { "MERGE",    icpMerge,    0, interpreter::thProcedure },
        { "SUBTRACT", icpSubtract, 0, interpreter::thProcedure },
    };

    /*
     *  Properties for ConfigurationContext Entry
     */

    enum KeyProperty {
        ikpName,
        ikpSource,
        ikpValue
    };

    const interpreter::NameTable KEY_TABLE[] = {
        /* @q Name:Str (Configuration Entry Property)
           Name of this configuration entry.
           @since PCC2 2.41.5 */
        { "NAME",     ikpName,     0, interpreter::thString },

        /* @q Source:Int (Configuration Entry Property)
           Source of this configuration entry.
           - 0 = default
           - 1 = system configuration file (global file)
           - 2 = user configuration file (user-specific file, e.g. in home directory)
           - 3 = game configuration file (game-specific file, e.g. in game directory)
           @since PCC2 2.41.5 */
        { "SOURCE",   ikpSource,   0, interpreter::thInt },

        /* @q Value:Str (Configuration Entry Property)
           Value of this configuration entry, as a string.
           For typed access, use {Get (Configuration Function)|Get()}.
           @since PCC2 2.41.5 */
        { "VALUE",    ikpValue,    0, interpreter::thString },
    };

    /*
     *  Utilities
     */

    void setDependantOptions(const game::interface::ConfigurationContext::Data& state)
    {
        if (HostConfiguration* p = dynamic_cast<HostConfiguration*>(&*state.config)) {
            p->setDependantOptions();
        }
    }

    afl::data::Value* makeScalarValue(int32_t value, const game::config::ValueParser& parser)
    {
        if ((value == 0 || value == 1) && (dynamic_cast<const game::config::BooleanValueParser*>(&parser) != 0)) {
            return makeBooleanValue(value);
        } else {
            return makeIntegerValue(value);
        }
    }
}

/*
 *  KeyContext - a Context representing a single key, with optional enumeration
 *
 *  Note that all KeyContext instances derived from the same origin share their iterator.
 *  This means that if a KeyContext is cloned and next() called on both copies, their state will get out of sync.
 *  Normal script code cannot do that.
 *  Script code can take a copy of KeyContext in each state...
 *     ForEach cfg->Entry As a Do Array.Push as, a
 *  ...but cannot call next() on them.
 *  (The same problem also applies to DirectoryContext)
 */

class game::interface::ConfigurationContext::KeyContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
 public:
    struct State : public afl::base::RefCounted {
        // Configuration
        Ref<Configuration> config;

        // Non-null iterator if this KeyContext is iterable
        Ptr<Configuration::Enumerator_t> iter;

        // Name of current option.
        // We do NOT keep the ConfigurationOption* pointer that we get from the enumerator.
        // Instead, we look it up fresh all the time.
        // This will make iteration O(nlogn) instead of O(n), but will allow parallel modification.
        // Parallel modification may change the type of an option and therefore invalidate pointers.
        String_t name;

        State(const Ref<Configuration>& config)
            : config(config), iter(), name()
            { }
    };

    explicit KeyContext(const Ref<State>& state);

    // Context:
    virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
    virtual afl::data::Value* get(PropertyIndex_t index);
    virtual bool next();
    virtual interpreter::Context* clone() const;
    virtual afl::base::Deletable* getObject();
    virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

    // BaseValue:
    virtual String_t toString(bool readable) const;
    virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

 private:
    Ref<State> m_state;
};

game::interface::ConfigurationContext::KeyContext::KeyContext(const Ref<State>& state)
    : m_state(state)
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::ConfigurationContext::KeyContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (interpreter::lookupName(name, KEY_TABLE, result)) {
        return this;
    } else {
        return 0;
    }
}

afl::data::Value*
game::interface::ConfigurationContext::KeyContext::get(PropertyIndex_t index)
{
    ConfigurationOption* opt = m_state->config->getOptionByName(m_state->name);
    if (opt != 0) {
        switch (index) {
         case ikpName:
            return makeStringValue(m_state->name);
         case ikpSource:
            return makeIntegerValue(opt->getSource());
         case ikpValue:
            return makeStringValue(opt->toString());
        }
    }
    return 0;
}

bool
game::interface::ConfigurationContext::KeyContext::next()
{
    if (m_state->iter.get() != 0) {
        Configuration::OptionInfo_t opt;
        if (m_state->iter->getNextElement(opt)) {
            m_state->name = opt.first;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

interpreter::Context*
game::interface::ConfigurationContext::KeyContext::clone() const
{
    return new KeyContext(m_state);
}

afl::base::Deletable*
game::interface::ConfigurationContext::KeyContext::getObject()
{
    return 0;
}

void
game::interface::ConfigurationContext::KeyContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(KEY_TABLE);
}

// BaseValue:
String_t
game::interface::ConfigurationContext::KeyContext::toString(bool /*readable*/) const
{
    return "#<ConfigurationKey>";
}

void
game::interface::ConfigurationContext::KeyContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

/*
 *  EntryFunction - Value for the "Entry" attribute (indexable/iterable)
 */

/* @q Entry(key:Str):ConfigEntry (Configuration Function)
   Access the properties of a configuration file entry.
   If the given key does not exist in the configuration, yields EMPTY.

   This function can also be used as
   | ForEach cfg->Entry Do ...
   to iterate over all entries in a configuration.

   @since PCC2 2.41.5 */
class game::interface::ConfigurationContext::EntryFunction : public interpreter::IndexableValue {
 public:
    explicit EntryFunction(const Data& data)
        : m_data(data)
        { }

    // IndexableValue:
    virtual KeyContext* get(interpreter::Arguments& args);
    virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

    // CallableValue:
    virtual size_t getDimension(size_t which) const;
    virtual KeyContext* makeFirstContext();
    virtual EntryFunction* clone() const;

    // BaseValue:
    virtual String_t toString(bool readable) const;
    virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

 private:
    Data m_data;
};

game::interface::ConfigurationContext::KeyContext*
game::interface::ConfigurationContext::EntryFunction::get(interpreter::Arguments& args)
{
    args.checkArgumentCount(1);

    String_t key;
    if (!checkStringArg(key, args.getNext())) {
        return 0;
    }

    ConfigurationOption* opt = m_data.config->getOptionByName(key);
    if (opt == 0) {
        return 0;
    }

    // Context will not have an iterator and therefore not be iterable
    // (not contractual, but script code will not call next() on a directly-obtained context)
    Ref<KeyContext::State> state = *new KeyContext::State(m_data.config);
    state->name = key;
    return new KeyContext(state);
}

void
game::interface::ConfigurationContext::EntryFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    return rejectSet(args, value);
}

size_t
game::interface::ConfigurationContext::EntryFunction::getDimension(size_t /*which*/) const
{
    return 0;
}

game::interface::ConfigurationContext::KeyContext*
game::interface::ConfigurationContext::EntryFunction::makeFirstContext()
{
    Ref<Configuration::Enumerator_t> iter = m_data.config->getOptions();
    Configuration::OptionInfo_t info;
    if (iter->getNextElement(info)) {
        Ref<KeyContext::State> state = *new KeyContext::State(m_data.config);
        state->iter = iter.asPtr();
        state->name = info.first;
        return new KeyContext(state);
    } else {
        return 0;
    }
}

game::interface::ConfigurationContext::EntryFunction*
game::interface::ConfigurationContext::EntryFunction::clone() const
{
    return new EntryFunction(m_data);
}

String_t
game::interface::ConfigurationContext::EntryFunction::toString(bool /*readable*/) const
{
    return "#<ConfigurationEntry>";
}

void
game::interface::ConfigurationContext::EntryFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}


/*
 *  ConfigurationContext
 */

game::interface::ConfigurationContext::ConfigurationContext(Session& session, const afl::base::Ref<game::config::Configuration>& config)
    : m_data(session, config)
{ }

/** Destructor. */
game::interface::ConfigurationContext::~ConfigurationContext()
{ }

game::config::Configuration&
game::interface::ConfigurationContext::config()
{
    return *m_data.config;
}

// ReadOnlyAccessor:
afl::data::Value*
game::interface::ConfigurationContext::get(PropertyIndex_t index)
{
    typedef interpreter::SimpleFunction<Data, const Data&> Function_t;
    typedef interpreter::SimpleProcedure<Data, const Data&> Procedure_t;

    switch (index) {
     case icpAdd:      return new Procedure_t(m_data, IFConfiguration_Add);
     case icpCreate:   return new Procedure_t(m_data, IFConfiguration_Create);
     case icpEntry:    return new EntryFunction(m_data);
     case icpGet:      return new Function_t (m_data, IFConfiguration_Get);
     case icpLoad:     return new Procedure_t(m_data, IFConfiguration_Load);
     case icpMerge:    return new Procedure_t(m_data, IFConfiguration_Merge);
     case icpSubtract: return new Procedure_t(m_data, IFConfiguration_Subtract);
    }
    return 0;
}

// Context:
interpreter::Context::PropertyAccessor*
game::interface::ConfigurationContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (interpreter::lookupName(name, CONFIG_TABLE, result)) {
        return this;
    } else {
        return 0;
    }
}

game::interface::ConfigurationContext*
game::interface::ConfigurationContext::clone() const
{
    return new ConfigurationContext(m_data.session, m_data.config);
}

afl::base::Deletable*
game::interface::ConfigurationContext::getObject()
{
    return 0;
}

void
game::interface::ConfigurationContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(CONFIG_TABLE);
}

// BaseValue:
String_t
game::interface::ConfigurationContext::toString(bool /*readable*/) const
{
    return "#<Configuration>";
}

void
game::interface::ConfigurationContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

game::interface::ConfigurationContext*
game::interface::ConfigurationContext::check(afl::data::Value* value)
{
    if (value == 0) {
        return 0;
    }

    ConfigurationContext* ctx = dynamic_cast<ConfigurationContext*>(value);
    if (ctx == 0) {
        throw Error::typeError();
    }

    return ctx;
}


/*
 *  Public Entry Points
 */

/* @q Configuration(Optional kind:Int):Config (Function)
   Create a configuration object.

   Creates a blank, independent configuration object.
   This object can be used to process configuration files ("ini files", "pconfig.src").
   Modifying it will not directly affect PCC2.

   If the parameter not specified or 0, creates a generic, empty configuration.

   If the parameter is specified as 1, creates an empty host configuration (pconfig.src).
   All values are present with correct type and default values.
   The {Get (Configuration Function)|Get} function's second parameter will default to the current player, if known.

   If the parameter is specified as 2, creates an empty user configuration (pcc2.ini).
   All values are present with correct type and default values.

   @see System.Cfg, System.Pref */
afl::data::Value*
game::interface::IFConfiguration(Session& session, interpreter::Arguments& args)
{
    int32_t flavor = 0;
    args.checkArgumentCount(0, 1);
    interpreter::checkIntegerArg(flavor, args.getNext(), 0, 2);

    switch (flavor) {
     case 0:
        return new ConfigurationContext(session, Configuration::create());
     case 1:
        return new ConfigurationContext(session, HostConfiguration::create());
     case 2:
        return new ConfigurationContext(session, UserConfiguration::create());
    }
    return 0;
}

/* @q Add line:Str (Configuration Command)
   Modify this configuration.
   %line is a configuration assignment as it could appear in a configuration file, e.g. "NumShips=500",
   This command will process the line, and update the configuration.

   If the option is known, the value will be handled according to its known type.
   For example, the value for %NumShips needs to be a number.

   If the option is not known, it will be created anew as a plain string option.

   @see AddConfig, AddPref, Create (Configuration Command)
   @since PCC2 2.41.5 */
void
game::interface::IFConfiguration_Add(const ConfigurationContext::Data& state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // Add key:Str, value:Str
    // Parse args
    args.checkArgumentCount(1);
    String_t text;
    if (!checkStringArg(text, args.getNext())) {
        return;
    }

    // Parse
    String_t::size_type n = text.find('=');
    if (n == String_t::npos) {
        throw Error("Invalid configuration setting");
    }

    // Assign the option.
    // We need not verify that this option exists, it will be created.
    state.config->setOption(afl::string::strTrim(String_t(text, 0, n)), afl::string::strTrim(String_t(text, n+1)), ConfigurationOption::User);
    setDependantOptions(state);
}

/* @q Create key:Str, type:Str (Configuration Command)
   Create a new configuration option.

   %key is the name of the option.

   %type is the type of the value.
   Supported types are:
   - "int"/"integer": a number
   - "str"/"string": a string
   - "bool"/"boolean": a boolean value (yes/no)
   The type affects acceptable values for the option, and the return type produced by {Get()}.

   @see CreateConfigOption, CreatePrefOption
   @see Add

   @since PCC2 2.41.5 */
void
game::interface::IFConfiguration_Create(const ConfigurationContext::Data& state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // Parse args
    String_t key;
    String_t type;
    args.checkArgumentCount(2);
    if (!checkStringArg(key, args.getNext()) || !checkStringArg(type, args.getNext())) {
        return;
    }

    // Create the option by indexing with an appropriate descriptor
    Configuration& config = *state.config;
    if (type == "str" || type == "string") {
        game::config::StringOptionDescriptor desc;
        desc.m_name = key.c_str();
        config[desc];
    } else if (type == "int" || type == "integer") {
        game::config::IntegerOptionDescriptor desc;
        desc.m_name = key.c_str();
        desc.m_parser = &game::config::IntegerValueParser::instance;
        config[desc];
    } else if (type == "bool" || type == "boolean") {
        game::config::IntegerOptionDescriptor desc;
        desc.m_name = key.c_str();
        desc.m_parser = &game::config::BooleanValueParser::instance;
        config[desc];
    } else {
        throw Error::rangeError();
    }
}

/* @q Load fd:File, Optional section:Str, default:Bool (Configuration Command)
   Load configuration file.
   If the section parameter is given, load only the specified section;
   if default is true, treat options before the first section delimiter as being part of that section.

   @since PCC2 2.41.5 */
void
game::interface::IFConfiguration_Load(const ConfigurationContext::Data& state, interpreter::Process& proc, interpreter::Arguments& args)
{
    // Load #fd:File, Optional sec:String, default:Bool

    // Parse args
    args.checkArgumentCount(1, 3);

    // - Mandatory file
    afl::io::TextFile* tf = 0;
    if (!proc.world().fileTable().checkFileArg(tf, args.getNext())) {
        return;
    }

    // - Optional section, flag
    String_t sectionName;
    bool hasSection = checkStringArg(sectionName, args.getNext());
    bool isInSection = getBooleanValue(args.getNext());

    // Config parser
    game::config::ConfigurationParser parser(state.session.log(), state.session.translator(), *state.config, ConfigurationOption::User);
    if (hasSection) {
        parser.setSection(sectionName, isInSection);
    }
    parser.parseTextFile(*tf);
    setDependantOptions(state);
}

/* @q Merge other:Config (Configuration Command)
   Merge into another configuration.
   For set each option in %other (source different from Default (0)),
   updates this configuration with that value.

   @since PCC2 2.41.5 */
void
game::interface::IFConfiguration_Merge(const ConfigurationContext::Data& state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // Merge other:Config
    args.checkArgumentCount(1);

    ConfigurationContext* other = ConfigurationContext::check(args.getNext());
    if (other == 0) {
        return;
    }

    state.config->merge(other->config());
    setDependantOptions(state);
}

/* @q Subtract other:Config (Configuration Command)
   Remove options equal to another configuration by setting them to "default".

   Given
   | Call me->Subtract them
   Then, if %me contains an option with the same value as the same option in %them,
   this will set that option's source to Default (0),
   as an indication that
   | Call them->Merge me
   will not modify that option.

   @since PCC2 2.41.5 */
void
game::interface::IFConfiguration_Subtract(const ConfigurationContext::Data& state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // Subtract other:Config
    args.checkArgumentCount(1);

    ConfigurationContext* other = ConfigurationContext::check(args.getNext());
    if (other == 0) {
        return;
    }

    if (&other->config() == &*state.config) {
        throw Error("\"Subtract\" cannot be used to remove on configuration from itself");
    }

    state.config->subtract(other->config());
}

/* @q Get(key:Str, Optional index:Int):Any (Configuration Function)
   The first parameter is the name of the option to retrieve.
   The function returns the value of this option, an integer, boolean or string.

   If the option is an array option, the second parameter must be specified as the index into the array,
   starting at 1.
   If this function is called on a host configuration, and the option is a per-player option,
   the second parameter defaults to the current player if not specified.

   @see Cfg(), Pref()

   @since PCC2 2.41.5 */
afl::data::Value*
game::interface::IFConfiguration_Get(const ConfigurationContext::Data& state, interpreter::Arguments& args)
{
    // Get(key:Str, Optional index:Int)
    const char*const functionName = "Get";  // FIXME: proper function name (via Arguments?)

    // Parse args
    args.checkArgumentCount(1, 2);

    // Config key
    String_t optName;
    if (!checkStringArg(optName, args.getNext())) {
        return 0;
    }

    // Player number
    afl::base::Optional<int32_t> player;
    if (args.getNumArgs() > 0) {
        int p;
        if (!checkIntegerArg(p, args.getNext())) {
            return 0;
        }
        player = p;
    }

    // Viewpoint player number for host config access
    afl::base::Optional<int32_t> viewpointPlayer;
    if (Game* g = state.session.getGame().get()) {
        if (dynamic_cast<HostConfiguration*>(&*state.config) != 0) {
            viewpointPlayer = g->getViewpointPlayer();
        }
    }

    // Fetch option
    // (Unlike PCC2, resolve the alias first, so we automatically deal with badly-configured aliases.)
    const ConfigurationOption* opt = state.config->getOptionByName(optName);
    if (const game::config::AliasOption* alias = dynamic_cast<const game::config::AliasOption*>(opt)) {
        opt = alias->getForwardedOption();
    }
    if (!opt) {
        throw Error(Format("Invalid first argument to \"%s\"", functionName));
    }

    if (const game::config::GenericIntegerArrayOption* bopt = dynamic_cast<const game::config::GenericIntegerArrayOption*>(opt)) {
        // Integers; optional player
        int index;
        if (!player.get(index)) {
            /* Possible limits are
                 2    NewNativesPopulationRange
                 4    WraparoundRectangle
                 8    MeteorShowerOreRanges
                 9    NewNativesRaceRate
                 10   ConfigExpOption, e.g. EModBayRechargeRate
                 11   ConfigStdOption, e.g. RaceMiningRate
               \change c2ng has MAX_PLAYERS instead of 11, but otherwise, the logic remains the same. */
            if (bopt->getArray().size() == size_t(game::MAX_PLAYERS) && viewpointPlayer.get(index)) {
                // ok
            } else {
                throw Error::tooFewArguments(functionName);
            }
        }
        if (const int32_t* p = bopt->getArray().at(index - 1)) {
            return makeScalarValue(*p, bopt->parser());
        } else {
            throw Error::rangeError();
        }
    } else if (const game::config::IntegerOption* intopt = dynamic_cast<const game::config::IntegerOption*>(opt)) {
        // single int, no player. Example: NumShips
        if (player.isValid()) {
            throw Error::tooManyArguments(functionName);
        }
        return makeScalarValue((*intopt)(), intopt->parser());
    } else if (const game::config::CostArrayOption* costopt = dynamic_cast<const game::config::CostArrayOption*>(opt)) {
        // Array of costs. Example: StarbaseCost
        int index;
        if (!player.get(index)) {
            if (!viewpointPlayer.get(index)) {
                throw Error::tooFewArguments(functionName);
            }
        }
        if (index <= 0 || index >= game::MAX_PLAYERS) {
            throw Error::rangeError();
        }
        return makeStringValue((*costopt)(index).toCargoSpecString());
    } else if (const game::config::StringArrayOption* saopt = dynamic_cast<const game::config::StringArrayOption*>(opt)) {
        // String array, applies to Language, ExperienceLevelNames. Parameter must be given.
        int index;
        if (player.get(index)) {
            if (index < saopt->getFirstIndex() || index >= saopt->getFirstIndex() + saopt->getNumSlots()) {
                throw Error::rangeError();
            }
            return makeStringValue((*saopt)(index));
        } else {
            return makeStringValue((*saopt).toString());
        }
    } else {
        // Anything else (including StringOption): just return the value.
        if (player.isValid()) {
            throw Error::tooManyArguments(functionName);
        }
        return makeStringValue(opt->toString());
    }
}
