/**
  *  \file interpreter/world.hpp
  *  \brief Class interpreter::World
  */
#ifndef C2NG_INTERPRETER_WORLD_HPP
#define C2NG_INTERPRETER_WORLD_HPP

#include "afl/base/ptr.hpp"
#include "afl/container/ptrmap.hpp"
#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/filetable.hpp"
#include "interpreter/mutexlist.hpp"
#include "interpreter/objectpropertyvector.hpp"
#include "util/keymaptable.hpp"

namespace interpreter {

    class SpecialCommand;
    class PropertyAcceptor;
    class Context;
    class Error;

    /** Interpreter root element.
        Contains all state for an interpreter session except for game data and user-interface bindings
        (see game::Session for that).

        World provides storage for global and object properties.
        These include
        - predefined ship/planet properties (CreateShipProperty, CreatePlanetProperty)
        - predefined global variables (A..Z, SYSTEM.ERR) and functions (mutex, file)
        - other global properties (keymaps, atoms, special commands)

        World does <em>not</em> provide a global context to access these items.
        This context must be provided by the parent object (game::Session provides game::interface::GlobalContext);
        World just provides a means to manage such global contexts that are duplicated into each process.
        The reason is that there must be a single all-encompassing global context to maintain consistent serialisation between PCC2 and c2ng.

        World contains values that were global in PCC2. */
    class World {
     public:
        /*
         *  Property Indexes
         */

        /** Ship property: comment. */
        static const afl::data::NameMap::Index_t sp_Comment = 0;

        /** Planet property: comment. */
        static const afl::data::NameMap::Index_t pp_Comment = 0;


        /*
         *  Constructor
         */

        /** Constructor.
            \param log Logger (used to log interpreter messages and Print output)
            \param tx Translator (for error messages)
            \param fs File system (used to access files) */
        World(afl::sys::LogListener& log, afl::string::Translator& tx, afl::io::FileSystem& fs);

        /** Destructor. */
        ~World();


        /*
         *  Property Access
         */

        /** Set a global value.
            If a global value of this name already exists, it is overwritten.
            Otherwise, a new variable is created.
            \param name Name of value
            \param value Newly-allocated value */
        void setNewGlobalValue(const char* name, afl::data::Value* value);

        /** Get a global value.
            \param name Name of value
            \return Value */
        const afl::data::Value* getGlobalValue(const char* name) const;

        /** Access global property names.
            \return global property names */
        afl::data::NameMap& globalPropertyNames();

        /** Access global property names (const).
            \return global property names */
        const afl::data::NameMap& globalPropertyNames() const;

        /** Access ship property names.
            \return ship property names */
        afl::data::NameMap& shipPropertyNames();

        /** Access ship property names (const).
            \return ship property names */
        const afl::data::NameMap& shipPropertyNames() const;

        /** Access planet property names.
            \return planet property names */
        afl::data::NameMap& planetPropertyNames();

        /** Access planet property names (const).
            \return planet property names */
        const afl::data::NameMap& planetPropertyNames() const;

        /** Access global values.
            \return global values */
        afl::data::Segment& globalValues();

        /** Access global values (const).
            \return global values */
        const afl::data::Segment& globalValues() const;

        /** Access ship properties.
            \return ship properties */
        ObjectPropertyVector& shipProperties();

        /** Access ship properties (const).
            \return ship properties */
        const ObjectPropertyVector& shipProperties() const;

        /** Access planet properties.
            \return planet properties */
        ObjectPropertyVector& planetProperties();
        /** Access planet properties (const).
            \return planet properties */
        const ObjectPropertyVector& planetProperties() const;

        /** Define a special command.
            \param name Name of the command, upper-case
            \param newCmd Newly-allocated SpecialCommand descendant implementing the command */
        void addNewSpecialCommand(const char* name, SpecialCommand* newCmd);

        /** Look up special command.
            \param name Name of command
            \return SpecialCommand descendant that compiles this command. 0 if this is not a known special command. */
        SpecialCommand* lookupSpecialCommand(String_t name) const;

        /** Enumerate special commands.
            \param acceptor Callback */
        void enumSpecialCommands(PropertyAcceptor& acceptor) const;

        /** Access keymaps.
            \return keymap table */
        util::KeymapTable& keymaps();

        /** Access keymaps (const).
            \return keymap table */
        const util::KeymapTable& keymaps() const;

        /** Access atoms.
            \return atom table */
        util::AtomTable& atomTable();

        /** Access atoms (const).
            \return atom table */
        const util::AtomTable& atomTable() const;

        /** Access mutexes.
            \return mutex list */
        MutexList& mutexList();

        /** Access mutexes (const).
            \return mutex list */
        const MutexList& mutexList() const;

        /** Access files.
            \return file table */
        FileTable& fileTable();

        /** Access files (const).
            \return file table */
        const FileTable& fileTable() const;

        /** Add new global context.
            The context is added to the globalContexts() object where it can be retrieved for copying into new processes.
            \param ctx Newly-allocated Context */
        void addNewGlobalContext(Context* ctx);

        /** Access global contexts.
            \return list of global contexts */
        const afl::container::PtrVector<Context>& globalContexts() const;

        /** Set system load directory.
            Files opened with "Load" are checked here if not found in the local load directory.
            \param dir Directory; can be null */
        void setSystemLoadDirectory(afl::base::Ptr<afl::io::Directory> dir);

        /** Get system load directory.
            \return directory as set with setSystemLoadDirectory() */
        afl::base::Ptr<afl::io::Directory> getSystemLoadDirectory() const;

        /** Set local load directory.
            Files opened with "Load" are checked here first.
            \param dir Directory; can be null */
        void setLocalLoadDirectory(afl::base::Ptr<afl::io::Directory> dir);

        /** Get local load directory.
            \return directory as set with setLocalLoadDirectory() */
        afl::base::Ptr<afl::io::Directory> getLocalLoadDirectory() const;

        /** Open file for loading.
            Checks the file in
            - the local load directory
            - the global load directory
            - the file system's default directory
            \param name File name
            \return File opened for reading if found; null otherwise */
        afl::base::Ptr<afl::io::Stream> openLoadFile(const String_t name) const;

        /** Access logger.
            \return logger */
        afl::sys::LogListener& logListener();

        /** Log an error.
            Writes the error and it's optional trace onto the logger.
            \param level Log level for error message (the trace is always written as Info)
            \param e Error */
        void logError(afl::sys::LogListener::Level level, const Error& e);

        /** Access translator.
            \return translator */
        afl::string::Translator& translator() const;

        /** Access file system.
            \return file system */
        afl::io::FileSystem& fileSystem();

        /** Compile a file.
            Compiles a file into a new bytecode object.
            The bytecode is independent from the execution context and can be executed when desired.
            (World is needed for logging, file access, and special commands.)
            \param file File to compile
            \param origin Origin
            \param level Optimisation level
            \return newly-allocated byte code
            \throw Error on error */
        BCORef_t compileFile(afl::io::Stream& file, const String_t& origin, int level);

        /** Compile a command.
            This is a shortcut to compile a fire-and-forget command that does not produce a result.
            \param command Command to execute
            \return newly-allocated byte code
            \throw Error on error
            \see compileCommand(String_t,bool,bool&). */
        BCORef_t compileCommand(String_t command);

        /** Compile a command.
            The bytecode is independent from the execution context and can be executed when desired.
            (World is needed for logging, file access, and special commands.)
            \param command [in] Command to execute
            \param wantResult [in] true if we anticipate this command to produce a result (i.e. console), false for fire-and-forget execution
            \param hasResult [out] true if the command produces a result
            \return newly-allocated byte code
            \throw Error on error */
        BCORef_t compileCommand(String_t command, bool wantResult, bool& hasResult);

        /** Notify listeners.
            Call notifyListeners() on all sub-objects that have one. */
        void notifyListeners();

     private:
        // ex int/intglobal.h:global_property_names
        afl::data::NameMap m_globalPropertyNames;

        // ex int/intglobal.h:ship_property_names
        afl::data::NameMap m_shipPropertyNames;

        // ex int/intglobal.h:planet_property_names
        afl::data::NameMap m_planetPropertyNames;

        // ex int/intglobal.h:global_values
        afl::data::Segment m_globalValues;

        // ex int/intglobal.h:ship_properties
        ObjectPropertyVector m_shipProperties;

        // ex int/intglobal.h:planet_properties
        ObjectPropertyVector m_planetProperties;

        // ex int/special.cc:special_commands
        afl::container::PtrMap<String_t,SpecialCommand> m_specialCommands;

        // ex int/keymap.cc:keymaps
        util::KeymapTable m_keymaps;

        // ex util/atom.cc:tab
        util::AtomTable m_atomTable;

        // ex IntExecutionContext::global_context
        afl::container::PtrVector<Context> m_globalContexts;

        // ex int/mutex.h globals
        // Must be before ProcessList because processes reference it in destructor
        MutexList m_mutexList;

        // ex int/file.h globals
        FileTable m_fileTable;

        // replacement for console
        afl::sys::LogListener& m_log;

        // translator
        afl::string::Translator& m_translator;

        // replacement for global file system
        afl::io::FileSystem& m_fileSystem;

        // ex game_file_dir, sort-of
        afl::base::Ptr<afl::io::Directory> m_systemLoadDirectory;
        afl::base::Ptr<afl::io::Directory> m_localLoadDirectory;

        void init();
    };

}

#endif
