/**
  *  \file interpreter/world.cpp
  *  \brief Class interpreter::World
  */

#include "interpreter/world.hpp"
#include "interpreter/specialcommand.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "interpreter/mutexfunctions.hpp"
#include "interpreter/filefunctions.hpp"
#include "afl/io/textfile.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/memorycommandsource.hpp"

const afl::data::NameMap::Index_t interpreter::World::sp_Comment;
const afl::data::NameMap::Index_t interpreter::World::pp_Comment;

// Constructor.
interpreter::World::World(afl::sys::LogListener& log, afl::io::FileSystem& fs)
    : m_globalPropertyNames(),
      m_shipPropertyNames(),
      m_planetPropertyNames(),
      m_globalValues(),
      m_specialCommands(),
      m_keymaps(),
      m_atomTable(),
      m_globalContexts(),
      m_mutexList(),
      m_processList(),
      m_log(log),
      m_fileSystem(fs),
      m_systemLoadDirectory(),
      m_localLoadDirectory()
{
    init();
}

// Destructor.
interpreter::World::~World()
{ }

// Set a global value.
void
interpreter::World::setNewGlobalValue(const char* name, afl::data::Value* value)
{
    // ex int/intglobal.h:defineGlobalValue
    afl::data::NameMap::Index_t index;
    try {
        index = m_globalPropertyNames.addMaybe(name);
    }
    catch (...) {
        delete value;
        throw;
    }
    m_globalValues.setNew(index, value);
}

// Get a global value.
afl::data::Value*
interpreter::World::getGlobalValue(const char* name) const
{
    return m_globalValues[m_globalPropertyNames.getIndexByName(name)];
}

// Access global property names.
afl::data::NameMap&
interpreter::World::globalPropertyNames()
{
    return m_globalPropertyNames;
}

// Access global property names (const).
const afl::data::NameMap&
interpreter::World::globalPropertyNames() const
{
    return m_globalPropertyNames;
}

// Access ship property names.
afl::data::NameMap&
interpreter::World::shipPropertyNames()
{
    return m_shipPropertyNames;
}

// Access ship property names (const).
const afl::data::NameMap&
interpreter::World::shipPropertyNames() const
{
    return m_shipPropertyNames;
}

// Access planet property names.
afl::data::NameMap&
interpreter::World::planetPropertyNames()
{
    return m_planetPropertyNames;
}

// Access planet property names (const).
const afl::data::NameMap&
interpreter::World::planetPropertyNames() const
{
    return m_planetPropertyNames;
}

// Access global values.
afl::data::Segment&
interpreter::World::globalValues()
{
    return m_globalValues;
}

// Access global values (const).
const afl::data::Segment&
interpreter::World::globalValues() const
{
    return m_globalValues;
}

// Access ship properties.
interpreter::ObjectPropertyVector&
interpreter::World::shipProperties()
{
    return m_shipProperties;
}

// Access ship properties (const).
const interpreter::ObjectPropertyVector&
interpreter::World::shipProperties() const
{
    return m_shipProperties;
}

// Access planet properties.
interpreter::ObjectPropertyVector&
interpreter::World::planetProperties()
{
    return m_planetProperties;
}

// Access planet properties (const).
const interpreter::ObjectPropertyVector&
interpreter::World::planetProperties() const
{
    return m_planetProperties;
}


// Define a special command.
void
interpreter::World::addNewSpecialCommand(const char* name, SpecialCommand* newCmd)
{
    // FIXME: exception safety...
    m_specialCommands.insertNew(name, newCmd);
}

// Look up special command.
interpreter::SpecialCommand*
interpreter::World::lookupSpecialCommand(String_t name) const
{
    return m_specialCommands[name];
}

// Enumerate special commands.
void
interpreter::World::enumSpecialCommands(PropertyAcceptor& acceptor) const
{
    // ex int/special.cc:enumSpecialCommands
    for (afl::container::PtrMap<String_t,SpecialCommand>::iterator i = m_specialCommands.begin(); i != m_specialCommands.end(); ++i) {
        acceptor.addProperty(i->first, thNone);
    }
}

// Access keymaps.
util::KeymapTable&
interpreter::World::keymaps()
{
    return m_keymaps;
}

// Access keymaps (const).
const util::KeymapTable&
interpreter::World::keymaps() const
{
    return m_keymaps;
}

// Access atoms.
util::AtomTable&
interpreter::World::atomTable()
{
    return m_atomTable;
}

// Access atoms (const).
const util::AtomTable&
interpreter::World::atomTable() const
{
    return m_atomTable;
}

// Access process list.
interpreter::ProcessList&
interpreter::World::processList()
{
    return m_processList;
}

// Access process list (const).
const interpreter::ProcessList&
interpreter::World::processList() const
{
    return m_processList;
}

// Access mutexes.
interpreter::MutexList&
interpreter::World::mutexList()
{
    return m_mutexList;
}

// Access mutexes (const).
const interpreter::MutexList&
interpreter::World::mutexList() const
{
    return m_mutexList;
}

// Access files.
interpreter::FileTable&
interpreter::World::fileTable()
{
    return m_fileTable;
}

// Access files (const).
const interpreter::FileTable&
interpreter::World::fileTable() const
{
    return m_fileTable;
}

// Add new global context.
void
interpreter::World::addNewGlobalContext(Context* ctx)
{
    // ex IntExecutionContext::setNewGlobalContext, sort-of
    m_globalContexts.pushBackNew(ctx);
}

// Access global contexts.
const afl::container::PtrVector<interpreter::Context>&
interpreter::World::globalContexts() const
{
    return m_globalContexts;
}

//Set system load directory.
void
interpreter::World::setSystemLoadDirectory(afl::base::Ptr<afl::io::Directory> dir)
{
    m_systemLoadDirectory = dir;
}

// Get system load directory.
afl::base::Ptr<afl::io::Directory>
interpreter::World::getSystemLoadDirectory() const
{
    return m_systemLoadDirectory;
}

// Set local load directory.
void
interpreter::World::setLocalLoadDirectory(afl::base::Ptr<afl::io::Directory> dir)
{
    m_localLoadDirectory = dir;
}

// Get local load directory.
afl::base::Ptr<afl::io::Directory>
interpreter::World::getLocalLoadDirectory() const
{
    return m_localLoadDirectory;
}

// Open file for loading.
afl::base::Ptr<afl::io::Stream>
interpreter::World::openLoadFile(const String_t name) const
{
    // ex ccexec.pas:Struc_Load (sort-of)
    // FIXME: this calls openFileNT on a Directory, giving it a possibly relative or absolute path name instead of just a file name.
    // This is the same as in PCC2, and it happens to work due to the way how our Directory descendants are implemented, but it is so far not contractual.
    afl::base::Ptr<afl::io::Stream> result;
    if (result.get() == 0 && m_localLoadDirectory.get() != 0) {
        result = m_localLoadDirectory->openFileNT(name, afl::io::FileSystem::OpenRead);
    }
    if (result.get() == 0 && m_systemLoadDirectory.get() != 0) {
        result = m_systemLoadDirectory->openFileNT(name, afl::io::FileSystem::OpenRead);
    }
    if (result.get() == 0) {
        result = m_fileSystem.openFileNT(name, afl::io::FileSystem::OpenRead);
    }
    return result;
}

// Access logger.
afl::sys::LogListener&
interpreter::World::logListener()
{
    return m_log;
}

// Log an error.
void
interpreter::World::logError(afl::sys::LogListener::Level level, const Error& e)
{
    m_log.write(level, "script.error", e.what());

    String_t trace = e.getTrace();
    if (!trace.empty()) {
        m_log.write(m_log.Info, "script.trace", trace);
    }
}

// Access file system.
afl::io::FileSystem&
interpreter::World::fileSystem()
{
    return m_fileSystem;
}

// Compile a file.
interpreter::BCORef_t
interpreter::World::compileFile(afl::io::Stream& file, const String_t& origin, int level)
{
    // Generate compilation objects
    afl::io::TextFile tf(file);
    FileCommandSource fcs(tf);
    BCORef_t nbco = *new BytecodeObject();
    nbco->setFileName(file.getName());
    nbco->setOrigin(origin);

    // Compile
    try {
        StatementCompiler sc(fcs);
        DefaultStatementCompilationContext scc(*this);
        scc.withFlag(CompilationContext::LocalContext)
            .withFlag(CompilationContext::ExpressionsAreStatements)
            .withFlag(CompilationContext::LinearExecution);
        sc.setOptimisationLevel(level);
        sc.compileList(*nbco, scc);
        sc.finishBCO(*nbco, scc);
        return nbco;
    }
    catch (Error& e) {
        fcs.addTraceTo(e, afl::string::Translator::getSystemInstance());
        throw e;
    }
}

// Compile a command.
interpreter::BCORef_t
interpreter::World::compileCommand(String_t command)
{
    bool hasResult;
    return compileCommand(command, false, hasResult);
}

// Compile a command.
interpreter::BCORef_t
interpreter::World::compileCommand(String_t command, bool wantResult, bool& hasResult)
{
    // Create compilation context
    MemoryCommandSource mcs(command);
    BCORef_t bco = *new BytecodeObject();

    // Compile
    StatementCompiler sc(mcs);
    DefaultStatementCompilationContext scc(*this);
    // scc.withContextProvider(&proc); // FIXME: can we do this? ScriptSide does it, but it requires creating the process beforehand.
    scc.withFlag(CompilationContext::RefuseBlocks);
    scc.withFlag(CompilationContext::LinearExecution);
    if (!wantResult) {
        scc.withFlag(scc.ExpressionsAreStatements);
    }
    StatementCompiler::StatementResult result = sc.compile(*bco, scc);
    sc.finishBCO(*bco, scc);
    hasResult = (result == StatementCompiler::CompiledExpression);
    return bco;
}

// Notify listeners.
void
interpreter::World::notifyListeners()
{
    m_keymaps.notifyListeners();
}

/** Initialize sub-objects. */
void
interpreter::World::init()
{
    // ex int/intglobal.h:initInterpreter

    /* @q Comment:Str (Ship Property, Planet Property)
       User comment.
       This is the comment that can be edited with <kbd>F9</kbd>.
       @assignable */
    m_shipPropertyNames.add("COMMENT");
    m_planetPropertyNames.add("COMMENT");
    for (char c = 'A'; c < 'Z'; ++c) {
        m_globalPropertyNames.add(String_t(1, c));
    }

    /* @q System.Err:Str (Global Variable)
       Error message.
       If a command within a {Try} statement generates an error,
       the error message is stored in this variable.
       The %Else part of the %Try statement can therefore look at the message,
       or re-throw the error using {Abort}.

       If a local variable %System.Err is visible, the error message is stored in that
       instead of the global one.

       @diff In PCC 1.x, %System.Err is a global property; the error message is always
       stored in the global property, and a local %System.Err is ignored.
       @assignable */
    m_globalPropertyNames.add("SYSTEM.ERR");

    registerMutexFunctions(*this);
    registerFileFunctions(*this);
}
