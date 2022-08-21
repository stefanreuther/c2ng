/**
  *  \file server/file/readonlydirectoryhandler.hpp
  *  \brief Interface server::file::ReadOnlyDirectoryHandler
  */
#ifndef C2NG_SERVER_FILE_READONLYDIRECTORYHANDLER_HPP
#define C2NG_SERVER_FILE_READONLYDIRECTORYHANDLER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/types.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/string/string.hpp"

namespace server { namespace file {

    /** Underlying storage interface, read-only version.
        This interface implements access to file/directory storage in a copy-in/copy-out fashion.
        It provides the underlying storage for the service logic implemented in DirectoryItem / PathResolver.

        Each ReadOnlyDirectoryHandler instance describes one directory.
        New ReadOnlyDirectoryHandler instances are created for nested directories; see getDirectory().

        This is the read-only interface to allow implementing read-only operations.
        The full, read-write interface is in DirectoryHandler. */
    class ReadOnlyDirectoryHandler : public afl::base::Deletable {
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
        virtual ReadOnlyDirectoryHandler* getDirectory(const Info& info) = 0;


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
