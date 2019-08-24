/**
  *  \file server/file/directoryhandler.hpp
  *  \brief Interface server::file::DirectoryHandler
  */
#ifndef C2NG_SERVER_FILE_DIRECTORYHANDLER_HPP
#define C2NG_SERVER_FILE_DIRECTORYHANDLER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/ref.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/string/string.hpp"
#include "afl/base/optional.hpp"
#include "afl/io/stream.hpp"

namespace server { namespace file {

    /** Underlying storage interface.
        This interface implements access to file/directory storage in a copy-in/copy-out fashion.
        It provides the underlying storage for the service logic implemented in DirectoryItem / PathResolver.

        Each DirectoryHandler instance describes one directory.
        New DirectoryHandler instances are created for nested directories; see getDirectory(). */
    class DirectoryHandler : public afl::base::Deletable {
     public:
        /*
         *  Nested Types
         */

        /** Item type. */
        enum Type {
            /** Unknown item.
                A DirectoryHandler can report unknown items to make the user aware of unknown elements present on the storage.
                For example, DirectoryItem will refuse to delete a directory containing unknown items. */
            IsUnknown,

            /** Regular file. */
            IsFile,

            /** Subdirectory. */
            IsDirectory
        };

        /** Information about an item.
            This is our stripped-down equivalent of a "struct stat" or afl::io::DirectoryEntry. */
        struct Info {
            /** Item name. */
            String_t name;
            /** Item type. */
            Type type;
            /** File size.
                Can be missing if the item is not a file, or the file size is not representable. */
            afl::base::Optional<int32_t> size;
            /** Content Id.
                Set if the underlying storage provides a unique Id for this content.
                In this case, two files with identical Id have the same content;
                a changed Id on a file indicates that its content changed. */
            afl::base::Optional<String_t> contentId;

            Info()
                : name(), type(), size(), contentId()
                { }

            Info(const String_t& name, Type type)
                : name(name), type(type), size(), contentId()
                { }
        };

        /** Callback for getDirectoryContent(). */
        class Callback : public afl::base::Deletable {
         public:
            virtual void addItem(const Info& info) = 0;
        };


        /*
         *  Files
         */

        /** Get name of this directory.
            This is used for logging purposes.
            \return name */
        virtual String_t getName() = 0;

        /** Get content of a file in this directory.
            The file is identified by the info object obtained from the getDirectoryContent() callback;
            this may be more efficient than using getFileByName() but requires listing the directory first.
            The file content is provided in the form of a (possibly emulated, read-only) FileMapping
            which allows passing the content around without copying.
            \param info Info of file
            \throw std::runtime_error on errors (e.g. I/O error, bad parameters; conditions and exception type depending on actual derived class) */
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& info) = 0;

        /** Get content of a file in this directory.
            This finds the file even if you don't have an Info structure.
            The file content is provided in the form of a (possibly emulated, read-only) FileMapping
            which allows passing the content around without copying.
            \param name Name of file
            \throw std::runtime_error on errors (e.g. I/O error, bad parameters; conditions and exception type depending on actual derived class) */
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t name) = 0;

        /** Create or update a file.
            If the file already exists, it is created; otherwise it is overwritten.
            It is an error if a directory with the same name already exists.
            \param name Name of file
            \param content New content of file
            \return new file information
            \throw std::runtime_error on errors (e.g. I/O error, bad parameters; conditions and exception type depending on actual derived class) */
        virtual Info createFile(String_t name, afl::base::ConstBytes_t content) = 0;

        /** Remove a file.
            It is an error if this file does not exist.
            \param Name of file to remove.
            \throw std::runtime_error on errors (e.g. I/O error, bad parameters; conditions and exception type depending on actual derived class) */
        virtual void removeFile(String_t name) = 0;

        /** Copy a file into this directory.
            This is an optional function.

            This function should perform the equivalent operation to
            <code>createFile(name, source.getFile(sourceInfo)->get())</code>
            if that can be performed more efficiently.
            If there is no possible optimisation, this function must return Nothing,
            and callers must react on that by doing the above naive implementation.

            It is therefore safe to always return Nothing.

            \param source Source DirectoryHandler
            \param sourceInfo Info of source file relative to \c source
            \param name Name of file to create in this DirectoryHandler

            \retval populated info if the operation can be performed and succeeded (e.g. both DirectoryHandler's are on same server; server-side copy succeeded).

            \retval afl::base::Nothing if the operation can not be performed (e.g. both DirectoryHandler's are on different storages).

            \throw std::runtime_error Operation could be performed, but failed
            (e.g. both DirectoryHandler's are on the same server, but server-side copy failed). */
        virtual afl::base::Optional<Info> copyFile(DirectoryHandler& source, const Info& sourceInfo, String_t name) = 0;


        /*
         *  Directories
         */

        /** Read content of this directory.
            Calls Callback::addItem for each item in this directory, in no particular guaranteed order.
            \param callback Callback
            \throw std::runtime_error on errors (conditions and exception type depending on actual derived class) */
        virtual void readContent(Callback& callback) = 0;

        /** Get handler for a subdirectory.
            \param info Info of directory
            \throw std::runtime_error on errors (conditions and exception type depending on actual derived class) */
        virtual DirectoryHandler* getDirectory(const Info& info) = 0;

        /** Create a subdirectory.
            It is an error if this subdirectory or a file with the same name already exists.
            \param name Name of new subdirectory
            \throw std::runtime_error on errors (conditions and exception type depending on actual derived class) */
        virtual Info createDirectory(String_t name) = 0;

        /** Remove a subdirectory.
            It is an error if such a subdirectory does not exist or it is not empty.
            \param name Name of subdirectory to remove
            \throw std::runtime_error on errors (conditions and exception type depending on actual derived class) */
        virtual void removeDirectory(String_t name) = 0;


        /*
         *  Utilities
         */

        /** Find an item, given its name.
            \param name [in] Name to look for
            \param info [out] Resulting info object
            \retval true Item has been found, \c result has been updated
            \retval false Item not found */
        bool findItem(String_t name, Info& info);
    };

    /** Convert size from original type to API type, if representable.
        \tparam T (implicit) original type
        \param sz original size
        \return sz if representable, otherwise Nothing */
    template<typename T>
    afl::base::Optional<int32_t> convertSize(T sz)
    {
        int32_t converted = int32_t(sz);
        if (converted >= 0 && T(converted) == sz) {
            return converted;
        } else {
            return afl::base::Nothing;
        }
    }

} }

#endif
