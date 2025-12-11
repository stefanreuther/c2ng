/**
  *  \file server/file/internaldirectoryhandler.hpp
  *  \brief Class server::file::InternalDirectoryHandler
  */
#ifndef C2NG_SERVER_FILE_INTERNALDIRECTORYHANDLER_HPP
#define C2NG_SERVER_FILE_INTERNALDIRECTORYHANDLER_HPP

#include "afl/base/growablememory.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "server/file/directoryhandler.hpp"

namespace server { namespace file {

    /** In-Memory implementation of DirectoryHandler.
        This class is used for testing.
        The typical use-case:
        - creates a InternalDirectoryHandler::Directory
        - creates an InternalDirectoryHandler using that Directory
        - creates a Root that uses that InternalDirectoryHandler

        Since this class is used for testing, it allows manipulation of its inner data structures. */
    class InternalDirectoryHandler : public DirectoryHandler {
     public:
        /** Representation of a file.
            It can contain data. */
        struct File {
            String_t name;
            afl::base::GrowableMemory<uint8_t> content;
            explicit File(const String_t& name)
                : name(name), content()
                { }
        };

        /** Representation of a directory.
            It can contain more directories and files. */
        struct Directory {
            String_t name;
            afl::container::PtrVector<Directory> subdirectories;
            afl::container::PtrVector<File> files;
            explicit Directory(const String_t& name)
                : name(name), subdirectories(), files()
                { }
        };

        /** Constructor.
            \param name Name
            \param dir Directory */
        InternalDirectoryHandler(String_t name, Directory& dir);

        // DirectoryHandler methods:
        virtual String_t getName();
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& info);
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t name);
        virtual Info createFile(String_t name, afl::base::ConstBytes_t content);
        virtual void removeFile(String_t name);
        virtual void readContent(Callback& callback);
        virtual DirectoryHandler* getDirectory(const Info& info);
        virtual Info createDirectory(String_t name);
        virtual void removeDirectory(String_t name);
        virtual afl::base::Optional<Info> copyFile(ReadOnlyDirectoryHandler& source, const Info& sourceInfo, String_t name);
        virtual SnapshotHandler* getSnapshotHandler();

        /** Find file, given a name.
            \param name Name to find
            \return File if found, null otherwise */
        File* findFile(const String_t& name);

        /** Find directory, given a name.
            \param name Name to find
            \return Directory if found, null otherwise */
        Directory* findDirectory(const String_t& name);

     private:
        const String_t m_name;
        Directory& m_dir;

        String_t makeName(const String_t& childName) const;
    };

} }

#endif
