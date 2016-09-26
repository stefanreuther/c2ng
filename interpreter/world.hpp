/**
  *  \file interpreter/world.hpp
  */
#ifndef C2NG_INTERPRETER_WORLD_HPP
#define C2NG_INTERPRETER_WORLD_HPP

#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"
#include "afl/container/ptrmap.hpp"
#include "util/keymaptable.hpp"
#include "afl/sys/loglistener.hpp"
#include "interpreter/objectpropertyvector.hpp"
#include "interpreter/processlist.hpp"
#include "afl/base/ptr.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"

namespace interpreter {

    class SpecialCommand;
    class PropertyAcceptor;
    class Context;
    class Error;

    class World {
     public:
        // Ship properties
        static const afl::data::NameMap::Index_t sp_Comment = 0;

        // Planet Properties
        static const afl::data::NameMap::Index_t pp_Comment = 0;


        World(afl::sys::LogListener& log, afl::io::FileSystem& fs);
        ~World();

        void setNewGlobalValue(const char* name, afl::data::Value* value);

        afl::data::NameMap& globalPropertyNames();
        const afl::data::NameMap& globalPropertyNames() const;
        afl::data::NameMap& shipPropertyNames();
        const afl::data::NameMap& shipPropertyNames() const;
        afl::data::NameMap& planetPropertyNames();
        const afl::data::NameMap& planetPropertyNames() const;
        afl::data::Segment& globalValues();
        const afl::data::Segment& globalValues() const;
        ObjectPropertyVector& shipProperties();
        const ObjectPropertyVector& shipProperties() const;
        ObjectPropertyVector& planetProperties();
        const ObjectPropertyVector& planetProperties() const;

        void addNewSpecialCommand(const char* name, SpecialCommand* newCmd);
        SpecialCommand* lookupSpecialCommand(String_t name) const;
        void enumSpecialCommands(PropertyAcceptor& acceptor) const;

        util::KeymapTable& keymaps();
        const util::KeymapTable& keymaps() const;

        util::AtomTable& atomTable();
        const util::AtomTable& atomTable() const;

        ProcessList& processList();
        const ProcessList& processList() const;

        void addNewGlobalContext(Context* ctx);
        const afl::container::PtrVector<Context>& globalContexts() const;

        void setLoadDirectory(afl::base::Ptr<afl::io::Directory> dir);
        afl::base::Ptr<afl::io::Directory> getLoadDirectory() const;

        afl::sys::LogListener& logListener();
        void logError(afl::sys::LogListener::Level level, const Error& e);

        afl::io::FileSystem& fileSystem();

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

        // ex int/process.cc:process_list
        ProcessList m_processList;

        // replacement for console
        afl::sys::LogListener& m_log;

        // replacement for global file system
        afl::io::FileSystem& m_fileSystem;

        // ex game_file_dir, sort-of
        afl::base::Ptr<afl::io::Directory> m_loadDirectory;

        void init();
    };

}

#endif
