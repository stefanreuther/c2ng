/**
  *  \file server/console/fileexporter.hpp
  *  \brief Interface server::console::FileExporter
  */
#ifndef C2NG_SERVER_CONSOLE_FILEEXPORTER_HPP
#define C2NG_SERVER_CONSOLE_FILEEXPORTER_HPP

#include "afl/base/deletable.hpp"
#include "afl/io/filesystem.hpp"

namespace server { namespace console {

    /** File export policy.
        The interface specifies a policy for exporting files (exportFile()).
        We also provide a set of implementations. */
    class FileExporter : public afl::base::Deletable {
     public:
        /** Export mode. */
        enum Mode {
            NoFiles,             ///< Do not export any files.
            InlineFiles,         ///< Export files as inline strings.
            NormalFiles,         ///< Export files as normal files in a file hierarchy.
            DedupFiles           ///< Deduplicate files while exporting.
        };

        /** Export a file.
            @param fileName    File name (parameter to FileBase::getFile())
            @param fileContent File content (result of FileBase::getFile())
            @return script content to reproduce the file content */
        virtual String_t exportFile(const String_t& fileName, const String_t& fileContent) = 0;

        /** Create a FileExporter.
            @param mode         Mode
            @param fs           File system instance
            @param serviceName  Service name (to create files)
            @return newly-created FileExporter instance, caller takes responsibility. Can be null. */
        static FileExporter* create(Mode mode, afl::io::FileSystem& fs, String_t serviceName);

     private:
        class InlineExporter;
        class NormalExporter;
        class DedupExporter;
    };

} }

#endif
