/**
  *  \file server/host/spec/directory.hpp
  *  \brief Class server::host::spec::Directory
  */
#ifndef C2NG_SERVER_HOST_SPEC_DIRECTORY_HPP
#define C2NG_SERVER_HOST_SPEC_DIRECTORY_HPP

#include "afl/io/directory.hpp"
#include "server/interface/filebase.hpp"

namespace server { namespace host { namespace spec {

    /** Implementation of Directory for host specification publisher.
        This is a limited implementation to avoid that we do unexpected things:

        (a) Directory is scanned ahead, and only files in the directory listing are published.
        This is a speed optimisation, and eventually allows us to retrieve contentId's ahead of time.
        However, it also means that this class is intended to be short-lived.

        (b) It does not support writing, or changing into subdirectories or parent directory,
        not even as an option, for safety.

        (c) It can be disabled, at which time it will fail all further file accesses.
        Loading a shiplist is instant and will access all files during load.
        However, in case someone keeps a Directory object around for a longer time,
        this guarantees that it cannot be used to interfere with further use of the filer connection.
        (We do not configure any access permissions, so a later access would use later access permissions.)

        (d) If a file to be opened does not exist, but a file with the same name, ending in ".frag", does,
        the latter is opened instead.

        Thus, this class is similar to server::play::fs::Directory, but not identical. */
    class Directory : public afl::io::Directory {
     public:
        class Entry;
        class Enum;
        typedef server::interface::FileBase::ContentInfoMap_t ContentInfoMap_t;

        /** Create directory.
            @param filer   File access
            @param dirName Directory name */
        static afl::base::Ref<Directory> create(server::interface::FileBase& filer, String_t dirName);

        /** Set file access permission.
            If set to true (default), files can be accessed.
            If set to false, all file accesses will fail (in particular, the FileBase instance passed to the constructor will not be accessed).
            @param flag File access permission */
        void setEnabled(bool flag);

        // Directory virtuals:
        virtual afl::base::Ref<afl::io::DirectoryEntry> getDirectoryEntryByName(String_t name);
        virtual afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > getDirectoryEntries();
        virtual afl::base::Ptr<afl::io::Directory> getParentDirectory();
        virtual String_t getDirectoryName();
        virtual String_t getTitle();
        virtual void flush();

     private:
        Directory(server::interface::FileBase& filer, String_t dirName);

        /** Filer. Only to be accessed if m_enabled=true. */
        server::interface::FileBase& m_filer;

        /** Directory name. */
        const String_t m_dirName;

        /** File access permission. */
        bool m_enabled;

        /** Cached directory content. */
        server::interface::FileBase::ContentInfoMap_t m_content;

        String_t makePathName(const String_t& fileName) const;
        ContentInfoMap_t::const_iterator find(const String_t& fileName) const;
    };

} } }

#endif
