/**
  *  \file test/ui/res/providertest.cpp
  *  \brief Test for ui::res::Provider
  */

#include "ui/res/provider.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using afl::io::InternalDirectory;
using afl::io::InternalStream;

/** Interface test. */
AFL_TEST("ui.res.Provider:basics", a)
{
    // Interface instantiation
    class Tester : public ui::res::Provider {
     public:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t /*name*/, ui::res::Manager& /*mgr*/)
            { return 0; }
    };
    Tester t;

    // Methods
    a.check("01", !ui::res::Provider::graphicsSuffixes().empty());
}

/** Interface test. */
AFL_TEST("ui.res.Provider:openResourceFile", a)
{
    // Test instance
    class Tester : public ui::res::Provider {
     public:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t /*name*/, ui::res::Manager& /*mgr*/)
            { return 0; }
    };
    Tester t;

    // An extension list
    const char*const LIST[] = { "txt", "doc" };

    // Empty directory
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        a.checkNull("01", t.openResourceFile(*dir, "t", LIST).get());
        a.checkNull("02", t.openResourceFile(*dir, "t.", LIST).get());
    }

    // First extension only
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        dir->addStream("t.txt", *new InternalStream());
        a.checkNull("11", t.openResourceFile(*dir, "t", LIST).get());
        a.checkNonNull("12", t.openResourceFile(*dir, "t.", LIST).get());
    }

    // Second extension only
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        dir->addStream("t.doc", *new InternalStream());
        a.checkNull("21", t.openResourceFile(*dir, "t", LIST).get());
        a.checkNonNull("22", t.openResourceFile(*dir, "t.", LIST).get());
    }

    // No extension
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        dir->addStream("t", *new InternalStream());
        a.checkNonNull("31", t.openResourceFile(*dir, "t", LIST).get());
        a.checkNull("32", t.openResourceFile(*dir, "t.", LIST).get());
    }

    // File with dot
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        dir->addStream("t.", *new InternalStream());
        a.checkNonNull("41", t.openResourceFile(*dir, "t.", LIST).get());
        a.checkNull("42", t.openResourceFile(*dir, "t", LIST).get());
    }

    // Multiple
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        Ref<InternalStream> f1 = *new InternalStream();
        f1->write(afl::string::toBytes("111"));
        dir->addStream("t.txt", f1);
        dir->addStream("t.doc", *new InternalStream());
        a.checkNull("51", t.openResourceFile(*dir, "t", LIST).get());
        a.checkNonNull("52", t.openResourceFile(*dir, "t.", LIST).get());
        a.check("53", t.openResourceFile(*dir, "t.", LIST)->getSize() != 0);
    }
}
