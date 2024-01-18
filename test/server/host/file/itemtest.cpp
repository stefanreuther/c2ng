/**
  *  \file test/server/host/file/itemtest.cpp
  *  \brief Test for server::host::file::Item
  */

#include "server/host/file/item.hpp"

#include "afl/test/testrunner.hpp"
#include <stdexcept>

/** Interface test. */
AFL_TEST_NOARG("server.host.file.Item:interface")
{
    class Tester : public server::host::file::Item {
     public:
        virtual String_t getName()
            { return String_t(); }
        virtual Info_t getInfo()
            { return Info_t(); }
        virtual Item* find(const String_t& /*name*/)
            { return 0; }
        virtual void listContent(ItemVector_t& /*out*/)
            { }
        virtual String_t getContent()
            { return String_t(); }
    };
    Tester t;
}

/** Test resolvePath(). */
AFL_TEST("server.host.file.Item:resolvePath", a)
{
    using server::interface::FileBase;
    using server::host::file::Item;
    class TestItem : public Item {
     public:
        TestItem(const Info_t& i)
            : m_info(i)
            { }
        virtual String_t getName()
            { return m_info.name; }
        virtual Info_t getInfo()
            { return m_info; }
        virtual Item* find(const String_t& name)
            {
                if (m_info.type != FileBase::IsDirectory) {
                    return 0;
                } else if (name.compare(0, 1, "f") == 0) {
                    Info_t i;
                    i.name = name;
                    i.type = FileBase::IsFile;
                    i.size = 99;
                    return new TestItem(i);
                } else if (name.compare(0, 1, "d") == 0) {
                    Info_t i;
                    i.name = name;
                    i.type = FileBase::IsDirectory;
                    return new TestItem(i);
                } else {
                    return 0;
                }
            }

        // We do not expect these functions to be called.
        // Throw const char*, not std::exception, to be able to tell them from exceptions we expect.
        virtual void listContent(ItemVector_t& /*out*/)
            { throw "unexpected: listContent()"; }
        virtual String_t getContent()
            { throw "unexpected: getContent()"; }
     private:
        Info_t m_info;
    };

    // Test environment
    Item::Info_t i;
    i.name = "root";
    i.type = FileBase::IsDirectory;
    TestItem root(i);

    // Tests
    // - good case
    {
        Item::ItemVector_t vec;
        Item& it = root.resolvePath("d1/d2/f3", vec);
        a.checkEqual("01. getName", it.getName(), "f3");
        a.checkEqual("02. size",   vec.size(), 3U);
        a.checkEqual("03. item",   vec[2], &it);
        a.checkEqual("04. name 0", vec[0]->getName(), "d1");
        a.checkEqual("05. name 1", vec[1]->getName(), "d2");
    }

    // - bad cases
    {
        Item::ItemVector_t vec;
        AFL_CHECK_THROWS(a("11. error"), root.resolvePath("q", vec), std::exception);
        AFL_CHECK_THROWS(a("12. error"), root.resolvePath("q/f1", vec), std::exception);
        AFL_CHECK_THROWS(a("13. error"), root.resolvePath("/d1/d2/f3", vec), std::exception);
        AFL_CHECK_THROWS(a("14. error"), root.resolvePath("d1//d2/f3", vec), std::exception);
        AFL_CHECK_THROWS(a("15. error"), root.resolvePath("d1/d2/", vec), std::exception);
        AFL_CHECK_THROWS(a("16. error"), root.resolvePath("d1/d2/f3/f4", vec), std::exception);
        AFL_CHECK_THROWS(a("17. error"), root.resolvePath("d1/d2/f3/f4", vec), std::exception);
        AFL_CHECK_THROWS(a("18. error"), root.resolvePath("d1/d2/", vec), std::exception);
        AFL_CHECK_THROWS(a("19. error"), root.resolvePath("d1/d2/", vec), std::exception);
    }
}
