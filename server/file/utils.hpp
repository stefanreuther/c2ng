/**
  *  \file server/file/utils.hpp
  *  \brief server::file Utilities
  */
#ifndef C2NG_SERVER_FILE_UTILS_HPP
#define C2NG_SERVER_FILE_UTILS_HPP

#include <vector>
#include "server/file/directoryhandler.hpp"

namespace server { namespace file {

    typedef std::vector<DirectoryHandler::Info> InfoVector_t;


    /** List a directory.
        \param out Output
        \param dir Directory to list */
    void listDirectory(InfoVector_t& out, DirectoryHandler& dir);


    /** Copy a directory or directory tree.

        <b>Note:</b> this works on the DirectoryHandler level and therefore bypasses caches on DirectoryItem level.
        If the DirectoryHandler's are derived from DirectoryItem's somehow, call DirectoryItem::forgetContent()
        before calling this.

        \param out Target directory
        \param in Source directory
        \param recursive true for recursive copy

        \throw afl::except::FileProblemException on error */
    void copyDirectory(DirectoryHandler& out, DirectoryHandler& in, bool recursive);


    /** Remove a directory's content.
        Removes everything within the directory, but not the directory itself.
        \param dir Directory
        \throw afl::except::FileProblemException on error */
    void removeDirectoryContent(DirectoryHandler& dir);

    /** Synchronize a directory tree.
        Makes the target directory contain the same content as the source directory.
        Unlike copyDirectory() which only copies files, this also removes excess files and resolves type conflicts
        (target has a file where source has a directory, and vice versa).

        Synchronisation is always recursive.

        <b>Note:</b> this works on the DirectoryHandler level and therefore bypasses caches on DirectoryItem level.
        If the DirectoryHandler's are derived from DirectoryItem's somehow, call DirectoryItem::forgetContent()
        before calling this.

        \param out Target directory
        \param in Source directory

        \throw afl::except::FileProblemException on error */
    void synchronizeDirectories(DirectoryHandler& out, DirectoryHandler& in);
} }

#endif
