/**
  *  \file interpreter/world.cpp
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


interpreter::World::World(afl::sys::LogListener& log, afl::io::FileSystem& fs)
    : m_globalPropertyNames(),
      m_shipPropertyNames(),
      m_planetPropertyNames(),
      m_globalValues(),
      m_specialCommands(),
      m_keymaps(),
      m_atomTable(),
      m_globalContexts(),
      m_processList(),
      m_mutexList(),
      m_log(log),
      m_fileSystem(fs),
      m_systemLoadDirectory(),
      m_localLoadDirectory()
{
    init();
}

interpreter::World::~World()
{ }

/** Define a global value.
    \param name Name of the value
    \param proc value */
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

afl::data::NameMap&
interpreter::World::globalPropertyNames()
{
    return m_globalPropertyNames;
}

const afl::data::NameMap&
interpreter::World::globalPropertyNames() const
{
    return m_globalPropertyNames;
}

afl::data::NameMap&
interpreter::World::shipPropertyNames()
{
    return m_shipPropertyNames;
}

const afl::data::NameMap&
interpreter::World::shipPropertyNames() const
{
    return m_shipPropertyNames;
}

afl::data::NameMap&
interpreter::World::planetPropertyNames()
{
    return m_planetPropertyNames;
}

const afl::data::NameMap&
interpreter::World::planetPropertyNames() const
{
    return m_planetPropertyNames;
}

afl::data::Segment&
interpreter::World::globalValues()
{
    return m_globalValues;
}

const afl::data::Segment&
interpreter::World::globalValues() const
{
    return m_globalValues;
}

interpreter::ObjectPropertyVector&
interpreter::World::shipProperties()
{
    return m_shipProperties;
}

const interpreter::ObjectPropertyVector&
interpreter::World::shipProperties() const
{
    return m_shipProperties;
}

interpreter::ObjectPropertyVector&
interpreter::World::planetProperties()
{
    return m_planetProperties;
}

const interpreter::ObjectPropertyVector&
interpreter::World::planetProperties() const
{
    return m_planetProperties;
}


/** Define a special command.
    \param name   [in] Name of the command, upper-case
    \param newCmd [in] Newly-created SpecialCommand descendant object */
void
interpreter::World::addNewSpecialCommand(const char* name, SpecialCommand* newCmd)
{
    // FIXME: exception safety...
    m_specialCommands.insertNew(name, newCmd);
}

/** Look up special command.
    \param name Name of command
    \return IntSpecialCommand descendant that compiles this command. 0 if this is not a special command. */
interpreter::SpecialCommand*
interpreter::World::lookupSpecialCommand(String_t name) const
{
    return m_specialCommands[name];
}

void
interpreter::World::enumSpecialCommands(PropertyAcceptor& acceptor) const
{
    // ex int/special.cc:enumSpecialCommands
    for (afl::container::PtrMap<String_t,SpecialCommand>::iterator i = m_specialCommands.begin(); i != m_specialCommands.end(); ++i) {
        acceptor.addProperty(i->first, thNone);
    }
}

util::KeymapTable&
interpreter::World::keymaps()
{
    return m_keymaps;
}

const util::KeymapTable&
interpreter::World::keymaps() const
{
    return m_keymaps;
}

util::AtomTable&
interpreter::World::atomTable()
{
    return m_atomTable;
}

const util::AtomTable&
interpreter::World::atomTable() const
{
    return m_atomTable;
}

interpreter::ProcessList&
interpreter::World::processList()
{
    return m_processList;
}

const interpreter::ProcessList&
interpreter::World::processList() const
{
    return m_processList;
}

interpreter::MutexList&
interpreter::World::mutexList()
{
    return m_mutexList;
}

const interpreter::MutexList&
interpreter::World::mutexList() const
{
    return m_mutexList;
}

interpreter::FileTable&
interpreter::World::fileTable()
{
    return m_fileTable;
}

const interpreter::FileTable&
interpreter::World::fileTable() const
{
    return m_fileTable;
}

void
interpreter::World::addNewGlobalContext(Context* ctx)
{
    // ex IntExecutionContext::setNewGlobalContext, sort-of
    m_globalContexts.pushBackNew(ctx);
}

const afl::container::PtrVector<interpreter::Context>&
interpreter::World::globalContexts() const
{
    return m_globalContexts;
}

void
interpreter::World::setSystemLoadDirectory(afl::base::Ptr<afl::io::Directory> dir)
{
    m_systemLoadDirectory = dir;
}

afl::base::Ptr<afl::io::Directory>
interpreter::World::getSystemLoadDirectory() const
{
    return m_systemLoadDirectory;
}

void
interpreter::World::setLocalLoadDirectory(afl::base::Ptr<afl::io::Directory> dir)
{
    m_localLoadDirectory = dir;
}

afl::base::Ptr<afl::io::Directory>
interpreter::World::getLocalLoadDirectory() const
{
    return m_localLoadDirectory;
}

afl::base::Ptr<afl::io::Stream>
interpreter::World::openLoadFile(const String_t name) const
{
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

afl::sys::LogListener&
interpreter::World::logListener()
{
    return m_log;
}

void
interpreter::World::logError(afl::sys::LogListener::Level level, const Error& e)
{
    m_log.write(level, "script.error", e.what());

    String_t trace = e.getTrace();
    if (!trace.empty()) {
        m_log.write(m_log.Info, "script.trace", trace);
    }
}

afl::io::FileSystem&
interpreter::World::fileSystem()
{
    return m_fileSystem;
}

interpreter::BCORef_t
interpreter::World::compileFile(afl::io::Stream& file)
{
    // Generate compilation objects
    afl::io::TextFile tf(file);
    FileCommandSource fcs(tf);
    BCORef_t nbco = *new BytecodeObject();
    nbco->setFileName(file.getName());

    // Compile
    try {
        StatementCompiler sc(fcs);
        DefaultStatementCompilationContext scc(*this);
        scc.withFlag(CompilationContext::LocalContext)
            .withFlag(CompilationContext::ExpressionsAreStatements)
            .withFlag(CompilationContext::LinearExecution);
        sc.compileList(*nbco, scc);
        sc.finishBCO(*nbco, scc);
        return nbco;
    }
    catch (Error& e) {
        fcs.addTraceTo(e, afl::string::Translator::getSystemInstance());
        throw e;
    }
}

interpreter::BCORef_t
interpreter::World::compileCommand(String_t command)
{
    bool hasResult;
    return compileCommand(command, false, hasResult);
}

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

void
interpreter::World::notifyListeners()
{
    m_keymaps.notifyListeners();
}

/** Initialize interpreter globals. Call this once per program invocation. */
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
