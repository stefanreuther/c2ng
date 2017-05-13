/**
  *  \file server/file/internaldirectoryhandler.hpp
  */
#ifndef C2NG_SERVER_FILE_INTERNALDIRECTORYHANDLER_HPP
#define C2NG_SERVER_FILE_INTERNALDIRECTORYHANDLER_HPP

#include "afl/string/string.hpp"
#include "server/file/directoryhandler.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/base/growablememory.hpp"

namespace server { namespace file {

    class InternalDirectoryHandler : public DirectoryHandler {
     public:
        struct File {
            String_t name;
            afl::base::GrowableMemory<uint8_t> content;
            explicit File(const String_t& name)
                : name(name), content()
                { }
        };
        struct Directory {
            String_t name;
            afl::container::PtrVector<Directory> subdirectories;
            afl::container::PtrVector<File> files;
            explicit Directory(const String_t& name)
                : name(name), subdirectories(), files()
                { }
        };

        InternalDirectoryHandler(String_t name, Directory& dir);

        virtual String_t getName();
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& info);
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t name);
        virtual Info createFile(String_t name, afl::base::ConstBytes_t content);
        virtual void removeFile(String_t name);
        virtual void readContent(Callback& callback);
        virtual DirectoryHandler* getDirectory(const Info& info);
        virtual Info createDirectory(String_t name);
        virtual void removeDirectory(String_t name);
        virtual afl::base::Optional<Info> copyFile(DirectoryHandler& source, const Info& sourceInfo, String_t name);

        File* findFile(const String_t& name);
        Directory* findDirectory(const String_t& name);

     private:
        String_t m_name;
        Directory& m_dir;

        String_t makeName(const String_t& childName) const;
    };

} }

#endif
