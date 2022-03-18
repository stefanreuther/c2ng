/**
  *  \file server/file/pathresolver.hpp
  *  \brief Class server::file::PathResolver
  */
#ifndef C2NG_SERVER_FILE_PATHRESOLVER_HPP
#define C2NG_SERVER_FILE_PATHRESOLVER_HPP

#include "server/file/directoryitem.hpp"
#include "server/file/item.hpp"

namespace server { namespace file {

    class Root;

    /** Path resolver.
        Resolving a path is a two-step process:
        - first, resolve the directory path if the given file name contains one.
          This step is more or less the same for all operations.
        - then, resolve the final component.
          This step is different between operations.
          Some require the final component to exist, some don't.

        In function calls, first call resolvePath() to move into the final directory.
        When there, you can call resolveLeaf(), hasPermission(), getDirectory(). */
    class PathResolver {
     public:
        /** Constructor.
            \param root Root (effectively used for logging only)
            \param item Root directory / initial current directory
            \param user Current user Id for permission checks */
        PathResolver(Root& root, DirectoryItem& item, String_t user);

        /*
         *  Simple Operations
         */

        /** Resolve a directory path.
            Processes directory components and advances the current directory accordingly.
            Verifies the final component, but does not resolve it.
            \param path [in/out] On input, complete file name. On return, only the final component.
            \throw std::runtime_error 400 (Bad Request) if file name is invalid
            \throw std::runtime_error 403 (Permission Denied) if an intermediate component does not exist but user is not allowed to read that
            \throw std::runtime_error 404 (Not Found) if an intermediate component does not exist and user is allowed to read that */
        void resolvePath(String_t& path);

        /** Resolve the final component.
            \param path [in] File name
            \return Resulting item, null if not found */
        Item* resolveLeaf(String_t path) const;

        /** Check for permission.
            \param priv Permission flag to check
            \return true iff user has this permission in the final directory */
        bool hasPermission(DirectoryItem::Permission priv) const;

        /** Check for permission, throw.
            \param priv Permission flag to check
            \throw std::runtime_error 403 if permission is lacking */
        void checkPermission(DirectoryItem::Permission priv) const;

        /** Get current directory.
            \return directory */
        DirectoryItem& getDirectory() const;


        /*
         *  Complete Operations
         */

        /** Resolve path to a directory and check permissions.

            If this function is called with path="a/b/c", the permission will be checked on "a/b/c" (e.g. permission to manipulate files in this directory).
            After the call, hasPermission() will check the permissions on "a/b" (e.g. permission to manipulate directory c itself).

            \param path complete path
            \param priv Permission to check
            \return resulting directory
            \throw std::runtime_error 400 (Bad Request) if file name is invalid (as for resolvePath)
            \throw std::runtime_error 403 (Permission Denied) as for resolvePath, plus if requested permission is not available
            \throw std::runtime_error 404 (Not Found) as for resolvePath
            \throw std::runtime_error 405 (Not a Directory) if it exists as a file, and user is allowed to read that */
        DirectoryItem& resolveToDirectory(String_t path, DirectoryItem::Permission priv);

        /** Resolve path to a file.

            If this function is called with path="a/b/f.txt", the permission will be checked on "a/b" (file permissions are given per-directory).
            After the call, hasPermission() will check the same permissions.

            \param fileName complete path
            \param priv Permission to check
            \return resulting file
            \throw std::runtime_error 400 (Bad Request) if file name is invalid (as for resolvePath)
            \throw std::runtime_error 403 (Permission Denied) as for resolvePath, plus if requested permission is not available
            \throw std::runtime_error 404 (Not Found) as for resolvePath, plus if item is not a file */
        FileItem& resolveToFile(String_t fileName, DirectoryItem::Permission priv);

        /** Resolve path to an item.

            If this function is called with path="a/b/f", the permission will be checked on "a/b" (file permissions are given per-directory).
            After the call, hasPermission() will check the same permissions.

            \param itemName complete path
            \param priv Permission to check
            \return resulting file
            \throw std::runtime_error 400 (Bad Request) if file name is invalid (as for resolvePath)
            \throw std::runtime_error 403 (Permission Denied) as for resolvePath, plus if requested permission is not available
            \throw std::runtime_error 404 (Not Found) as for resolvePath */
        Item& resolveToItem(String_t itemName, DirectoryItem::Permission priv);

     private:
        Root& m_root;
        DirectoryItem* m_base;
        const String_t m_user;
    };

} }

#endif
