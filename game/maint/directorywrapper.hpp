/**
  *  \file game/maint/directorywrapper.hpp
  */
#ifndef C2NG_GAME_MAINT_DIRECTORYWRAPPER_HPP
#define C2NG_GAME_MAINT_DIRECTORYWRAPPER_HPP

#include "afl/io/directory.hpp"
#include "afl/base/ref.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/textwriter.hpp"

namespace game { namespace maint {

    class DirectoryWrapper : public afl::io::Directory {
     public:
        enum EraseMode {
            PassThroughErase,   // Pass through, no change
            LogErase,           // Pass through and log success in human-readable form
            IgnoreAndLogErase   // Do not execute, log file names
        };

        enum WriteMode {
            PassThroughWrites,  // Pass through, no change
            IgnoreWrites        // Redirect to NullStream
        };

        static afl::base::Ref<DirectoryWrapper> create(afl::base::Ref<Directory> parent, afl::io::TextWriter& writer, afl::string::Translator& tx);
        ~DirectoryWrapper();

        virtual afl::base::Ref<afl::io::DirectoryEntry> getDirectoryEntryByName(String_t name);
        virtual afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > getDirectoryEntries();
        virtual afl::base::Ptr<Directory> getParentDirectory();
        virtual String_t getDirectoryName();
        virtual String_t getTitle();

        void setWriteMode(WriteMode mode);
        void setEraseMode(EraseMode mode);

        int getNumRemovedFiles() const;

     private:
        DirectoryWrapper(afl::base::Ref<Directory> parent, afl::io::TextWriter& writer, afl::string::Translator& tx);

        class Enum;
        class Entry;

        afl::base::Ref<Directory> m_parent;
        afl::io::TextWriter& m_writer;
        afl::string::Translator& m_translator;

        EraseMode m_eraseMode;
        WriteMode m_writeMode;
        int m_numRemovedFiles;
    };

} }

#endif
