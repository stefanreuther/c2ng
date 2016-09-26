/**
  *  \file interpreter/world.cpp
  */

#include "interpreter/world.hpp"
#include "interpreter/specialcommand.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"

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
      m_log(log),
      m_fileSystem(fs),
      m_loadDirectory()
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
interpreter::World::setLoadDirectory(afl::base::Ptr<afl::io::Directory> dir)
{
    m_loadDirectory = dir;
}

afl::base::Ptr<afl::io::Directory>
interpreter::World::getLoadDirectory() const
{
    return m_loadDirectory;
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
}
