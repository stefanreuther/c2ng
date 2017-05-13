/**
  *  \file server/file/directorywrapper.hpp
  *  \brief Class server::file::DirectoryWrapper
  */
#ifndef C2NG_SERVER_FILE_DIRECTORYWRAPPER_HPP
#define C2NG_SERVER_FILE_DIRECTORYWRAPPER_HPP

#include "afl/io/directory.hpp"

namespace server { namespace file {

    class DirectoryItem;

    /** Wrap a DirectoryItem into a afl::io::Directory (read-only).
        Use this to call code that needs a Directory when you have a DirectoryItem.
        This implements read-only access, and does not attempt to meaningfully handle parallel modifications to the file space.
        It accesses the managed file space (hence, requires a DirectoryItem),
        but assumes reading and access checking to have been performed before.

        DirectoryWrapper only allows access to files in the directory, not to subdirectories
        (which might have different access permissions). */
    class DirectoryWrapper : public afl::io::Directory {
     public:
        static afl::base::Ref<DirectoryWrapper> create(DirectoryItem& item);

        virtual ~DirectoryWrapper();
        virtual afl::base::Ref<afl::io::DirectoryEntry> getDirectoryEntryByName(String_t name);
        virtual afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > getDirectoryEntries();
        virtual afl::base::Ptr<Directory> getParentDirectory();
        virtual String_t getDirectoryName();
        virtual String_t getTitle();

     private:
        /** Constructor.
            \param item Directory to access.
            \pre item.wasRead() */
        explicit DirectoryWrapper(DirectoryItem& item);

        class File;
        class Entry;
        class Enum;

        DirectoryItem& m_item;
    };

} }

#endif
