/**
  *  \file u/t_server_file_directoryhandler.cpp
  *  \brief Test for server::file::DirectoryHandler
  */

#include "server/file/directoryhandler.hpp"

#include <stdexcept>
#include "t_server_file.hpp"

/** Interface test. */
void
TestServerFileDirectoryHandler::testInterface()
{
    class Tester : public server::file::DirectoryHandler {
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
    Tester t;
}

/** Test findItem(). */
void
TestServerFileDirectoryHandler::testFind()
{
    class Tester : public server::file::DirectoryHandler {
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
        virtual void readContent(Callback& callback)
            {
                {
                    Info a;
                    a.name = "a";
                    a.size = 99;
                    a.type = IsFile;
                    callback.addItem(a);
                }

                {
                    Info b;
                    b.name = "b";
                    b.size = 100;
                    b.type = IsFile;
                    callback.addItem(b);
                }

                {
                    Info c;
                    c.name = "c";
                    c.type = IsDirectory;
                    callback.addItem(c);
                }
            }
        virtual DirectoryHandler* getDirectory(const Info& /*info*/)
            { return 0; }
        virtual Info createDirectory(String_t /*name*/)
            { return Info(); }
        virtual void removeDirectory(String_t /*name*/)
            { }
    };
    Tester testee;

    server::file::DirectoryHandler::Info out;
    TS_ASSERT(testee.findItem("a", out));
    TS_ASSERT_EQUALS(out.name, "a");
    TS_ASSERT_EQUALS(out.type, server::file::DirectoryHandler::IsFile);
    TS_ASSERT_EQUALS(out.size.orElse(-1), 99);

    TS_ASSERT(testee.findItem("b", out));
    TS_ASSERT_EQUALS(out.name, "b");
    TS_ASSERT_EQUALS(out.type, server::file::DirectoryHandler::IsFile);
    TS_ASSERT_EQUALS(out.size.orElse(-1), 100);

    TS_ASSERT(testee.findItem("c", out));
    TS_ASSERT_EQUALS(out.name, "c");
    TS_ASSERT_EQUALS(out.type, server::file::DirectoryHandler::IsDirectory);
    TS_ASSERT(!out.size.isValid());

    TS_ASSERT(!testee.findItem("cc", out));
    TS_ASSERT(!testee.findItem("a0", out));
    TS_ASSERT(!testee.findItem("a.", out));
    TS_ASSERT(!testee.findItem("c/", out));
}

/** Test convertSize(). */
void
TestServerFileDirectoryHandler::testConvertSize()
{
    using server::file::convertSize;

    // From integer literal
    TS_ASSERT_EQUALS(server::file::convertSize(0).orElse(42), 0);
    TS_ASSERT_EQUALS(server::file::convertSize(99).orElse(42), 99);
    TS_ASSERT_EQUALS(server::file::convertSize(-1).orElse(42), 42);

    // From 64-bit unsigned integer
    TS_ASSERT_EQUALS(server::file::convertSize(static_cast<uint64_t>(0)).orElse(42), 0);
    TS_ASSERT_EQUALS(server::file::convertSize(static_cast<uint64_t>(99)).orElse(42), 99);
    TS_ASSERT_EQUALS(server::file::convertSize(static_cast<uint64_t>(0x200000000ULL)).orElse(42), 42);
    TS_ASSERT_EQUALS(server::file::convertSize(static_cast<uint64_t>(0xFFFFFFFFULL)).orElse(42), 42);
    TS_ASSERT_EQUALS(server::file::convertSize(static_cast<uint64_t>(0x7FFFFFFFULL)).orElse(42), 0x7FFFFFFF);
}

