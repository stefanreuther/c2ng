/**
  *  \file server/file/directoryitem.hpp
  *  \brief Class server::file::DirectoryItem
  */
#ifndef C2NG_SERVER_FILE_DIRECTORYITEM_HPP
#define C2NG_SERVER_FILE_DIRECTORYITEM_HPP

#include <memory>
#include <map>
#include "afl/bits/smallset.hpp"
#include "afl/container/ptrvector.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/fileitem.hpp"
#include "server/file/item.hpp"
#include "server/interface/filebase.hpp"

namespace server { namespace file {

    class Root;
    class GameStatus;

    /** In-Memory Representation of a Directory.
        Provides higher-level operations and a link to a DirectoryHandler.

        A DirectoryItem caches metadata content of the directory.
        This content is read on demand only (readContent(), readGameStatus()).
        Functions that want to access the directory's content must invoke these functions.

        DirectoryItem implements the mapping of user-perceived file storage and attribute model
        to the underlying storage model, i.e. files in one operating-system user account and
        ".c2file" metadata files.

        DirectoryItem implements the raw access primitives.
        It does not perform permission checks; these are up to FileBase.
        DirectoryItem will only perform essential consistency checks,
        i.e. it will refuse operations it cannot do or that would produce an invalid state.
        Those are documented as exceptions ("Throws:").

        In addition, functions accessing the underlying storage may throw exceptions.
        Those exceptions will not follow the conventions outlined in server/errors.hpp.
        A front-end would treat them as a "500 Server Error".
        Those errors always indicate malfunction on the server (e.g. I/O error)
        or a misunderstanding between c2ng and the operating system (e.g. file deleted from underlying storage). */
    class DirectoryItem : public Item {
     public:
        /** Access permission.
            Permissions are given per directory. */
        enum Permission {
            AllowRead,                  ///< Users can read files in this directory if they know the name.
            AllowWrite,                 ///< Users can write files in this directory.
            AllowList,                  ///< Users can list the files in this directory.
            AllowAccess                 ///< Users can assign permissions for this directory.
        };

        /** Set of permissions. */
        typedef afl::bits::SmallSet<Permission> Permissions_t;

        /** Constructor.
            \param name Name (single path name component)
            \param parent Parent directory. Null for the root.
            \param handler Directory handler. Must not be null. */
        DirectoryItem(String_t name, DirectoryItem* parent, std::auto_ptr<DirectoryHandler> handler);

        /** Destructor. */
        ~DirectoryItem();

        /*
         *  Content Management
         */

        /** Read content.
            This reads the directory metadata unless it has already been read.
            Errors will only be logged, with the directory appearing empty afterwards.
            \param root Service root (essentially, for logging, and to make explicit we're excessing the outside world here) */
        void readContent(Root& root);

        /** Check whether this directory was read.
            \return true if readContent() has been called already */
        bool wasRead() const;

        /** Forget this directory and all its subdirectories.
            Returns this object to unread status.
            Use this to reclaim memory (RAM), and to force resynchronisation if the underlying storage changed.
            \param root Service root (essentially, for logging) */
        void forgetContent(Root& root);

        /** Read game status.
            Reads game-specific metadata (FileGame content) unless it has already been read.
            \param root Service root (provides logging, config)
            \return status */
        GameStatus& readGameStatus(Root& root);

        /*
         *  Children access
         */

        /** Find directory by name.
            \param name Directory name
            \return DirectoryItem if found, null otherwise
            \pre wasRead() */
        DirectoryItem* findDirectory(String_t name) const;

        /** Find file by name.
            \param name File name
            \return FileItem if found, null otherwise
            \pre wasRead() */
        FileItem* findFile(String_t name) const;

        /** Get number of subdirectories.
            \return Number of subdirectories
            \pre wasRead() */
        size_t getNumDirectories() const;

        /** Get subdirectory by index.
            \param n Index [0, getNumDirectories())
            \return DirectoryItem if n in range, null otherwise
            \pre wasRead() */
        DirectoryItem* getDirectoryByIndex(size_t n) const;

        /** Get number of files.
            \return Number of files
            \pre wasRead() */
        size_t getNumFiles() const;

        /** Get file by index.
            \param n Index [0, getNumFiles())
            \return FileItem if n in range, null otherwise
            \pre wasRead() */
        FileItem* getFileByIndex(size_t n) const;

        /** Get content of a file.
            \param fileItem FileItem for the file
            \return File mapping containing the file. See DirectoryHandler::getFile(). */
        afl::base::Ref<afl::io::FileMapping> getFileContent(FileItem& fileItem);

        /** Create or overwrite a file.
            \param fileName Name of file
            \param content Content of file
            \pre wasRead()
            \throw std::runtime_error ALREADY_EXISTS if a conflicting item (directory) exists */
        void createFile(const String_t& fileName, afl::base::ConstBytes_t content);

        /** Copy file.
            \param sourceDirectory Source directory
            \param sourceFile Source file (a child of sourceDirectory)
            \param fileName Desired name
            \pre wasRead()
            \throw std::runtime_error ALREADY_EXISTS if a conflicting item (directory) exists */
        bool copyFile(DirectoryItem& sourceDirectory, FileItem& sourceFile, const String_t& fileName);

        /** Create subdirectory.
            \param dirName Name of subdirectory
            \throw std::runtime_error ALREADY_EXISTS if item already exists */
        DirectoryItem* createDirectory(const String_t& dirName);

        /*
         *  Removal of Items
         */

        /** Remove an item.
            The item must be a file or a user-perceived empty subdirectory within this directory.
            \param root Service root (provides logging, config)
            \param it Item to remove
            \throw std::runtime_error PERMISSION_DENIED if item cannot be removed (e.g. nonempty directory) */
        void removeItem(Root& root, Item* it);

        /** Remove user content.
            Removes all files in this folder, as-if by calling removeItem(), but more efficient.
            \throw std::runtime_error PERMISSION_DENIED if an item cannot be removed (e.g. nonempty directory) */
        void removeUserContent(Root& root);

        /*
         *  Properties
         */

        /** Get directory property.
            This takes full property names, and can thus be used to modify all properties of a directory.
            Properties modified with PROPSET have a "prop:" prefix.
            \param p Name of property
            \pre wasRead() */
        String_t getProperty(String_t p) const;

        /** Modify/set directory property.
            This takes full property names, and can thus be used to modify all properties of a directory.
            Properties modified with PROPSET have a "prop:" prefix.

            Note that the name must not contain "=", "\r" or "\n"; the value must not contain "\r" or "\n".

            \param p Name of property
            \param v Value of property
            \pre wasRead() */
        void setProperty(String_t p, String_t v);

        /** Get owner of this directory.
            The owner is derived indirectly from the "owner" property.
            \return user Id */
        String_t getOwner() const;

        /** Check for user permission.
            \param user User Id to check
            \param p Permission to check
            \return true if the given user has permission p
            \pre wasRead() */
        bool hasPermission(String_t user, Permission p) const;

        /** List all permissions.
            \param result [out] List of permissions
            \pre wasRead() */
        void listPermissions(std::vector<server::interface::FileBase::Permission>& result) const;

        /** Set user permission.
            \param userId User Id
            \param permission New permission string
            \pre wasRead() */
        void setPermission(String_t userId, String_t permission);

        /** Get visibility level for a directory.
            \retval 0 only owner can access it
            \retval 1 some permissions have been granted
            \retval 2 world-permissions have been granted
            \pre wasRead() */
        int getVisibilityLevel() const;

        /*
         *  Misc
         */

        /** Compute disk usage totals.
            The used resources are added to the parameters, recursively.
            \param [in]  root        Server root
            \param [out] numFiles    Number of files and directories
            \param [out] totalKBytes Disk usage of files, rounding up to full kilobytes for each file */
        void computeTotals(Root& root, int32_t& numFiles, int32_t& totalKBytes);

     private:
        typedef std::map<String_t, String_t> ControlInfo_t;
        DirectoryItem* m_parent;
        std::auto_ptr<DirectoryHandler> m_handler;

        // Content
        std::auto_ptr<FileItem> m_controlFile;
        afl::container::PtrVector<DirectoryItem> m_subdirectories;
        afl::container::PtrVector<FileItem> m_files;
        bool m_hasUnknownContent;

        // Status
        bool m_wasRead;

        String_t m_owner;
        ControlInfo_t m_controlInfo;

        std::auto_ptr<GameStatus> m_gameStatus;   // ex dir_info

        // Internal functions
        void loadControlFile();
        void saveControlFile();
        void updateOwner();

        void removeSystemContent(Root& root);

        // Utilities
        static Permissions_t getPermissionsFromString(const String_t& str);
        static String_t getStringFromPermissions(Permissions_t p);
    };

} }

#endif
