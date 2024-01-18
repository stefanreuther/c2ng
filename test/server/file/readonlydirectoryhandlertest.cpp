/**
  *  \file test/server/file/readonlydirectoryhandlertest.cpp
  *  \brief Test for server::file::ReadOnlyDirectoryHandler
  */

#include "server/file/readonlydirectoryhandler.hpp"

#include <stdexcept>
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.file.ReadOnlyDirectoryHandler:interface")
{
    class Tester : public server::file::ReadOnlyDirectoryHandler {
     public:
        virtual void addItem(const Info& /*info*/)
            { }
        virtual String_t getName()
            { return String_t(); }
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& /*info*/)
            { throw std::runtime_error("no ref"); }
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t /*name*/)
            { throw std::runtime_error("no ref"); }
        virtual void readContent(Callback& /*callback*/)
            { }
        virtual server::file::ReadOnlyDirectoryHandler* getDirectory(const Info& /*info*/)
            { return 0; }
    };
    Tester t;
}

/** Test findItem(). */
AFL_TEST("server.file.ReadOnlyDirectoryHandler:findItem", a)
{
    class Tester : public server::file::ReadOnlyDirectoryHandler {
     public:
        virtual String_t getName()
            { return String_t(); }
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& /*info*/)
            { throw std::runtime_error("no ref"); }
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t /*name*/)
            { throw std::runtime_error("no ref"); }
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
        virtual server::file::ReadOnlyDirectoryHandler* getDirectory(const Info& /*info*/)
            { return 0; }
    };
    Tester testee;

    server::file::ReadOnlyDirectoryHandler::Info out;
    a.check("01. findItem a", testee.findItem("a", out));
    a.checkEqual("02. name", out.name, "a");
    a.checkEqual("03. type", out.type, server::file::ReadOnlyDirectoryHandler::IsFile);
    a.checkEqual("04. size", out.size.orElse(-1), 99);

    a.check("11. findItem b", testee.findItem("b", out));
    a.checkEqual("12. name", out.name, "b");
    a.checkEqual("13. type", out.type, server::file::ReadOnlyDirectoryHandler::IsFile);
    a.checkEqual("14. size", out.size.orElse(-1), 100);

    a.check("21. findItem c", testee.findItem("c", out));
    a.checkEqual("22. name", out.name, "c");
    a.checkEqual("23. type", out.type, server::file::ReadOnlyDirectoryHandler::IsDirectory);
    a.check("24. size", !out.size.isValid());

    a.check("31. findItem cc", !testee.findItem("cc", out));
    a.check("32. findItem c0", !testee.findItem("a0", out));
    a.check("33. findItem c.", !testee.findItem("a.", out));
    a.check("34. findItem c/", !testee.findItem("c/", out));
}

/** Test convertSize(). */
AFL_TEST("server.file.ReadOnlyDirectoryHandler:convertSize", a)
{
    using server::file::convertSize;

    // From integer literal
    a.checkEqual("01", convertSize(0).orElse(42), 0);
    a.checkEqual("02", convertSize(99).orElse(42), 99);
    a.checkEqual("03", convertSize(-1).orElse(42), 42);

    // From 64-bit unsigned integer
    a.checkEqual("11", convertSize(static_cast<uint64_t>(0)).orElse(42), 0);
    a.checkEqual("12", convertSize(static_cast<uint64_t>(99)).orElse(42), 99);
    a.checkEqual("13", convertSize(static_cast<uint64_t>(0x200000000ULL)).orElse(42), 42);
    a.checkEqual("14", convertSize(static_cast<uint64_t>(0xFFFFFFFFULL)).orElse(42), 42);
    a.checkEqual("15", convertSize(static_cast<uint64_t>(0x7FFFFFFFULL)).orElse(42), 0x7FFFFFFF);
}
