/**
  *  \file u/t_server_file_root.cpp
  *  \brief Test for server::file::Root
  */

#include "server/file/root.hpp"

#include "t_server_file.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/directoryitem.hpp"
#include "afl/io/internaldirectory.hpp"

/** Simple test. */
void
TestServerFileRoot::testIt()
{
    // Must create a DirectoryItem, which in turn requires a DirectoryHandler.
    class NullDirectoryHandler : public server::file::DirectoryHandler {
     public:
        virtual String_t getName()
            { return String_t(); }
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& /*info*/)
            { throw std::runtime_error("no ref"); }
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t /*name*/)
            { throw std::runtime_error("no ref"); }
        virtual Info createFile(String_t /*name*/, afl::base::ConstBytes_t /*content*/)
            { return Info(); }
        virtual void removeFile(String_t /*name*/)
            { }
        virtual afl::base::Optional<Info> copyFile(DirectoryHandler& /*source*/, const Info& /*sourceInfo*/, String_t /*name*/)
            { return afl::base::Nothing; }
        virtual void readContent(Callback& /*callback*/)
            { }
        virtual DirectoryHandler* getDirectory(const Info& /*info*/)
            { return 0; }
        virtual Info createDirectory(String_t /*name*/)
            { return Info(); }
        virtual void removeDirectory(String_t /*name*/)
            { }
    };
    server::file::DirectoryItem item("(root)", 0, std::auto_ptr<server::file::DirectoryHandler>(new NullDirectoryHandler()));    

    // Test it
    server::file::Root testee(item, afl::io::InternalDirectory::create("(spec)"));

    TS_ASSERT_EQUALS(&testee.rootDirectory(), &item);

    testee.setMaxFileSize(16777216);
    TS_ASSERT_EQUALS(testee.getMaxFileSize(), 16777216U);
}
