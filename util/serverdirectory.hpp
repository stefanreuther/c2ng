/**
  *  \file util/serverdirectory.hpp
  *  \brief Class util::ServerDirectory
  */
#ifndef C2NG_UTIL_SERVERDIRECTORY_HPP
#define C2NG_UTIL_SERVERDIRECTORY_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"

namespace util {

    /** Implementation of Directory for data stored in a copy-in, copy-out storage (server).

        Translates the Directory interface into a simpler one (Transfer).
        Files are downloaded on demand.
        Use flush() to write modifications back (=upload changes, perform deletions).
        There is no implicit flush() in the destructor.

        This implementation supports read and write access as well as file deletion,
        but not renaming.

        This class does not support subdirectories,
        but tries to prevent that a subdirectory is being overwritten by being aware of non-files.

        This class is not thread-safe (like most I/O classes). */
    class ServerDirectory : public afl::io::Directory {
     public:
        /** Information about a file. */
        struct FileInfo {
            /** Name (basename). */
            String_t name;

            /** Size in bytes. */
            afl::io::Stream::FileSize_t size;

            /** File status.
                true: this is a file.
                false: this is something else (e.g. a directory, device, etc.). */
            bool isFile;

            /** Constructor.
                @param name    Name
                @param size    File size in bytes
                @param isFile  true if this is a file */
            FileInfo(const String_t& name, afl::io::Stream::FileSize_t size, bool isFile)
                : name(name), size(size), isFile(isFile)
                { }
        };

        /** Transport implementation.

            All methods are supposed to talk to the underlying storage;
            no caching needed.

            A Transport can be (partially) read-only.
            If isWritable() consistently returns false,
            putFile()/eraseFile() will never be called and can be implemented empty.
            If isWritable() returns false() only sometimes,
            those methods will be called and need to deal possible read-only status of the underlying storage. */
        class Transport : public afl::base::Deletable, public afl::base::RefCounted {
         public:
            /** Get file content.
                @param [in]  name   File name
                @param [out] data   File content appended here
                @throw FileProblemException on error */
            virtual void getFile(String_t name, afl::base::GrowableBytes_t& data) = 0;

            /** Store file content.
                @param [in]  name   File name
                @param [in]  data   File content
                @throw FileProblemException on error */
            virtual void putFile(String_t name, afl::base::ConstBytes_t data) = 0;

            /** Erase a file.
                @param [in]  name   File name
                @throw FileProblemException on error */
            virtual void eraseFile(String_t name) = 0;

            /** Get content of directory.
                @param [out] result Result appended here
                @throw FileProblemException on error */
            virtual void getContent(std::vector<FileInfo>& result) = 0;

            /** Check validity of a file name.
                Used to pre-validate creation of new files.
                @param [in]  name   File name
                @return true if valid, false if invalid. */
            virtual bool isValidFileName(String_t name) const = 0;

            /** Check permission to write.
                This is used to reject write requests early.
                @return true if writable, false if not. */
            virtual bool isWritable() const = 0;
        };

     public:
        /** Destructor. */
        ~ServerDirectory();

        /** Constructor.
            @param transport        Transport implementation
            @param title            Title to report for getTitle()
            @param parentDirectory  Parent directory to report for getParentDirectory(), can be null */
        static afl::base::Ref<ServerDirectory> create(afl::base::Ref<Transport> transport, String_t title, afl::base::Ptr<Directory> parentDirectory);

        /** Flush.
            Writes all changes to the underlying transport and discards stored data.
            If an operation throws an exception, tries to complete remaining operations and re-throws the exception.

            When trying to upload a, b, c, and b fails, c will still be uploaded and b's exception be thrown.
            When b and c fail, you'll also receive b's exception. */
        void flush();

        // Directory
        virtual afl::base::Ref<afl::io::DirectoryEntry> getDirectoryEntryByName(String_t name);
        virtual afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > getDirectoryEntries();
        virtual afl::base::Ptr<Directory> getParentDirectory();
        virtual String_t getDirectoryName();
        virtual String_t getTitle();

     private:
        // Private constructor to ensure heap allocation
        ServerDirectory(const afl::base::Ref<Transport>& transport, const String_t& title, const afl::base::Ptr<Directory>& parentDirectory);

        // Embedded classes
        class Entry;
        class Enum;
        struct File;

        // Attributes
        const afl::base::Ref<Transport> m_transport;
        const String_t m_title;
        const afl::base::Ptr<Directory> m_parentDirectory;
        afl::container::PtrVector<File> m_files;
        bool m_filesLoaded;

        // Private functions
        void checkWritable(const String_t& name);
        void loadContent();
        std::pair<File*, size_t> findEntry(const String_t& name, size_t hint);
        std::pair<File*, size_t> createEntry(const String_t& name);
        bool flushEntry(size_t& index);
    };

}

#endif
