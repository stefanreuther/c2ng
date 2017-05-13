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

        virtual void copyFile(String_t sourceFile, String_t destFile) = 0;
        virtual void forgetDirectory(String_t dirName) = 0;
        virtual void testFiles(afl::base::Memory<const String_t> fileNames, afl::data::IntegerList_t& resultFlags) = 0;
        virtual String_t getFile(String_t fileName) = 0;
        virtual void getDirectoryContent(String_t dirName, ContentInfoMap_t& result) = 0;
        virtual void getDirectoryPermission(String_t dirName, String_t& ownerUserId, std::vector<Permission>& result) = 0;
        virtual void createDirectory(String_t dirName) = 0;
        virtual void createDirectoryTree(String_t dirName) = 0;
        virtual void createDirectoryAsUser(String_t dirName, String_t userId) = 0;
        virtual afl::data::Value* getDirectoryProperty(String_t dirName, String_t propName) = 0;
        virtual void setDirectoryProperty(String_t dirName, String_t propName, String_t propValue) = 0;
        virtual void putFile(String_t fileName, String_t content) = 0;
        virtual void removeFile(String_t fileName) = 0;
        virtual void removeDirectory(String_t dirName) = 0;
        virtual void setDirectoryPermissions(String_t dirName, String_t userId, String_t permission) = 0;
        virtual Info getFileInformation(String_t fileName) = 0;
        virtual Usage getDiskUsage(String_t dirName) = 0;

        int32_t getDirectoryIntegerProperty(String_t dirName, String_t propName);
        String_t getDirectoryStringProperty(String_t dirName, String_t propName);
    };

} }

#endif
