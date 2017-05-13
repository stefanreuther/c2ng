/**
  *  \file u/t_ui_res_provider.cpp
  *  \brief Test for ui::res::Provider
  */

#include "ui/res/provider.hpp"

#include "t_ui_res.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalstream.hpp"

using afl::base::Ref;
using afl::io::InternalDirectory;
using afl::io::InternalStream;

/** Interface test. */
void
TestUiResProvider::testIt()
{
    // Interface instantiation
    class Tester : public ui::res::Provider {
     public:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t /*name*/, ui::res::Manager& /*mgr*/)
            { return 0; }
    };
    Tester t;

    // Methods
    TS_ASSERT(!ui::res::Provider::graphicsSuffixes().empty());
}

/** Interface test. */
void
TestUiResProvider::testOpen()
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
        TS_ASSERT(t.openResourceFile(*dir, "t", LIST).get() == 0);
        TS_ASSERT(t.openResourceFile(*dir, "t.", LIST).get() == 0);
    }

    // First extension only
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        dir->addStream("t.txt", *new InternalStream());
        TS_ASSERT(t.openResourceFile(*dir, "t", LIST).get() == 0);
        TS_ASSERT(t.openResourceFile(*dir, "t.", LIST).get() != 0);
    }
    
    // Second extension only
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        dir->addStream("t.doc", *new InternalStream());
        TS_ASSERT(t.openResourceFile(*dir, "t", LIST).get() == 0);
        TS_ASSERT(t.openResourceFile(*dir, "t.", LIST).get() != 0);
    }
    
    // No extension
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        dir->addStream("t", *new InternalStream());
        TS_ASSERT(t.openResourceFile(*dir, "t", LIST).get() != 0);
        TS_ASSERT(t.openResourceFile(*dir, "t.", LIST).get() == 0);
    }

    // File with dot
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        dir->addStream("t.", *new InternalStream());
        TS_ASSERT(t.openResourceFile(*dir, "t.", LIST).get() != 0);
        TS_ASSERT(t.openResourceFile(*dir, "t", LIST).get() == 0);
    }

    // Multiple
    {
        Ref<InternalDirectory> dir = InternalDirectory::create("");
        Ref<InternalStream> f1 = *new InternalStream();
        f1->write(afl::string::toBytes("111"));
        dir->addStream("t.txt", f1);
        dir->addStream("t.doc", *new InternalStream());
        TS_ASSERT(t.openResourceFile(*dir, "t", LIST).get() == 0);
        TS_ASSERT(t.openResourceFile(*dir, "t.", LIST).get() != 0);
        TS_ASSERT(t.openResourceFile(*dir, "t.", LIST)->getSize() != 0);
    }
}

