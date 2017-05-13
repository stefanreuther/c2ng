/**
  *  \file server/file/filesystemhandler.hpp
  */
#ifndef C2NG_SERVER_FILE_FILESYSTEMHANDLER_HPP
#define C2NG_SERVER_FILE_FILESYSTEMHANDLER_HPP

#include "server/file/directoryhandler.hpp"
#include "afl/io/filesystem.hpp"

namespace server { namespace file {

    class FileSystemHandler : public DirectoryHandler {
     public:
        FileSystemHandler(afl::io::FileSystem& fs, String_t name);
        ~FileSystemHandler();

        virtual String_t getName();

        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& info);
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t name);
        virtual Info createFile(String_t name, afl::base::ConstBytes_t content);
        virtual void removeFile(String_t name);
        virtual afl::base::Optional<Info> copyFile(DirectoryHandler& source, const Info& sourceInfo, String_t name);

        virtual void readContent(Callback& callback);
        virtual DirectoryHandler* getDirectory(const Info& info);
        virtual Info createDirectory(String_t name);
        virtual void removeDirectory(String_t name);

     private:
        afl::io::FileSystem& m_fileSystem;
        String_t m_name;
    };

} }

#endif
