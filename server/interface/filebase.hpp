/**
  *  \file server/interface/filebase.hpp
  *  \brief Interface server::interface::FileBase
  */
#ifndef C2NG_SERVER_INTERFACE_FILEBASE_HPP
#define C2NG_SERVER_INTERFACE_FILEBASE_HPP

#include "afl/base/deletable.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/string/string.hpp"
#include "afl/base/memory.hpp"
#include "afl/container/ptrmap.hpp"
#include "afl/data/value.hpp"
#include "afl/base/optional.hpp"

namespace server { namespace interface {

    /** File Server Base Interface.
        This interface allows access to files, including administrative and user configuration operations. */
    class FileBase : public afl::base::Deletable {
     public:
        /** File type.
            Expect this enum to grow over time. */
        enum Type {
            IsFile,             ///< Regular file.
            IsDirectory,        ///< Directory.
            IsUnknown           ///< Anything else.
        };

        /** Information about a file.
            Expect this structure to grow over time.
            All elements other than type are optional. */
        struct Info {
            /** Type. */
            Type type;

            /** Visibility. Set for directories.
                - 0: normal
                - 1: directory has some permissions for another user
                - 2: directory has permissions for everyone */
            afl::base::Optional<int32_t> visibility;

            /** Size in bytes. Set for files. */
            afl::base::Optional<int32_t> size;

            /** Content Id. */
            afl::base::Optional<String_t> contentId;

            Info()
                : type(IsUnknown), visibility(), size(), contentId()
                { }
        };

        /** Permission entry.
            Maps a user Id to a permission string. */
        struct Permission {
            /** User Id. */
            String_t userId;

            /** Permission string.
                Contains a list of enabled permissions.
                - r: files in this directory can be read
                - w: files in this directory can be written
                - l: directory content can be listed
                - a: permissions in this directory can be changed */
            String_t permission;

            Permission()
                : userId(), permission()
                { }
            Permission(String_t userId, String_t permission)
                : userId(userId), permission(permission)
                { }
        };

        /** Disk usage summary. */
        struct Usage {
            int32_t numItems;      ///< Number of items (files, directories).
            int32_t totalKBytes;   ///< Disk usage in kilobytes.

            Usage()
                : numItems(0), totalKBytes(0)
                { }
        };

        typedef afl::container::PtrMap<String_t, Info> ContentInfoMap_t;

        /** Copy a file.
            \param sourceFile Source file name. File must exist and be accessible.
            \param destFile Destination file name. File may or may not exist, but refer to a writable directory. */
        virtual void copyFile(String_t sourceFile, String_t destFile) = 0;

        /** Forget a directory.
            Uses include:
            - forget cached data to free memory
            - force synchronisation with possible external modifications to the underlying dataspace
            \param dirName Directory name. Passing a nonexistant or inaccessible is not an error. */
        virtual void forgetDirectory(String_t dirName) = 0;

        /** Test accessibility of files.
            \param fileNames [in] List of file names.
            \param resultFlags [out] For each file, a flag. 1=file exists and is readable, 0=file not accessible.
                   Inaccessible or invalid file names are reported using this parameter and do not produce an error. */
        virtual void testFiles(afl::base::Memory<const String_t> fileNames, afl::data::IntegerList_t& resultFlags) = 0;

        /** Get file content.
            \param fileName File name. File must exist and be readable.
            \return file content */
        virtual String_t getFile(String_t fileName) = 0;

        /** Get directory content.
            \param dirName [in] Directory name. Directory must exist and be listable.
            \param result [out] Directory content */
        virtual void getDirectoryContent(String_t dirName, ContentInfoMap_t& result) = 0;

        /** Get directory permissions.
            \param dirName [in] Directory name. Directory must exist and access control allowed.
            \param ownerUserId [out] Owner
            \param result [out] Access control list */
        virtual void getDirectoryPermission(String_t dirName, String_t& ownerUserId, std::vector<Permission>& result) = 0;

        /** Create a directory.
            \param dirName New directory name. Name must not refer to an existing directory, but its parent must exist and be writable. */
        virtual void createDirectory(String_t dirName) = 0;

        /** Create directory tree.
            This command is used to make sure that a directory path exists.
            Missing components are created; it is not an error if (parts of) the directory already exist.
            \param dirName New directory name */
        virtual void createDirectoryTree(String_t dirName) = 0;

        /** Create a directory as user.
            This is the only way to create objects owned by a user.
            This command is restricted to admin usage.
            \param dirName New directory name. Name must not refer to an existing directory, but its parent must exist and be writable.
            \param userId New directory owner */
        virtual void createDirectoryAsUser(String_t dirName, String_t userId) = 0;

        /** Get directory property.
            \param dirName Directory name. Must exist and be readable.
            \param propName Name of property to read
            \return newly-allocated property value (could be any type; typically a string) */
        virtual afl::data::Value* getDirectoryProperty(String_t dirName, String_t propName) = 0;

        /** Set directory property.
            \param dirName Directory name. Must exist and be writable.
            \param propName Name of property. Restrictions may apply (c2file: no CR/LF/equals).
            \param propValue Value. Restrictions may apply (c2file: no CR/LF). */
        virtual void setDirectoryProperty(String_t dirName, String_t propName, String_t propValue) = 0;

        /** Create a file.
            \param fileName File name. The directory must exist and be writable. If the file exists, it is overwritten.
            \param content File content */
        virtual void putFile(String_t fileName, String_t content) = 0;

        /** Remove a file or empty directory.
            \param fileName File name. Must refer to a file or an empty directory, within a writable directory. */
        virtual void removeFile(String_t fileName) = 0;

        /** Remove a directory tree.
            Removes all files and directories beneath the given directory.
            \param dirName Directory name. Must be within a writable directory; all content must be writable. */
        virtual void removeDirectory(String_t dirName) = 0;

        /** Set directory permissions.
            \param dirName Directory name. Must exist and access be control allowed.
            \param userId User to set permissions for
            \param permission Permission string */
        virtual void setDirectoryPermissions(String_t dirName, String_t userId, String_t permission) = 0;

        /** Get file information.
            \param fileName File name. Must exist, directory must be listable.
            \return file info */
        virtual Info getFileInformation(String_t fileName) = 0;

        /** Get disk usage.
            Returns the total number of files and space used.
            \param dirName Directory name. Must be listable.
            \return disk usage */
        virtual Usage getDiskUsage(String_t dirName) = 0;

        /** Get directory property, integer result.
            \param dirName Directory name. Must exist and be readable.
            \param propName Name of property to read
            \return property value as integer
            \see getDirectoryProperty */
        int32_t getDirectoryIntegerProperty(String_t dirName, String_t propName);

        /** Get directory property, string result.
            \param dirName Directory name. Must exist and be readable.
            \param propName Name of property to read
            \return property value as string
            \see getDirectoryProperty */
        String_t getDirectoryStringProperty(String_t dirName, String_t propName);

        /** Get file content if the file exists.
            Unlike getFile(), this will not throw on errors, but instead return Nothing.
            This is the equivalent to afl::io::Directory::openFileNT
            \param fileName File name. File must exist and be readable.
            \return file content; nothing if reading the file fails */
        afl::base::Optional<String_t> getFileNT(String_t fileName);
    };

} }

#endif
