/**
  *  \file server/file/directoryhandler.hpp
  *  \brief Interface server::file::DirectoryHandler
  */
#ifndef C2NG_SERVER_FILE_DIRECTORYHANDLER_HPP
#define C2NG_SERVER_FILE_DIRECTORYHANDLER_HPP

#include "server/file/readonlydirectoryhandler.hpp"

namespace server { namespace file {

    /** Underlying storage interface, full version.
        This interface implements access to file/directory storage in a copy-in/copy-out fashion.
        It provides the underlying storage for the service logic implemented in DirectoryItem / PathResolver.

        Each DirectoryHandler instance describes one directory.
        New DirectoryHandler instances are created for nested directories; see getDirectory().

        This interface extends ReadOnlyDirectoryHandler to add modifying operations. */
    class DirectoryHandler : public ReadOnlyDirectoryHandler {
     public:
        /*
         *  Files
         */

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
            \param name Name of file to remove.
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
        virtual afl::base::Optional<Info> copyFile(ReadOnlyDirectoryHandler& source, const Info& sourceInfo, String_t name) = 0;


        /*
         *  Directories
         */

        /** Get handler for a subdirectory.
            Extends the ReadOnlyDirectoryHandler version to return a full DirectoryHandler.
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
    };

} }

#endif
